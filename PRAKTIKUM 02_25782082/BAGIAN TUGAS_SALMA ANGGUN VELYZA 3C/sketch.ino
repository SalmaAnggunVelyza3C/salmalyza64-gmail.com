#include <DHT.h>

#define DHTPIN 13
#define DHTTYPE DHT22

const byte ldrPin   = 36; // Sudah pas sesuai diagram.json (Pin D36)
const byte relayPin = 12;
const byte ledPin   = 5;  // Pakai D5, sudah terpisah dari D13

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  delay(1000);
  dht.begin();

  pinMode(relayPin, OUTPUT);
  pinMode(ledPin, OUTPUT);

  digitalWrite(relayPin, HIGH);
  digitalWrite(ledPin, LOW);

  analogReadResolution(10);
}

void loop() {
  delay(2000);

  float temp   = dht.readTemperature();
  int ldrValue = analogRead(ldrPin);

  if (isnan(temp)) {
    Serial.println("Gagal membaca dari sensor DHT!");
    return;
  }

  Serial.print("Suhu: ");
  Serial.print(temp);
  Serial.print(" °C | LDR ADC: ");
  Serial.println(ldrValue);

  
  if (temp > 34.0 || ldrValue < 500) {
    digitalWrite(relayPin, LOW);
    digitalWrite(ledPin, HIGH);
    Serial.println("Peringatan: Aktuator Aktif!");
  } else {
    digitalWrite(relayPin, HIGH);
    digitalWrite(ledPin, LOW);
    Serial.println("Kondisi Aman");
  }
}