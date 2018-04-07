import os, sys
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from common import radio, lightgate
import time
import alsaaudio
import socket
import sys
import pickle

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
		if ( clientStatus < 3 ):
			clientStatus += 1
	elif ( message == LIGHT_GATE_ON ):
		lightGameCaptured = true
	elif ( message == LIGHT_GATE_OFF ):
		lightGateCaptured = false
		if ( inGame ):
			lastGameTime = millis() - startGameTime
			inGame = false	
		
radio.setupReader( handleMessagesFromClient )

# Basically we want to decrement the status in case we lose the client
def decrementClientStatus():
	clientStatus -= 1
	time.sleep(10)	

def clientHandler( c = None ):
	data = {
		'clientStatus': clientStatus,
		'lightGateCaptured': lightGateCaptured,
		'raceInProgress': ( gameStartingSoon or inGame ),
		'lastRaceTime': inGame 	
	}
	c.sendto(data, client)
	c.close();

s = socket(AF_INET, SOCK_STREAM)
server_address = ('localhost', 10000)
s.bind( server_address )
s.listen(5)

# Start all of the threads
Thread( target=decrementClientStatus ).start()

while True:
	c, addr = s.accept()
	Thread(target=clientHandler, kwargs={ 'c' : c } ).start()

