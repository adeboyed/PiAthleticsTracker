import os, sys
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from common import radio, lightgate
import time
import alsaaudio

'''



'''

# Set volume to 100%
m = alsaaudio.Mixer()
m.setvolume(100)

MESSAGE_PULSE = "P";
LIGHT_GATE_ON = "L";
LIGHT_GATE_OFF = "O";

print("PiAthleticsTracker: Server")


# Important state to keep
clientStatus = 0
lightGateCaptured = false

gameStartingSoon = false
inGame = false
startGameTime = 0
lastGameTime = 0

def millis():
	return time.time() * 1000

def handleMessagesFromClient( message ):
	if ( message == MESSAGE_PULSE ):
		if ( clientStatus < 2 ):
			clientStatus += 2
	elif ( message == LIGHT_GATE_ON ):
		lightGameCaptured = true
	elif ( message == LIGHT_GATE_OFF ):
		lightGateCaptured = false
		if ( inGame ):
			lastGameTime = millis() - startGameTime
			inGame = false	
		
radio.setupReader( handleMessagesFromClient )


# Basically we want to decrement the status in case we lose the client
while True:
	clientStatus -= 1;
	time.sleep(10)
