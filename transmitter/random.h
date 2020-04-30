/*Code taken from 
https://gist.githubusercontent.com/bloc97/b5831977ccfeae3aa71976686c9c8afa/raw/20a7b33ef983274e4171094c1e7e8c5d4f30894d/Method1.ino
and modified slightly.
Originally created by bloc97 on GitHub: 
*/

const int waitTime = 8;

byte lastByte = 0;

byte leftStack = 0;
byte rightStack = 0;

byte rotate(byte b, int r) {
  return (b << r) | (b >> (8-r));
}

void pushLeftStack(byte bitToPush) {
  leftStack = (leftStack << 1) ^ bitToPush ^ leftStack;
}
void pushRightStackRight(byte bitToPush) {
  rightStack = (rightStack >> 1) ^ (bitToPush << 7) ^ rightStack;
}

byte getTrueRotateRandomByte() {
  byte finalByte = 0;
  
  byte lastStack = leftStack ^ rightStack;
  
  for (int i=0; i<4; i++) {
    delayMicroseconds(waitTime);
    byte leftBits = analogRead(1);
    
    delayMicroseconds(waitTime);
    byte rightBits = analogRead(1);
    
    finalByte ^= rotate(leftBits, i);
    finalByte ^= rotate(rightBits, 7-i);
    
    for (int j=0; j<8; j++) {
      byte leftBit = (leftBits >> j) & 1;
      byte rightBit = (rightBits >> j) & 1;
  
      if (leftBit != rightBit) {
        if (lastStack % 2 == 0) {
          pushLeftStack(leftBit);
        } else {
          pushRightStackRight(leftBit);
        }
      }
    }
    
  }
  lastByte ^= (lastByte >> 3) ^ (lastByte << 5) ^ (lastByte >> 4);
  lastByte ^= finalByte;
  
  return lastByte ^ leftStack ^ rightStack;
}