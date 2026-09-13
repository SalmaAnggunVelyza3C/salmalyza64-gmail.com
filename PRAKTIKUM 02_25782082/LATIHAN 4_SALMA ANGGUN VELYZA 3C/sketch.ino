const byte ldrPin = 36;
const byte ledPin = 13;

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
}

void loop() {
  int nilaiLdr = analogRead(ldrPin);

  Serial.print("Nilai ADC LDR: ");
  Serial.println(nilaiLdr);


  if (nilaiLdr > 2100) { 
    digitalWrite(ledPin, HIGH);
    Serial.println("Kondisi: GELAP -> LED ON");
  } else {
    digitalWrite(ledPin, LOW);
    Serial.println("Kondisi: TERANG -> LED OFF");
  }

  delay(1000);
}