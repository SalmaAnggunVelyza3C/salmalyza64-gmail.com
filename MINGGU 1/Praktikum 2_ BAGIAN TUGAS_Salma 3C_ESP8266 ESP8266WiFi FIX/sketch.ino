const int buttonPin = 4;
const int ledPin = 5;

int lastButtonState = LOW;
bool ledState = false;

void setup() {
  Serial.begin(115200);

  // Menggunakan INPUT_PULLDOWN internal ESP32
  pinMode(buttonPin, INPUT_PULLDOWN);
  pinMode(ledPin, OUTPUT);

  digitalWrite(ledPin, LOW);
  Serial.println("Sistem Siap. Status LED: OFF");
}

void loop() {
  int currentButtonState = digitalRead(buttonPin);

  // Deteksi tombol baru saja ditekan
  if (currentButtonState == HIGH && lastButtonState == LOW) {
    ledState = !ledState;
    digitalWrite(ledPin, ledState ? HIGH : LOW);

    if (ledState) {
      Serial.println("Tombol ditekan -> LED ON");
    } else {
      Serial.println("Tombol ditekan -> LED OFF");
    }

    // Tahan proses sampai tombol bener-bener dilepas
    while (digitalRead(buttonPin) == HIGH) {
      delay(10);
    }
  }

  lastButtonState = currentButtonState;
}