import os, sys
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from common import radio, lightgate
import time

'''
Basically we will send a "pulse" to the server every 5 seconds saying we're alive
If the lightgate is interuptted then we'll send that message

'''
MESSAGE_PULSE = "P";
print("PiAthleticsTracker: Client")



# Define observer functions
def handleLightGate( red, green, blue ):
	radio.sendMessage( red + "," + green + "," + blue )	

def handleMessagesFromServer( message ):
	#Lol nothing to do

setupReader( handleLightGate );

while True:
	time.sleep( 5 )
	radio.sendMessage( MESSAGE_PULSE )
