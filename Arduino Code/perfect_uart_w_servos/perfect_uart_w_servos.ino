#include <Servo.h>

Servo panServo;  // create servo object to control a servo
Servo tiltServo;

int tiltPos = 110;    // variable to store the servo position
int panPos = 90;    // variable to store the servo position
int laserPin = 13;
int toggle = 1;
char inByte [2];
int tiltNum = 1;
int panNum = 1;

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
}

void loop() {
  // put your main code here, to run repeatedly:
  if(Serial.available()){
    Serial.readBytes(inByte, 1);
    Serial.println(inByte);
    
    if(strcmp(inByte, "u") == 0) {
      
      Serial.println("Going up by " + tiltNum);
      tiltPos = tiltPos + tiltNum;
      tiltServo.write(tiltPos); 
    }
    else if(strcmp(inByte, "d") == 0) {
      
      Serial.println("Going down by "+ tiltNum);
      tiltPos = tiltPos - tiltNum;
      tiltServo.write(tiltPos); 
    }
    else if(strcmp(inByte, "l") == 0) {
  
      Serial.println("Going left by " + panNum);
      panPos = panPos + panNum;
      panServo.write(panPos); 
    }  
    else if(strcmp(inByte, "r") == 0) {
  
      Serial.println("Going right by " + panNum);
      panPos = panPos - panNum;
      panServo.write(panPos); 
    }
    else{
      Serial.println("We recieved...");
      Serial.println("inByte");
    }
  }
  Serial.println("We ain't got shit");
}
