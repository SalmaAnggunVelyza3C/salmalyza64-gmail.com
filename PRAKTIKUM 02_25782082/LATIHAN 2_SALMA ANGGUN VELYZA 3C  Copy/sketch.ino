const byte ldrPin = A0;

void setup() {
  Serial.begin(115200);
}

void loop() {
  int ldrValue = analogRead(ldrPin);
  

  float persentase = (ldrValue / 1023.0) * 100.0;

  Serial.print("Intensitas Cahaya: ");
  Serial.print(persentase);
  Serial.println("%");

  delay(1000);
}