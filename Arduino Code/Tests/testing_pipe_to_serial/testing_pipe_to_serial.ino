byte input_bytes [2];

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial1.begin(115200);
  Serial.println("BOOTUP FINISHED");
}

void loop() {
  // put your main code here, to run repeatedly:
  if(Serial1.available() >= 2) { //wait for input bytes
    Serial1.readBytes(input_bytes, 2); 
    Serial.println("\nGOT INFORMATION:");
    Serial.println("input_byte[0]: "+String(input_bytes[0], DEC));
    Serial.println("input_byte[1]: "+String(input_bytes[1], DEC));
  }
}
