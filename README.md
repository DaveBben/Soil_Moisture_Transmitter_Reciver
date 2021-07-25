# Arduino Soil Moisture Transmitter and Receiver
This is a personal project of mine to determine whether or not my house plants needed to be watered. For this project, I attached a [capacitive soil sensor](https://www.amazon.com/gp/product/B07H3P1NRM/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1) to an Atmega328 and recorded the wetness of the soil every 4 hours. That data is transmitted using [433mhz radio](https://www.amazon.com/gp/product/B00HEDRHG6/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1) attached to the Atmega with a receiver attached to a raspberry pi. The raspberry pi then uploads that data to Google Firebase where I have a [cloud function](https://github.com/DaveBben/fcm_soil_notification_function) which reads the value of the soil sensor and triggers an alert through FCM to my android app.

## Arduino Code
All of the code for the Atmega328 is located in the `433mhz_transmitter` folder.

## Receiver Code
`receiver.py` is the python code which runs on the Raspberry Pi. I also created a service called `soil_moisture.service` which is enabled and executed by systemd on the raspberry pi. This service called the `start_soil_service.sh` after the `pigpiod.service` is called. Make sure that [pigpiod is installed](https://github.com/guymcswain/pigpio-client/wiki/Install-and-configure-pigpiod) on your Raspberry Pi before using this.

## Google Cloud Code
You can find the function code for Google Cloud [in this repo](https://github.com/DaveBben/fcm_soil_notification_function).

## Android code

# Schematics
Schematics and Gerber files are located in the `schematic_pcb_gerber_files` folder.


