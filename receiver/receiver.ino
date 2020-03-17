#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(7, 8); // CE, CSN
const byte addresses[][6] = {"00001", "00002"};
boolean buttonState = 0;
int button = 3;
int led = 4;

unsigned long listen_for_secret() {

	unsigned long secret = 0;

	delay(5);
	radio.startListening();

	while (!radio.available());
	radio.read(&secret, sizeof(secret));
	char buffer[80];
	sprintf(buffer, "Secret recieved: %lu", secret);
	Serial.println(buffer);

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
	pinMode(led, OUTPUT);

	unsigned long secret = listen_for_secret();
}

void loop() {

	delay(5);
	radio.startListening();

	if ( radio.available()) {
		while (radio.available()) {
		radio.read(&buttonState, sizeof(buttonState));
		if (buttonState == "HIGH") {
			digitalWrite(led, HIGH);
			}
		else {
			digitalWrite(led, LOW);
		}
	}

	delay(5);
	radio.stopListening();
	int value = digitalRead(button);
	radio.write(&value, sizeof(value));
	}
}