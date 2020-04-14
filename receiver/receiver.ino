#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

typedef unsigned long ulong;
typedef unsigned int uint;

RF24 radio(7, 8); // CE, CSN
const byte addresses[][6] = {"00001", "00002"};
ulong incoming;
int button = 3;
int leds[3] = {6,4,5};

int Delay = 130; //Milliseconds

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

	pinMode(2, OUTPUT);
	digitalWrite(2, HIGH);
		
	pinMode(button, INPUT);
	for (int i; i < sizeof(leds); i++){
		pinMode(leds[i], OUTPUT);
	}

	unsigned long secret = listen_for_secret();
}

void loop() {

	delay(Delay);
	radio.startListening();

	byte command = 0;
	byte potential = 0;
	uint challenge = 0;

	if (radio.available()) {
		radio.read(&incoming, sizeof(incoming));
		incoming ^= secret;

		potential = (ulong)incoming;
		command = (ulong)incoming >> 8; 
		challenge = (ulong)incoming >> 16;

		ulong response = solve_challenge(challenge);
		
		delay(Delay);
		send_ack(response ^ secret);
		
		mutate_secret(&response, &secret);
		Serial.print("New Secret: "); Serial.println(secret);

		for (byte i=0; i < sizeof(leds); i++){
			if (command - 1 == i){
				digitalWrite(leds[i], potential);
			}
			else{
				digitalWrite(leds[i], 0);
			}
		}
	}
	
	int value = digitalRead(button);
	if (value) {
		delay(Delay);
		radio.stopListening();
		int value = digitalRead(button);
		radio.write(&value, sizeof(value));
	}
}

ulong solve_challenge(uint challenge) {
	
	ulong prime = 16777619; //recommended prime and seed from: http://isthe.com/chongo/tech/comp/fnv/
	ulong seed = 2166136261;

	ulong response;

	byte low = challenge & 0xFF;
	byte high = challenge >> 8;

	response += (low ^ seed) * prime;
	response += (high ^ seed) * prime << 8;
	response += (high ^ seed) * prime << 16;
	response += (low ^ seed) * prime << 24;

	return response;
}

void send_ack(ulong ack) {

	radio.stopListening();
	radio.write(&ack, sizeof(ack));
	radio.startListening();
	
	return;
}

void mutate_secret(ulong* modifier, ulong* secret ) {

	*secret ^= *modifier;
	return;
}
