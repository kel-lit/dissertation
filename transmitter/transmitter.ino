#include <stdio.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "random.h"
#include "secret.h"

RF24 radio(7, 8); // CE, CSN
const byte addresses[][6] = {"00001", "00002"};
boolean buttonState = 0;
int button = 3;
int led = 4;

int setup_radio(unsigned long secret) {

	// Start timer for connection here ---- 
	int connected = 0;
	unsigned long acknowledge;
	while (!connected){
		delay(10);
		radio.stopListening();
		radio.write(&secret, sizeof(secret));
		delay(10);
		radio.startListening();
		while (!radio.available());
		radio.read(&acknowledge, sizeof(acknowledge));
		if (acknowledge){
			char buffer[80];
			Serial.println(buffer);
			if (acknowledge == secret){
				return 1;
			}
		}
	}
}

void setup() {
	Serial.begin(9600);

	radio.begin();
	radio.openWritingPipe(addresses[1]); // 00002
	radio.openReadingPipe(1, addresses[0]); // 00001
	radio.setPALevel(RF24_PA_MIN);

	pinMode(button, INPUT);
	pinMode(led, OUTPUT);

	randomSeed(analogRead(A0) * analogRead(A1));

	unsigned long secret = generate_secret();
	int success = setup_radio(secret);

	if (success == 1){
		Serial.println("Secret sharing was successful");
	}

}

void loop() {

	delay(5);
	radio.stopListening();

	byte value;
	byte new_val;

	// int value = digitalRead(button);
	if (Serial.available() > 0){
		new_val = Serial.read();
	}

	if (new_val != value){
		value = new_val;
		Serial.println(value);
	}

	if (value == 72){
		int state = "HIGH";
		radio.write(&value, sizeof(value));
	}
	
	// digitalWrite(5, value);
	

	delay(5);
	radio.startListening();

	while (!radio.available());
	radio.read(&buttonState, sizeof(buttonState));
	if (buttonState == HIGH) {
		digitalWrite(led, HIGH);
	}
	else {
		digitalWrite(led, LOW);
	}
}