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
  panServo.attach(9);
  tiltServo.attach(8);

  // Set initial servo posistions
  tiltServo.write(tiltPos);
  panServo.write(panPos);

  // Set the laser pin to low initially
  digitalWrite(laserPin, LOW);

  //Initalize Timer for interrupt
  cli();      //stop interrutps
  TCCR0A = 0; //set associated timer regs to 0
  TCNT0  = 0; //^^
  OCR0A = 255;                          // = 16MHz/1024(prescaler) - 1
  TCCR0B = (1 << WGM02);                // turn on CTC mode
  TCCR0B |= (1 << CS02) | (1 << CS00);  // Set CS02 and CS00 bits for 1028 (pg169) of atmega datasheet (this prescaler is same for timer 1,3,4)
  TIMSK0 &= ~(1 << OCIE0A);             // disable the timer interrupt
  sei();                                //enable interrupts
  Serial.println("BOOTUP FINISHED");
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
    Serial.println("BAD CRC MATCH!!! RECV: " + String(inBytes[2], HEX));
    Serial.println("RECV: " + String(inBytes[0], HEX) + " " + String(inBytes[1], HEX));
    //delay(100);
    //input_buffer_flush();
    //Serial.write("~");      //Nack, denoting master should not send any info
    //delay(200);             //300ms delay    
    //input_buffer_flush();   //flush all input buffer
    //Serial.write("^");      //Write ready for resetting comms
  }
  else {
    //Serial.println("CRC MATCH! " + String(inBytes[2], HEX));
    // Print out our new positions
    //For firing the laser:
    if(inBytes[0] == 0x7A && inBytes[1] == 0x86 && int_curr_val == 0) {
      digitalWrite(laserPin, HIGH);
      cli();      //stop interrutps
      TCCR0A = 0; //set associated timer regs to 0
      TCNT0  = 0; //^^
      TIMSK0 |= (1 << OCIE0A);             // enable the timer interrupt
      sei();      //enable interrupts
    }
    else {
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
    Serial.write("*");
  }

  // Send Ack
  //Serial.write("*");
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

/***
 * Timer vector handler
 */
ISR(TIMER0_COMPA_vect) {
  if(int_curr_val == 61) {
    cli();      //stop interrutps
    TIMSK0 &= ~(1 << OCIE0A);             // disable the timer interrupt
    sei();      //enable interrupts
    digitalWrite(laserPin, LOW);
    int_curr_val = 0;
  }
  else {
    int_curr_val+=1;
    //Serial.println("counter incr: " + String(int_curr_val, DEC));
  }
}
void input_buffer_flush() {
  while(Serial.available()) {
    Serial.read();
  }
}
