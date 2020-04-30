#include <stdio.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

typedef unsigned char byte;
typedef unsigned long ulong;
typedef unsigned int uint;

RF24 radio(7, 8); // CE, CSN
const byte addresses[][6] = {"00001", "00002"};

int button1 = 2;
int button2 = 3;
int leds[1] = {4};

int Delay = 5;

void setup() {

	Serial.begin(9600);

	radio.begin();
	radio.openWritingPipe(addresses[1]); // 00002
	radio.openReadingPipe(1, addresses[0]); // 00001
	radio.setPALevel(RF24_PA_MIN);

	pinMode(button1, INPUT_PULLUP);
	pinMode(button2, INPUT_PULLUP);
	pinMode(leds[0], OUTPUT);

}

void loop() {

	delay(Delay);
	radio.stopListening();

	byte command;
	byte potential;
	uint payload = 0;

	get_command(&command, &potential); //Get control inputs
	
	payload = create_payload(command, potential);

	radio.write(&payload, sizeof(payload));

	uint incoming;

    delay(Delay);
    radio.startListening();

	while(!radio.available());
	radio.read(&incoming, sizeof(incoming));
    Serial.println(incoming);

    potential = incoming;
    command = incoming >> 8;

    if (command == 1){
        digitalWrite(leds[command-1], potential);
    }
    else {
        digitalWrite(leds[0], 0);
    }
    
}

void get_command(byte* command, byte* potential){
	// This function gets the command and potential from physical controls
	byte value = 0;
	value += (digitalRead(button1) == HIGH) ? 0 : 1;
	value += ((digitalRead(button2) == HIGH) ? 0 : 1) << 1;
	
	*command = value;
	*potential = value ? 255 : 0;

	return;
}

uint create_payload(byte command, byte potential) {

	uint payload;

	payload += (uint)potential;
	payload += (uint)command << 8;

	return payload;
}
