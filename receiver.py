import piVirtualWire.piVirtualWire as piVirtualWire
import time
import pigpio
import struct
from datetime import datetime
import firebase_admin
from firebase_admin import credentials
from firebase_admin import firestore

pi = pigpio.pi()
rx = piVirtualWire.rx(pi, 18, 2000) # Set pigpio instance, TX module GPIO pin and baud rate

cred = credentials.Certificate('/home/pi/firebase-keys.json')
firebase_admin.initialize_app(cred)

db = firestore.client()


def insertIntoDatabase(id, moistureValue, voltage):

	post_data = {
    		'id': id,
    		'moisture': moistureValue,
		'battery': voltage,
		'timestamp': firestore.SERVER_TIMESTAMP
	}

	doc_ref = db.collection('plants').add(post_data)

while True:

		while rx.ready():
			values = rx.get()
			print(values)
			idLow = values[4]
			idHigh = values[5]
			xlow = values[6]
			xhigh = values[7]
			vlow = values[8]
			vhigh = values[9]
			id = (idHigh << 8) & 0xFF00 | idLow & 0x00FF
			moistureValue = ( xhigh << 8 ) & 0xFF00 | xlow & 0x00FF
			voltage = (vhigh << 8) & 0xFF00 | vlow & 0x00FF
			print(f'Moisture Sensor {id}  has a reading of  {moistureValue} milivolts  with a battery voltage of {voltage} ')
			insertIntoDatabase(id, moistureValue, voltage)
		time.sleep(0.5)

rx.cancel()
pi.stop()

