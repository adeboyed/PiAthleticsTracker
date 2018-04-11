import socket
import sys
import json

def GetStatus():
	sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	sock.connect( ('localhost', 1010) )
	data = sock.recv(1024)
	sock.close()
	return json.load( data )

def StartRace():
	sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	connection = sock.connect( ('localhost', 1011) )
	connection.close()
