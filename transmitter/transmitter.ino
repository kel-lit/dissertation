#include <stdio.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "random.h"
#include "secret.h"

typedef unsigned char byte;
typedef unsigned long ulong;
typedef unsigned int uint;

// Radio vars
RF24 radio(7, 8); // CE, CSN
const byte addresses[][6] = {"00001", "00002"};
// Button vars
boolean buttonState = 0;
int button1 = 2;
int button2 = 3;
int led = 4;
//reset
int reset = 10;
// Secret
ulong secret;
ulong last_secret;

int setup_radio(ulong secret) {
	//Sends secret and waits for acknowledgement from reciever.
	//Ideally, secret would be shared via other, closer proximity means e.g. NFC.
	ulong acknowledge;
	ulong started = millis();
	ulong wait_time = 5000; // 5 seconds

	while (millis() - started <= wait_time){ //Try to setup for 5 seconds
		delay(100);
		radio.stopListening();
		radio.write(&secret, sizeof(secret));
		delay(10);
		radio.startListening();
		radio.read(&acknowledge, sizeof(acknowledge));

		if (acknowledge == secret){
			return 1;
		}
	}
	return 0;
}

void setup() {

	Serial.begin(9600);
	digitalWrite(reset, HIGH);

	radio.begin();
	radio.openWritingPipe(addresses[1]); // 00002
	radio.openReadingPipe(1, addresses[0]); // 00001
	radio.setPALevel(RF24_PA_MIN);

	pinMode(button1, INPUT);
	pinMode(button2, INPUT);
	pinMode(reset, OUTPUT);
	pinMode(led, OUTPUT);

	secret = generate_secret(); //secret.h
	int success = setup_radio(secret);

	if (success){
		Serial.println("Secret sharing was successful");
	}
	else {
		digitalWrite(reset, LOW);// Reset arduino
	}
}

void loop() {

	delay(2000);
	radio.stopListening();

	byte command;
	byte potential;	
	uint challenge = 0;
	ulong payload = 0;

	get_command(&command, &potential); //Get control inputs
	
	challenge = generate_challenge();
	payload = create_payload(command, potential, challenge);
	Serial.print("Sending: "); Serial.println(payload);
	payload ^= secret; // Crude 'encryption' for proof-of-concept

	radio.write(&payload, sizeof(payload));
	//Calculate expected challenge response
	ulong expected_response;
	ulong incoming;

	expected_response = solve_challenge(challenge);
	delay(2000);
	radio.startListening();
	while(!radio.available());
	radio.read(&incoming, sizeof(incoming));
	Serial.print("Recieving: "); Serial.println(incoming);

	if (incoming ^ secret == expected_response){
		last_secret = secret;
		Serial.println(incoming);
	}
	else {
		//It might be a command

	}
}

void get_command(byte* command, byte* potential){
	// This function gets the command and potential from physical controls
	byte value = 0;
	value += digitalRead(button1);
	value += digitalRead(button2) << 1;
	
	*command = value;
	*potential = value ? 255 : 0;

	return;
}

uint generate_challenge() {

	uint challenge = 0;

	challenge += (byte)getTrueRotateRandomByte();
	challenge += (byte)getTrueRotateRandomByte() << 8;

	return challenge;
}

ulong solve_challenge(uint challenge) {
	
	ulong prime = 16777619; //recommended prime and seed from: http://isthe.com/chongo/tech/comp/fnv/
	ulong seed = 2166136261;
	ulong response;

	byte low = challenge;
	byte high = challenge >> 8;

	response += (low ^ seed) * prime;
	response += (high ^ seed) * prime << 8;
	response += (high ^ seed) * prime << 16;
	response += (low ^ seed) * prime << 24;

	return response;
}

ulong create_payload(byte command, byte potential, uint challenge) {

	ulong payload;

	payload += (ulong)potential;
	payload += (ulong)command << 8;
	payload += (ulong)challenge << 16;

	return payload;
}

void mutate_secret(ulong* modifier, ulong* secret ) {

	
	*secret ^= *modifier;

	return;
}