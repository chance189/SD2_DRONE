#include <Servo.h>

// Read in two 8 bit words 
// one for x angle
// one for y angle

Servo panServo;  
Servo tiltServo;
char angles[2]; 
int laserPin = 13;
int tiltCenter = 22; // Center for tilt servo
int panCenter = 90;   // Center for pan servo

void setup() {
  // Begin the Serial at 9600 Baud
  Serial.begin(9600);
  pinMode(laserPin, OUTPUT);  

  // Set the servos to the center
  tiltServo.write(tiltCenter);
  panServo.write(panCenter);
}

void loop() {

  if(Serial.available()){

    // Read this crap in
    Serial.readBytes(angles, 2);

    // This assumes x angle is in 0
    Serial.println("x_angle:");
    Serial.print(angles[0]);
    
    // This assumes y angle is in 1
    Serial.println("y_angle:");
    Serial.print(angles[1]);
    
    tiltServo.write(angles[1]);
    panServo.write(angles[0]);

    
    digitalWrite(laserPin, HIGH);
    delay(1000); 
    digitalWrite(laserPin, LOW);
  }

  else{
    Serial.println("We don't got nothing...");
  }

}
