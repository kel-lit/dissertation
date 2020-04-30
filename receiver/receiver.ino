#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "cipher.h"

typedef unsigned long ulong;
typedef unsigned int uint;

RF24 radio(7, 8); // CE, CSN
const byte addresses[][6] = {"00001", "00002"};

int leds[3] = {6,4,5};

int Delay = 40; //Milliseconds

ulong secret;

ulong listen_for_secret() {

	delay(5);
	radio.startListening();

	while (!radio.available());
	radio.read(&secret, sizeof(secret));
	Serial.print("Secret recieved: "); Serial.println(secret);

	if (secret){
		radio.stopListening();
		radio.write(&secret, sizeof(secret));
		return secret;
	}		
}

void setup() {

	Serial.begin(9600);

	radio.begin();
	radio.openWritingPipe(addresses[0]); // 00001
	radio.openReadingPipe(1, addresses[1]); // 00002
	radio.setPALevel(RF24_PA_MIN);

	for (int i; i < sizeof(leds); i++){
		pinMode(leds[i], OUTPUT);
	}

	ulong secret = listen_for_secret();
}

void loop() {

	delay(Delay);
	radio.startListening();

	byte command;
	byte potential;
	byte payload[5];
	byte checksum;
	uint challenge;

	if (radio.available()) {
		radio.read(&payload, sizeof(payload));
		
		checksum = payload[4];
		decrypt(payload, secret);

		if (verify_checksum(payload, checksum)) {
			command = payload[0];
			potential = payload[1];
			challenge = payload[2]; challenge += (uint)(payload[3] << 8);

			byte response[4];
			solve_challenge(response, challenge);

			byte res_copy[4];
			for (int i=0; i < sizeof(response); i++){
				res_copy[i] = response[i];
			}
			
			encrypt(response, secret);
			
			radio.stopListening();
			radio.write(&response, sizeof(response));
			radio.startListening();

			execute_command(command, potential);

			mutate_secret(res_copy, &secret);
			Serial.print("New Secret: "); Serial.println(secret);
		}
		else {
			Serial.println("Checksum failed");
		}
	}
	delay(Delay);
}

bool verify_checksum(byte *payload, byte checksum) {

	byte exp_checksum = get_checksum(payload);

	if (exp_checksum == checksum) {
		return true;
	}
	else {
		return false;
	}
}

byte get_checksum(byte *payload) {

	byte checksum = 0;

	for (int i=0; i < 4; i++) { // -1 to miss the checksum in payload
		checksum += payload[i] % 255;
	}

	return checksum;
}


void solve_challenge(byte *response_arr, uint challenge) {
	
	ulong prime = 16777619; //recommended prime and seed from: http://isthe.com/chongo/tech/comp/fnv/
	ulong seed = 2166136261;

	ulong response;

	byte low = challenge & 0xFF;
	byte high = challenge >> 8;

	response += (low ^ seed) * prime;
	response += (high ^ seed) * prime << 8;
	response += (high ^ seed) * prime << 16;
	response += (low ^ seed) * prime << 24;

	for (int i=0; i < 4; i++) response_arr[i] = (byte)(response >> i*8);

	return;
}

void execute_command(byte command, byte potential) { 
	//Controls receiver LEDs based on command input
	for (byte i=0; i < sizeof(leds); i++){
		if (command - 1 == i){
			digitalWrite(leds[i], potential);
		}
		else{
			digitalWrite(leds[i], 0);
		}
	}

	return;
}

void mutate_secret(byte *modifier_arr, ulong* secret ) {

	ulong modifier;
	for (int i=0; i < 4; i++) modifier += (ulong)modifier_arr[i] << i*8;

	*secret ^= modifier;
	return;
}