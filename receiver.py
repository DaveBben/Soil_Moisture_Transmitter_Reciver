import piVirtualWire.piVirtualWire as piVirtualWire
import time
import pigpio
import struct
from datetime import datetime
from pymongo import MongoClient
import firebase_admin
from firebase_admin import credentials
from firebase_admin import firestore

pi = pigpio.pi()
rx = piVirtualWire.rx(pi, 18, 2000) # Set pigpio instance, TX module GPIO pin and baud rate

cred = credentials.Certificate('/home/pi/firebase-keys.json')
firebase_admin.initialize_app(cred)

db = firestore.client()

client = MongoClient()
client = MongoClient('localhost', 27017)
db = client.plants


def insertIntoDatabase(id, moistureValue, voltage):
	posts = db.moisture
	post_data = {
    'date': datetime.now().strftime("%d/%m/%Y %H:%M:%S"),
    'id': id,
    'moisture': moistureValue,
	'battery': voltage
	}
	result = posts.insert_one(post_data)
	print('One post: {0}'.format(result.inserted_id))

	doc_ref = db.collection('plants').add({
    		'moisture': moistureValue,
    		'battery':  voltage,
    		 'id': id,
		'date':  datetime.now().strftime("%d/%m/%Y %H:%M:%S"),
		
	})

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

