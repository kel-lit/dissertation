#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "random.h"
#include "secret.h"
#include "cipher.h"

typedef unsigned char byte;
typedef unsigned long ulong;
typedef unsigned int uint;

RF24 radio(7, 8); // CE, CSN
const byte addresses[][6] = {"00001", "00002"};

int button1 = 2;
int button2 = 3;
int led = 4;

int Delay = 40;

ulong secret;
ulong last_secret;
bool responded = true; //Start true to generate a challenge first round
uint challenge;

int setup_radio(ulong secret) {
	//Sends secret and waits for acknowledgement from reciever.
	//Ideally, secret would be shared via other, closer proximity means e.g. NFC.
	ulong acknowledge;
	ulong started = millis();

	while (millis() - started <= 5000){ //Try to setup for 5 seconds
		delay(10);
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

	radio.begin();
	radio.openWritingPipe(addresses[1]); // 00002
	radio.openReadingPipe(1, addresses[0]); // 00001
	radio.setPALevel(RF24_PA_MIN);

	pinMode(button1, INPUT_PULLUP);
	pinMode(button2, INPUT_PULLUP);
	pinMode(led, OUTPUT);

	secret = generate_secret(); //secret.h
	int success = setup_radio(secret);

	if (success){
		Serial.println("Secret sharing was successful");
	}
	else {
		// Reset
	}
}

void loop() {

	byte command;
	byte potential;
	byte checksum = 0;
	byte payload[5];

	get_command(&command, &potential); //Get control inputs
	
	if (responded){
		challenge = generate_challenge();
	}

	create_payload(payload, command, potential, challenge);

	checksum = get_checksum(payload);
	encrypt(payload, secret);
	payload[4] = checksum;

	// Send payload
	delay(Delay);
	radio.stopListening();
	radio.write(&payload, sizeof(payload));

	//Calculate expected challenge response
	byte expected_response[4];
	byte incoming[4];

	solve_challenge(expected_response, challenge);

	//Listen for acknowledgement
	delay(Delay);
	radio.startListening();

	while(!radio.available());
	radio.read(&incoming, sizeof(incoming));
	decrypt(incoming, secret);
	
	if (verify_response(incoming, expected_response)) {
		//Mutate the current secret
		last_secret = secret;
		mutate_secret(expected_response, &secret);
		responded = true;
		Serial.print("Challenge passed, New secret: "); Serial.println(secret);
	}
	
	else {
		//Need to prompt receiver to revert back to previous secret
		responded = false;
		Serial.println("Challenge Failed");
	}
}

void get_command(byte* command, byte* potential){
	// This function gets the command and potential from physical controls
	byte value = 0;
	value += (digitalRead(button1) == HIGH) ? 0 : 1; //Using INPUT_PULLUP, button pressed == LOW, not pressed == HIGH
	value += ((digitalRead(button2) == HIGH) ? 0 : 1) << 1;
	
	*command = value;
	*potential = value ? 255 : 0;

	return;
}

uint generate_challenge() {

	uint challenge = 0;

	challenge += (uint)getTrueRotateRandomByte(); //random.h
	challenge += (uint)getTrueRotateRandomByte() << 8;

	return challenge;
}

void solve_challenge(byte *expected_response, uint challenge) {
	
	ulong prime = 16777619; //recommended prime and seed from: http://isthe.com/chongo/tech/comp/fnv/
	ulong seed = 2166136261;
	ulong response;

	byte low = challenge;
	byte high = challenge >> 8;

	response += (low ^ seed) * prime;
	response += (high ^ seed) * prime << 8;
	response += (high ^ seed) * prime << 16;
	response += (low ^ seed) * prime << 24;

	for (int i=0; i < 4; i++) expected_response[i] = (byte)(response >> i*8);

	return;
}

void create_payload(byte *payload, byte command, byte potential, uint challenge) {

	payload[0] = command;
	payload[1] = potential;
	payload[2] = (byte)challenge;
	payload[3] = (byte)(challenge >> 8);

	return;
}

byte get_checksum(byte *payload) {

	byte checksum = 0;

	for (int i=0; i < 4; i++) { // -1 to miss the checksum in payload
		checksum += payload[i] % 255;
	}

	return checksum;
}

bool verify_response(byte *incoming, byte *expected) {

	return (memcmp(incoming, expected, sizeof(expected)) ? false : true);
}

void mutate_secret(byte *modifier_arr, ulong* secret ) {
	
	ulong modifier;
	for (int i=0; i < 4; i++) modifier += (ulong)modifier_arr[i] << i*8;

	*secret ^= modifier;
	return;
}