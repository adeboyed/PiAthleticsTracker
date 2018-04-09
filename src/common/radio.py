import RPi.GPIO as GPIO
from common.lib_nrf24 import NRF24
import time
import spidev
import threading

class Reader(threading.Thread):
  def run(self, processor):
    print("Started Reader")
    ackPL = [1]
    while(true):
      mutex.acquire()
      if (radio.available(0) ):
        receivedMessage = []
        radio.read(receivedMessage, radio.getDynamicPayloadSize())
        print("Received: {}".format(receivedMessage))

        print("Translating the receivedMessage into unicode characters")
        string = ""
        for n in receivedMessage:
            # Decode into standard unicode set
            if (n >= 32 and n <= 126):
                string += chr(n)

        processor(string)
        print( "Recieved " + string )

        radio.writeAckPayload(1, ackPL, len(ackPL))
        print("Loaded payload reply of {}".format(ackPL))
      else:
        time.sleep( 1 / 100 )
      mutex.release()

# Setup all of the radio information
GPIO.setmode(GPIO.BCM)
 
pipes = [[0xe7, 0xe7, 0xe7, 0xe7, 0xe7], [0xc2, 0xc2, 0xc2, 0xc2, 0xc2]]
 
radio = NRF24(GPIO, spidev.SpiDev())
radio.begin(0, 17)
radio.setPayloadSize(32)
radio.setChannel(0x60)
 
radio.setDataRate(NRF24.BR_2MBPS)
radio.setPALevel(NRF24.PA_MIN)
radio.setAutoAck(True)
radio.enableDynamicPayloads()
radio.enableAckPayload()
 
radio.openReadingPipe(0, pipes[1])
radio.printDetails()

# Asynchronous radio thread
reader = Reader(name = "Thread-{}".format(2))

# Used for acquiring locks on the radio
mutex = threading.Lock()

def setupReader( processor ):
	reader.run(processor)

def sendMessage( message ):
	print("Attempting to send message: " + message )
	# Acquire exclusive access to the radio
	mutex.acquire()
	radio.write( message )
	print("Message sent {}".format(message))
	if radio.isAckPayloadAvailable():
		returnedPL = []
		radio.read(returnedPL, radio.getDynamicPayloadSize())
		print("Our returned payload was {}".format(returnedPL))
		mutex.release()
		return true	
	else:
		print("No payload received")
		mutex.release()
		return false
		
