import socket
import sys
import pickle

# Create a TCP/IP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_address = ('localhost', 10000)

def readStatus():
	sock.connect(server_address)
	
	data = false
	while not (data):
		data = sock.recv(16)
		connection.close()

	return pickle.load( data )	
