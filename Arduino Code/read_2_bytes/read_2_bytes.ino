#include <Servo.h>
#include "crc_8.h"

crc_8 crc8;
Servo panServo;  // create servo object to control a servo
Servo tiltServo;

/*** vars for Servo ***/
int tiltPos = 110;    // variable to store the servo position
int panPos = 90;      // variable to store the servo position
int laserPin = 13;

/*** vars for serial state ***/
int state, cntr = 0;
bool read_in_prog;        // Denotes that a read is currently being performed
byte inBytes [3];   
int in_byte;
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
  handle_serial_info();
}

/***
 * Func: handle_serial_info
 * params: null
 * Purpose: handles information from serial as it becomes available (on a byte per byte basis)
 *          function handles state in a rudimentary fashion through switch case. Handle each byte
 *          input as it comes through, and update variables as necessary once CRC checks out.
 *         
 */
void handle_serial_info()
{
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
      input_buffer_flush();
    }
    else {
      //Serial.println("CRC MATCH! " + String(inBytes[2], HEX));
      // Print out our new positions
      inByte0 = conv_unsignedbyte_signedint(inBytes[0]);
      inByte1 = conv_unsignedbyte_signedint(inBytes[1]);
      
      panPos = panPos - inByte0;
      tiltPos = tiltPos + inByte1;
      // Subtract 100 from each number we got and update the new positions
      //panPos = panPos - (int(inBytes[0]));
      //tiltPos = tiltPos + (int(inBytes[1]));
      //Serial.println("RECEIVED BYTE 0: " + String(inByte0, DEC));
      //Serial.println("RECEIVED BYTE 1: " + String(inByte1, DEC));
      //Serial.println("New panPos: "      + String(panPos, DEC));
      //Serial.println("New tiltPos: "     + String(tiltPos, DEC));
      
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

/***
 * Will convert input byte to an int and return it
 * Function assumes that input byte is free of errors, therefore put this after CRC
 */
int conv_unsignedbyte_signedint(byte unsigned_byte) {
  int signed_byte;
  if((unsigned_byte >> 7) & 1u) {
    signed_byte = -1*((~(unsigned_byte) + 1)&0xFF);
  }
  else {
    signed_byte = unsigned_byte&0xFF;
  }
  return signed_byte;
}

void input_buffer_flush() {
  while(Serial.available()) {
    Serial.read();
  }
}
