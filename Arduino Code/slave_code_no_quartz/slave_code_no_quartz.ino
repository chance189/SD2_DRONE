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
int cntr = 0;
bool read_in_prog = false;        // Denotes that a read is currently being performed
byte inBytes [3];   
int inByte0, inByte1;
volatile int int_curr_val = 0;     //lets us know how many times we've rolled over
byte start_byte = 0x7F;
byte end_byte = 0x80;

void setup() {
  Serial.begin(9600);
  pinMode(laserPin, OUTPUT);
  panServo.attach(2);
  tiltServo.attach(3);

  // Set initial servo posistions
  tiltServo.write(tiltPos);
  panServo.write(panPos);

  // Set the laser pin to low initially
  digitalWrite(laserPin, HIGH);
}

void loop() {
  handle_serial_info();
}

/***
 * Func: handle_serial_info
 * params: null
 * Purpose: handles information from serial as it becomes available. A start byte is used to determine
 *      if in the right state. If the byte is invalid, then keep polling serial. If valid input comes
 *      through, then the CRC is checked (3rd byte, or byte 2). Bad CRC means no dice.
 *      God have mercy on my tainted soul. The whispers... They tell me my timers will fail me... The horror
 *         
 */
void handle_serial_info()
{
  if(Serial.available() > 0) {
    byte in_byte = Serial.read();

    if(in_byte == end_byte) {
      read_in_prog = false;
      parse_rx();
    }

    if(read_in_prog) {  
      inBytes[cntr] = in_byte;
      cntr = cntr == 2 ? cntr : cntr + 1;
    }

    if(in_byte == start_byte) {
      cntr = 0;
      read_in_prog = true;
    }
  }
}

void parse_rx() {
  //Reset the CRC and recalculate it
  crc8.reset();
  crc8.next(inBytes[0]);
  crc8.next(inBytes[1]);

  //Check the CRC
  if(crc8.get_crc() != inBytes[2])  //CRC Mismatch
  {
    Serial.write("*");        //Even though nack, we should report back
  }
  else {
    //For firing the laser:
    if(inBytes[0] == 0x7A && inBytes[1] == 0x86) {
      Serial.write("*");
    }
    else {
      inByte0 = conv_unsignedbyte_signedint(inBytes[0]);
      inByte1 = conv_unsignedbyte_signedint(inBytes[1]);
      
      panPos = panPos - inByte0;
      tiltPos = tiltPos + inByte1;
      
      // Move servos to next position
      tiltServo.write(tiltPos);
      panServo.write(panPos);
      
    }
    Serial.write("*");    //Write back ack
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
