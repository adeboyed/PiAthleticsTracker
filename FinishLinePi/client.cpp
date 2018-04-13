#include <cstdlib>
#include <iostream>
#include <sstream>
#include <climits>
#include <string>
#include <unistd.h>
#include <RF24/RF24.h>
#include <thread>
#include "lightgate.h"

#define LIGHTGATE_OFF 2
#define	LIGHTGATE_ON 3

#define TIMEOUT_REQ_TIME 250
#define TIMEOUT_REQ_WAITING 10000

#define STATE_TIME_SYNCING 10
#define STATE_WAITING 20
#define STATE_IN_RACE 30
#define STATE_LIGHT_GATE 40

#define LIGHT_GATE_DELAY_TIME_SYNC 500
#define LIGHT_GATE_WAITING 400
#define LIGHT_GATE_CHECK 250
#define LIGHT_GATE_IN_RACE 30

using namespace std;
RF24 radio(22,0);
LightGate gate;

mutex radioLock;
bool radioListening = false;

// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = { 0xF0F0F0F0D2LL, 0xF0F0F0F0E1LL };
const unsigned long REQ_ACK = 1;
const unsigned long REQ_TIME = 100;
const unsigned long REQ_WAIT = 200;
const unsigned long REQ_LIGHT_GATE = 250;
const unsigned long REQ_RACE = 300;

unsigned long time_offset = 0;
unsigned long offset_rtt = ULONG_MAX; 
int offset_tries = 5;

unsigned long lightgate_active = LIGHTGATE_ON;
int current_state = STATE_TIME_SYNCING;

int lightgate_delay_rate = 2000;

void check_lightgate(){
	while (true){

		lightgate_active = ( gate.read() ) ? LIGHTGATE_ON ? LIGHTGATE_OFF;	
		
		if ( current_state == STATE_TIME_SYNCING ){
			delay( LIGHT_GATE_DELAY_TIME_SYNC );
		}else if ( current_state == STATE_WAITING ){ 
			delay( LIGHT_GATE_WAITING );
		}else if ( current_state == STATE_LIGHT_GATE ){
			delay( LIGHT_GATE_CHECK );
		}else if ( current_state == STATE_IN_RACE ){
			delay ( LIGHT_GATE_IN_RACE );
		}
	}
}

void simulate_race(){
	sleep(15);

	radioLock.lock();
	unsigned long measured_time = millis();
	//Adjust Time
	measured_time += time_offset;

	int send_tries = 3;

	while ( send_tries > 0 ){
		if ( radioListening ){
			radio.stopListening(); radioListening = false;
		}

		bool ok = radio.write( &measured_time, sizeof( unsigned long ) );
		unsigned long started_waiting_at = millis();
		if ( !ok ){
			printf("Failed! Radio Error \n");
		}

		// Now, continue listening
		radio.startListening(); radioListening = true;

		// Wait here until we get a response, or timeout (250ms)
		bool timeout = false;
		while ( ! radio.available() && ! timeout ) {
			if (millis() - started_waiting_at > TIMEOUT_REQ_TIME )
				timeout = true;
		}

		if ( !timeout ){
			unsigned long response;
			radio.read( &response, sizeof( unsigned long ) );

			if ( response == STATE_WAITING ){
				send_tries = 0;
			}else if ( response == STATE_IN_RACE ){
				send_tries++;
			} 
		}else {
			send_tries--;
		}
	}
	radioLock.unlock();

}

void cycle(){
	while ( true ){
		if ( current_state == STATE_TIME_SYNCING ){
			radioLock.lock();
			printf("STATE: TIME SYNCING \n");
			if ( radioListening ){
				radio.stopListening(); radioListening = false;
			}		

			unsigned long started_waiting_at = millis();
			bool ok = radio.write( &REQ_TIME, sizeof( unsigned long ) );
			if ( !ok ){
				printf("Failed! Radio Error \n");
			}
			// Now, continue listening
			radio.startListening(); radioListening = true;

			// Wait here until we get a response, or timeout (250ms)
			bool timeout = false;
			while ( ! radio.available() && ! timeout ) {
				if (millis() - started_waiting_at > TIMEOUT_REQ_TIME )
					timeout = true;
			}

			// Describe the results
			if ( timeout ){
				printf("Failed, response timed out.\n");

				//Increment tries in case of server loss
				if ( time_offset != 0 ){
					if ( offset_tries < 5 ){
						offset_tries++;	
					}else if ( offset_tries == 5 ){
						//Total reset
						time_offset = 0;
						offset_rtt = ULONG_MAX;
					}	
				}

			} else {
				// Grab the response, compare, and send to debugging spew
				unsigned long got_time;
				radio.read( &got_time, sizeof(unsigned long) );
				unsigned long rtt = millis() - started_waiting_at;
				if ( rtt < offset_rtt ){
					time_offset = ( got_time - rtt / 2 ) - started_waiting_at;
					printf("Got response %lu, round-trip delay: %lu\n", got_time, rtt );
					printf("min RTT achieved, calculated offset: %lu \n", time_offset );
					offset_rtt = rtt;
				} else {
					printf("Got response %lu, round-trip delay: %lu\n", got_time, rtt );
				}

				offset_tries--;	
				if ( offset_tries <= 0 ){
					current_state = STATE_WAITING;
				}
			}
			radioLock.unlock();	
			sleep(1);			
		} else if ( current_state == STATE_WAITING || current_state == STATE_IN_RACE ) {
			radioLock.lock();

			if ( current_state == STATE_WAITING ){
				printf("STATE: WAITNG \n" );
			} else if ( current_state == STATE_IN_RACE ){
				printf("STATE IN RACE \n" );
			}

			if ( !radioListening ){
				radio.startListening(); radioListening = true;
			}

			unsigned long started_waiting_at = millis();
			bool timeout = false;
			while ( ! radio.available() && ! timeout ) {
				if (millis() - started_waiting_at > TIMEOUT_REQ_WAITING )
					timeout = true;
			}

			if ( timeout ){
				printf("Have not recieved anything from server in 10 seconds, will enter time sync \n");
				offset_tries = 1;
				current_state = STATE_TIME_SYNCING;
			}else {
				unsigned long req_code;
				radio.read( &req_code, sizeof( unsigned long ) );

				printf("Recieved %lu from the server \n", req_code );
				if ( req_code == REQ_RACE ){
					current_state = STATE_IN_RACE;

					radio.stopListening();
					radio.write( &REQ_ACK, sizeof( unsigned long ) );	
					radio.startListening();
					radioLock.unlock();	
				}else if ( req_code == REQ_WAIT ){
					current_state = STATE_WAITING;
					
					radio.stopListening();
					radio.write( &REQ_ACK, sizeof( unsigned long ) );	
					radio.startListening();

					radioLock.unlock();	
					delay( 500 );
				}else if ( req_code == REQ_LIGHT_GATE ){
					current_state = STATE_LIGHT_GATE

					radio.stopListening();
					radio.write( &lightgate_active, sizeof ( unsigned long ) );
					radio.startListening();

					radioLock.unlock();	
					delay ( 500 );
				}			
			}	
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
	radio.openWritingPipe(pipes[0]);
	radio.openReadingPipe(1,pipes[1]);

	//Setup lightgate
	gate.setup();	

	printf("PiAthleticsTracker: Finish Line Pi \n");

	//Thread for checking the lightgate
	thread t1( check_lightgate );

	//Thread for managing the client
	thread t2 ( cycle );

	t1.join();
	t2.join();

}
