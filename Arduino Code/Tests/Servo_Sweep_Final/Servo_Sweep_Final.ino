
#include <Servo.h>


Servo panServo;  // create servo object to control a servo
Servo tiltServo;

int tiltPos = 0;    // variable to store the servo position
int panPos = 0;    // variable to store the servo position
int laserPin = 13;

// For serial connection
int test;

void setup() {

   // Begin the Serial at 9600 Baud
  Serial.begin(9600);

  pinMode(laserPin, OUTPUT);  
  panServo.attach(9); 
  tiltServo.attach(8); 

  attachInterrupt(digitalPinToInterrupt(2),gotInterupt,RISING); // Attach pin 2 as our interupt
}

void loop() {
  Serial.println("Serial is open");
  servoSweep();
}

void servoSweep(){

    // Move tileServo
  for (tiltPos = 90; tiltPos <= 135; tiltPos += 5) { 

    tiltServo.write(tiltPos); 
    
    // Move panServo
    for (panPos = 0; panPos <= 180; panPos += 1) {
      //digitalWrite(laserPin, HIGH); 
      panServo.write(panPos); 
      delay(25);
      
    }

    tiltPos +=5;
    tiltServo.write(tiltPos);
    
    // Move panServo
    for (panPos = 180; panPos >= 0; panPos -= 1) { 
      panServo.write(panPos); 
      delay(25); 
    }
  }
}

void checkSerial(){

    if(Serial.available())
   {
    char test = Serial.read();

    if (test==1){
      Serial.println("Recieved a 1");
      digitalWrite(laserPin, HIGH); 
      delay(5000); // Delay in ms
      digitalWrite(laserPin, LOW); 
      
    }
   }
   Serial.println("Nothing avaiable");
}

void gotInterupt(){

  int check = 0;
  
  while(check == 0) {

    int test = Serial.read();
    String terminalText = Serial.readStringUntil('\n');

    if (test==255){
      Serial.println("Recieved a 1");
      digitalWrite(laserPin, HIGH); 
      delay(5000); // Delay in ms
      digitalWrite(laserPin, LOW);
      check = 1;
      Serial.write(1); 
    }
    else {
      Serial.write(test);
    }

    
  }
  
}
