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

#define IN_SERVER_PORT htons(1010)
#define OUT_SERVER_PORT htons(1011)

using namespace std;

const int TIMEOUT_REQ_TIME = 250;
const int TIMEOUT_REQ_WAITING = 10000;

const unsigned long REQ_ACK = 1;
const unsigned long REQ_TIME = 100;
const unsigned long REQ_WAIT = 200;
const unsigned long  REQ_RACE = 300;

RF24 radio(22,0);

// Radio pipe addresses for the 2 nodes to communicate.
const uint8_t pipes[][6] = {"1Node","2Node"};

mutex radioLock;

int current_state = STATE_TIME_SYNCING;
bool radioListening = false;

unsigned long lastClientInteration = ULONG_MAX;
unsigned long lastSyncInteration = ULONG_MAX;
bool lightGateCaptured = false;

bool raceStartingSoon = false;
bool inRace = false;
int startRaceTime = 0;
int lastRaceTime = 0;

bool clientAlive(){
	return ( lastClientInteration - millis() ) < TIMEOUT_REQ_WAITING;
} 

void startRace(){
	if ( raceStartingSoon || inRace ){
		return;
	}

	raceStartingSoon = true;

	// Wait 10 seconds more starting race
    sleep(10);


	//Get client ready

	// Play track for ready

	//Wait 5 seconds
    time.sleep ( 5 )

	//Play track for set

	//Wait between 1 and 3 seconds
    time.sleep ( random.randint(1, 300) / 3 )

	//Play track for go

    raceStartingSoon = false
    inRace = true
	startRaceTime = millis();
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
		char *output = 
			"{" +
				"\"clientStatus\": " + clientAlive() + ", " +
				"\"lightGateCaptured\": " + lightGateCaptured + ", " +
				"\"raceInProgress\" : " + ( raceStartingSoon || inRace ) + ", " +
				"\"lastRameTime\" : " + lastRaceTime +
			"}";
			
		sockaddr_in clientAddr;
        socklen_t sin_size = sizeof(struct sockaddr_in);
        int clientSock = accept(serverSock,(struct sockaddr*)&clientAddr, &sin_size);	
		if ( clientSock > 0 ){
			write( clientSock, output, strlen(output) );
			close( clientSock );
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
        int clientSock = accept(serverSock,(struct sockaddr*)&clientAddr, &sin_size);	
		if ( clientSock > 0 ){
			close( clientSock );
			startRace();
		}
	}
}

void radioListen(){
	while (true){
		radioLock.lock();
		if ( radio.available() ){
			int payload;
			
			while(radio.available()){
				radio.read( &payload, sizeof( unsigned long ) );
            }
			radio.stopListening(); radioListening = false;

			if ( payload == REQ_TIME ){
				unsigned long got_time = millis();
				radio.write( &got_time, sizeof(unsigned long) );
				radio.startListening(); radioListening = true;
				lastClientInteration = millis();
				lastSyncInteration = millis();
				delay(400);
			}else if ( inRace ){
				radio.write ( &REQ_ACK, sizeof (unsigned long) );				
				radio.startListening(); radioListening = true;
                lastClientInteration = millis();

				//Finish Race
				inRace = false;
				lastRaceTime = payload - startRaceTime;
			}
		}
		radioLock.unlock();
	}
}

void clientCheck(){
	while (true){
		if ( clientAlive() && ( lastSyncInteration - millis() ) < 2000 ){
			while (true) radioLock.lock();
			radio.stopListening();

			radio.write( &REQ_REQUEST, sizeof( unsigned long ) );

			bool timeout = false;
            while ( ! radio.available() && ! timeout ) {
                if (millis() - started_waiting_at > TIMEOUT_REQ_TIME )
                    timeout = true;
            }

			if ( !timeout ){
				unsigned long response;
				radio.read( &response, sizeof( unsigned long ) );
				if ( response == REQ_ACK ){
					lastClientInteration = millis();
				}
			}

			radio.startListening();
			radioLock.unlock();
		}
		delay( 2500 );	
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

	//Turn up the volume
	system("amixer set PCM -- 100%");

	printf("PiAthleticsTracker: Start Line Pi \n");

	//Start threads for web client socket server
	thread t1( handle_web_clients );
	thread t2( handle_web_clients_race );

	//Start thread to listen on radio
	thread t3 ( radioListen );	

	//Start thread for monitoring client
	thread t4 ( clientCheck );

}
