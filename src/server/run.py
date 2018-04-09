import os, sys
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from common import radio
import time
from subprocess import call
import socket
import sys
import pickle
import random

'''



'''

# Set volume to 100%
call(["/usr/bin/amixer", "-q", "set", "Digital", "100"])

MESSAGE_PULSE = "P";
LIGHT_GATE_ON = "L";
LIGHT_GATE_OFF = "O";

print("PiAthleticsTracker: Server")


# Important state to keep
clientStatus = 0
lightGateCaptured = False

gameStartingSoon = False
inGame = False
startGameTime = 0
lastGameTime = 0

status_socket = socket(AF_INET, SOCK_STREAM)
status_socket.bind( ('localhost', 8081)  )
status_socket.listen(5)

race_socket = socket(AF_INET, SOCK_STREAM)
race_socket.bind( ('localhost', 8082)  )
race_socket.listen(5)

def millis():
	return time.time() * 1000

def handleMessagesFromClient( message ):
	if ( message == MESSAGE_PULSE ):
		if ( clientStatus < 3 ):
			clientStatus += 1
	elif ( message == LIGHT_GATE_ON ):
		lightGameCaptured = True
	elif ( message == LIGHT_GATE_OFF ):
		lightGateCaptured = False
		if ( inGame ):
			lastGameTime = millis() - startGameTime
			inGame = False
		
radio.setupReader( handleMessagesFromClient )

# Basically we want to decrement the status in case we lose the client
def decrementClientStatus():
	clientStatus -= 1
	time.sleep(10)	

def startRace():
	gameStartingSoon = True
  # Wait 10 seconds more starting race
	time.sleep(10)

  # Play track for ready

  # Wait 5 seconds
	time.sleep ( 5 )

  # Play track for set

  # Wait between 1 and 3 seconds
	time.sleep ( random.randint(1, 300) / 3 )

  # Play track for go
	

	gameStartingSoon = False
	inGame = True
	startGameTime = millis()

def statusClientHandler( c = None ):
	data = {
		'clientStatus': clientStatus,
		'lightGateCaptured': lightGateCaptured,
		'raceInProgress': ( gameStartingSoon or inGame ),
		'lastRaceTime': inGame 	
	}
	pickle.dump(data, output)
	c.sendto(data, client)
	c.close();

def raceClientHandler( c = None ):
	data = c.recv(1024)
	c.close()
	startRace()

def statusSocket():
	c, addr = status_socket.accept()
	Thread(target=statusClientHandler, kwargs={ 'c' : c } ).start()

def raceSocket():
	c, addr = race_socket.accept()
	Thread(target=raceClientHandler, kwargs={ 'c' : c } ).start()

# Start all of the threads
Thread( target=decrementClientStatus ).start()
Thread( target=statusSocket ).start()
Thread( target=raceSocket ).start()

