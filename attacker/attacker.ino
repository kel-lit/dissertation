#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

typedef unsigned char byte;

RF24 radio(7, 8); // CE, CSN
const byte addresses[][6] = {"00001", "00002"};

int Delay = 40; //Milliseconds

void setup() {

        Serial.begin(9600);

        radio.begin();
        radio.openReadingPipe(1, addresses[1]); // 00002
        radio.setPALevel(RF24_PA_MIN);

}

void loop() {

    radio.openReadingPipe(1, addresses[1]); // 00002
        radio.setPALevel(RF24_PA_MIN);
        delay(Delay);
        radio.startListening();

    byte incoming[5];

    if (radio.available()) {
        radio.read(&incoming, sizeof(incoming));
        for (int i=0; i < 5; i++){Serial.print(incoming[i]); Serial.print(",");}Serial.println("");
        if (incoming[0] != 0 & incoming[1] != 0){
            delay(Delay);
            radio.stopListening();
            radio.closeReadingPipe(1);
            radio.openWritingPipe(addresses[1]);
            delay(200); // Wait 200 milliseconds and replay
            for (int i = 0; i < 3; i++){ // Replay the payload 3 times
                Serial.println("Replaying");
                radio.write(&incoming, sizeof(incoming));
            }
        }
    }
}