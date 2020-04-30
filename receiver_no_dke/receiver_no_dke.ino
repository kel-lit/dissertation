#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

typedef unsigned int uint;

RF24 radio(7, 8); // CE, CSN
const byte addresses[][6] = {"00001", "00002"};
uint incoming;
int button = 2;
int leds[3] = {6,4,5};

int Delay = 5; //Milliseconds



void setup() {

	Serial.begin(9600);

	radio.begin();
	radio.openWritingPipe(addresses[0]); // 00001
	radio.openReadingPipe(1, addresses[1]); // 00002
	radio.setPALevel(RF24_PA_MIN);

	pinMode(2, INPUT_PULLUP);

	for (int i; i < sizeof(leds); i++){
		pinMode(leds[i], OUTPUT);
	}
}

void loop() {

	delay(Delay);
	radio.startListening();

	byte command = 0;
	byte potential = 0;

	if (radio.available()) {
		radio.read(&incoming, sizeof(incoming));
        Serial.println(incoming);

		potential = incoming;
		command = incoming >> 8;

		for (byte i=0; i < sizeof(leds); i++){
			if (command - 1 == i){
				digitalWrite(leds[i], potential);
			}
			else{
				digitalWrite(leds[i], 0);
			}
		}
	}

    get_command(&command, &potential);
    uint payload = create_payload(command, potential);

    delay(Delay);
    radio.stopListening();
    radio.write(&payload, sizeof(payload));
}

void get_command(byte* command, byte* potential){
	// This function gets the command and potential from physical controls
	byte value = 0;
	value += (digitalRead(button) == HIGH) ? 0 : 1;
	
	*command = value;
	*potential = value ? 255 : 0;

	return;
}

uint create_payload(byte command, byte potential){

    uint payload;

    payload += (uint)potential;
    payload += (uint)command << 8;

    return payload;
}