#include <cstdlib>
#include <iostream>
#include <sstream>
#include <climits>
#include <string>
#include <unistd.h>
#include <RF24/RF24.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "nlohmann/json.hpp"

#define IN_SERVER_PORT htons(1010)
#define OUT_SERVER_PORT htons(1011)

#define TIMEOUT_REQ_TIME 250
#define TIMEOUT_REQ_WAITING 10000
#define TIMEOUT_RACE 60000

using namespace std;
using json = nlohmann::json;

RF24 radio(22,0);

// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = { 0xF0F0F0F0D2LL, 0xF0F0F0F0E1LL };
const unsigned long REQ_ACK = 1;
const unsigned long REQ_TIME = 100;
const unsigned long REQ_WAIT = 200;
const unsigned long REQ_LIGHT_GATE = 250;
const unsigned long REQ_RACE = 300;

mutex radioLock;

bool radioListening = false;

unsigned long lastClientInteraction = ULONG_MAX;
unsigned long lastSyncInteraction = ULONG_MAX;
unsigned long lastWebClientInteraction = ULONG_MAX;
bool lightGateCaptured = false;

bool raceStartingSoon = false;
bool inRace = false;
int startRaceTime = 0;
int lastRaceTime = 0;

bool client_alive(){
	return ( millis() - lastClientInteraction ) < TIMEOUT_REQ_WAITING;
} 

bool send_race_status(){
	radioLock.lock();
	printf("Start Race Thread: sending message \n ");

	radio.stopListening();
	radio.write( &REQ_RACE, sizeof( unsigned long ) );
	radio.startListening();			

	unsigned long started_waiting_at = millis();
	bool timeout = false;
	while ( ! radio.available() && ! timeout ) {
		if (millis() - started_waiting_at > TIMEOUT_REQ_TIME )
			timeout = true;
	}

	if ( !timeout ){
		unsigned long response;
		radio.read( &response, sizeof( unsigned long ) );
		printf("Start Race Thread: recieved %lu from client \n", response);
		if ( response == REQ_ACK ){
			lastClientInteraction = millis();
			radioLock.unlock();
			return true;
		}
	}

	radioLock.unlock();
	return false;
}

void start_race(){
	if ( raceStartingSoon || inRace || !lightGateCaptured ){
		printf("Start Race Thread: race rejected \n ");
		return;
	}
	printf("Start Race Thread: starting sequence... \n ");

	raceStartingSoon = true;

	// Wait 10 seconds more starting race
	sleep(10);

	//Get client ready

	// Play track for ready

	//Wait 5 seconds
	sleep ( 5 );

	//Play track for set

	//Wait between 1 and 3 seconds
	delay( (rand() % 2000) + 1000 );

	//Play track for go
	if ( send_race_status() ){
		raceStartingSoon = false;
		inRace = true;
		startRaceTime = millis();
	}
}

void handle_web_clients(){
	//Setup server socket
	int serverSock=socket(AF_INET, SOCK_STREAM, 0);

	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = IN_SERVER_PORT;
	serverAddr.sin_addr.s_addr = INADDR_ANY;

	bind(serverSock, (struct sockaddr*)&serverAddr, sizeof(struct sockaddr));
	listen(serverSock,1);

	while (true){
		sockaddr_in clientAddr;
		socklen_t sin_size = sizeof(struct sockaddr_in);
		int client_sock = accept(serverSock,(struct sockaddr*)&clientAddr, &sin_size);	
		if ( client_sock > 0 ){
			printf("Valid web client socket request \n");
			json j;
			double time = roundf( lastRaceTime * 10 ) / 100; 	

			j["client_status"] = client_alive();
			j["light_gate_captured"] = lightGateCaptured;
			j["race_in_progress"] = ( raceStartingSoon || inRace );

			if ( time < 0 ){
				j["last_race_time"] = "timeout";
			} else {
				j["last_race_time"] = time;		
			}

			string output = j.dump();
			char char_array[ output.length() ];
			strcpy(char_array, output.c_str());

			printf("Sending %s to web client \n", char_array );
			write( client_sock, char_array, strlen(char_array) );
			close( client_sock );
			lastWebClientInteraction = millis();
		}else {
			printf("Invalid web client socket request \n");
		}
	}
}

void handle_web_clients_race(){
	int serverSock=socket(AF_INET, SOCK_STREAM, 0);

	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = OUT_SERVER_PORT;
	serverAddr.sin_addr.s_addr = INADDR_ANY;

	bind(serverSock, (struct sockaddr*)&serverAddr, sizeof(struct sockaddr));
	listen(serverSock, 1);

	while (true){
		sockaddr_in clientAddr;
		socklen_t sin_size = sizeof(struct sockaddr_in);
		int client_sock = accept(serverSock,(struct sockaddr*)&clientAddr, &sin_size);	
		if ( client_sock > 0 ){
			printf("Web Client requested start of race \n");
			close( client_sock );
			lastWebClientInteraction = millis();
			start_race();
		}
	}
}

void radio_listen(){
	while (true){
		radioLock.lock();
		if ( radio.available() ){
			printf("Radio Listen Thread: radio available \n");
			unsigned long payload;

			while(radio.available()){
				radio.read( &payload, sizeof( unsigned long ) );
			}
			radio.stopListening(); radioListening = false;

			printf("Radio Listen Thread: Recieved: %lu \n", payload );

			if ( payload == REQ_TIME ){
				unsigned long got_time = millis();
				radio.write( &got_time, sizeof(unsigned long) );
				radio.startListening(); radioListening = true;
				lastClientInteraction = millis();
				lastSyncInteraction = millis();
				delay(400);
			}else if ( inRace ){
				radio.write ( &REQ_ACK, sizeof (unsigned long) );				
				radio.startListening(); radioListening = true;
				lastClientInteraction = millis();

				//Finish Race
				inRace = false;
				lastRaceTime = payload - startRaceTime;
			}
		}
		radioLock.unlock();
	}
}

void client_check(){
	while (true){
		bool alive_client = client_alive();
		printf( alive_client ? "Client Check Thread: clientAlive \n" : "Client Check Thread: clientDead \n" ); 
		if ( alive_client && ( lastSyncInteraction - millis() ) > 2000 ){
			radioLock.lock();
			printf("Client Check Thread: sending heartbeat \n");

			radio.stopListening();
			if ( lastWebClientInteraction < TIMEOUT_REQ_WAITING ){ 
				radio.write( &REQ_LIGHT_GATE, sizeof( unsigned long ) );
			}else 
				radio.write( &REQ_WAIT, sizeof( unsigned long ) );
			}	
			radio.startListening();			

			unsigned long started_waiting_at = millis();
			bool timeout = false;
			while ( ! radio.available() && ! timeout ) {
				if (millis() - started_waiting_at > TIMEOUT_REQ_TIME )
					timeout = true;
			}

			if ( !timeout ){
				unsigned long response;
				radio.read( &response, sizeof( unsigned long ) );
				printf("Client Check Thread: recieved %lu from client \n", response);
				if ( response == REQ_ACK ){
					lastClientInteraction = millis();
				}else if ( response == LIGHTGATE_OFF ){
					lastClientInteraction = millis();
					lightGateCaptured = false;	
				}else if ( response == LIGHTGATE_ON ){
					lastClientInteraction = millis();
					lightGateCaptured = true;	
				}
			}

			radioLock.unlock();
		}

		//Poor design but we'll handle timeouts here
		if ( inRace ){
			long race_time = millis() - startRaceTime;
			if ( race_time > TIMEOUT_RACE ){
				//Client probably missed it, we're gonna timeout;
				lastRaceTime = -1;
				inRace = false;	 
			} 
		}
		if ( lastWebClientInteraction < TIMEOUT_REQ_WAITING ){
			delay( 750 );
		}else {
			delay( 2000 );
		}	
	}
}

int main(int argc, char** argv){
	// Setup and configure rf radio
	radio.begin();
	// optionally, increase the delay between retries & # of retries
	radio.setRetries(15,15);
	// Dump the configuration of the rf unit for debugging
	radio.printDetails();

	//Open pipes
	radio.openWritingPipe(pipes[1]);
	radio.openReadingPipe(1,pipes[0]);

	//Start Listening
	radio.startListening();

	//Turn up the volume
	system("amixer set PCM -- 100%");

	printf("PiAthleticsTracker: Start Line Pi \n");

	//Start threads for web client socket server
	thread t1( handle_web_clients );
	thread t2( handle_web_clients_race );

	//Start thread to listen on radio
	thread t3 ( radio_listen );	

	//Start thread for monitoring client
	thread t4 ( client_check );

	t1.join();
	t2.join();
	t3.join();
	t4.join();

}
