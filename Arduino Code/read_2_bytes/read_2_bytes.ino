#include <Servo.h>
#include "crc_8.h"

crc_8 crc8;
Servo panServo;  // create servo object to control a servo
Servo tiltServo;

int tiltPos = 110;    // variable to store the servo position
int panPos = 90;    // variable to store the servo position
int laserPin = 13;
byte inBytes [3];
int inByte0, inByte1;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(laserPin, OUTPUT);
  panServo.attach(9);
  tiltServo.attach(8);

  // Set initial servo posistions
  tiltServo.write(tiltPos);
  panServo.write(panPos);

  // Turn laser on
  digitalWrite(laserPin, HIGH);
  Serial.println("BOOTUP FINISHED");
}

void loop() {
  
  if (Serial.available()) {

    while (Serial.available() < 3) {} // Wait 'till there are 3 Bytes waiting
    // Once it's ready read it in...
    Serial.readBytes(inBytes, 3);
    crc8.reset();
    crc8.next(inBytes[0]);
    crc8.next(inBytes[1]);

    if(crc8.get_crc() != inBytes[2])  //CRC Mismatch
    {
      Serial.println("BAD CRC MATCH!!! RECV: " + String(inBytes[2], HEX));
    }
    else {
      Serial.println("CRC MATCH! " + String(inBytes[2], HEX));
      // Print out our new positions
      if((inBytes[0] >> 7) & 1u) {
        Serial.println("DEBUGGIN BYTE0: " + String(inBytes[0], BIN));
        inByte0 = -1*((~(inBytes[0])+ 1)&0xFF);
        Serial.println("DEBUGGIN BYTE0 AFTER: " + String(inByte0, BIN));
      }
      else {
        inByte0 = inBytes[0]&0xFF;
      }
  
      if((inBytes[1] >> 7) & 1u) {
        Serial.println("DEBUGGIN BYTE1: " + String(inBytes[1], BIN));
        inByte1 = -1*((~(inBytes[1]) + 1)&0xFF);
        Serial.println("DEBUGGIN BYTE1 AFTER: " + String(inByte1, BIN));
      }
      else {
        inByte1 = inBytes[1]&0xFF;
      }
      
      panPos = panPos - inByte0;
      tiltPos = tiltPos + inByte1;
      // Subtract 100 from each number we got and update the new positions
      //panPos = panPos - (int(inBytes[0]));
      //tiltPos = tiltPos + (int(inBytes[1]));
      Serial.println("RECEIVED BYTE 0: " + String(inByte0, DEC));
      Serial.println("RECEIVED BYTE 1: " + String(inByte1, DEC));
      Serial.println("New panPos: "      + String(panPos, DEC));
      Serial.println("New tiltPos: "     + String(tiltPos, DEC));
  
      // Check if we need to FIRE!!
      if (inBytes[0] == 200 & inBytes[1] == 200) {
  
        // FIRE THE BASS CANNON
        Serial.println("FIRE");
        digitalWrite(laserPin, HIGH);
        delay(1000);
        digitalWrite(laserPin, LOW);
  
        // Set the servos back to the ready state
        tiltPos = 110;
        panPos = 90;
        tiltServo.write(tiltPos);
        panServo.write(panPos);
  
        // Send confirmation
        Serial.write("g");
      }
  
      // Move servos to next position
  
      Serial.println("Panning by: " + String(panPos, DEC));
      tiltServo.write(tiltPos);
      Serial.println("Tilting to: " + String(tiltPos, DEC));
      panServo.write(panPos);
    }

    // Send Ack
    Serial.write("*");
  }
}
