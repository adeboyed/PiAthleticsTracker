#include <cstdlib>
#include <iostream>
#include <sstream>
#include <climits>
#include <string>
#include <unistd.h>
#include <RF24/RF24.h>

using namespace std;
RF24 radio(22,0);

const int TIMEOUT_REQ_TIME = 250;
const int TIMEOUT_REQ_WAITING = 10000;

const unsigned long REQ_ACK = 1;
const unsigned long REQ_TIME = 100;
const unsigned long REQ_WAIT = 200;
const unsigned long  REQ_RACE = 300;

const int STATE_TIME_SYNCING = 10;
const int STATE_WAITING = 20;
const int STATE_IN_RACE = 30;

// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = { 0xF0F0F0F0D2LL, 0xF0F0F0F0E1LL };

unsigned long time_offset = 0;
unsigned long offset_rtt = ULONG_MAX; 
int offset_tries = 5;

int current_state = STATE_TIME_SYNCING;
bool radioListening = false;

void cycle(){
	if ( current_state == STATE_TIME_SYNCING ){
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
		sleep(1);			
	} else if ( current_state == STATE_WAITING ) {
		printf("STATE: WAITNG \n" );
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
			printf("Have not recieved anything from server in 10 seconds, will enter time sync");
			offset_tries = 1;
			current_state = STATE_WAITING;
		}else {
			int req_code;
			radio.read( &req_code, sizeof( unsigned long ) );
			
			//Send back an ACK
			radio.stopListening();
            radio.write( &REQ_ACK, sizeof( unsigned long ) );	
			radio.startListening();			

			if ( req_code == REQ_RACE ){
				current_state = STATE_IN_RACE;	
			}else if ( req_code == REQ_WAIT ){
				sleep (1);
			}			
		}	
	} else if ( current_state == STATE_IN_RACE ){
		printf("STATE: IN_RACE \n");

		//Simulate race
		sleep(15);

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

	

		current_state = STATE_WAITING;
		sleep(1);
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

	printf("PiAthleticsTracker: Finish Line Pi \n");

	while (true){
		cycle();	
	}

}
