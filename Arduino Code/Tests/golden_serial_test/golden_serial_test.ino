
char input[7];
char in;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("BOOTUP FINISHED");
}

void loop() {
  // Below segment can test for loopback and validity of TX/RX of board
  if(Serial.available() > 0 ) {
    in = Serial.read();
    Serial.println(in);
  }
  //Serial.write("T");
}
