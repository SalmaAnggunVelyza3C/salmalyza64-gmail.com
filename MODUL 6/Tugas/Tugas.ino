#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <DHT.h>

// Kredensial Jaringan Wi-Fi
const char* ssid = "Kost 3 Putri 3";
const char* password = "tigaputri3";

// Konfigurasi Broker MQTT Publik
const char* mqttServer = "broker.hivemq.com";
const int mqttPort = 1883;
const char* clientId = "ESP8266_LabIoT_25782082_Task";

// Definisi Topik Mandiri (Ganti NIM Anda)
const char* topicPublish   = "kampus/labiot/25782082/telemetri";
const char* topicSubscribe = "kampus/labiot/25782082/kendali";
const char* topicAlert     = "kampus/labiot/25782082/alert";
const char* topicStatus    = "kampus/labiot/25782082/status";

// Alokasi Pin Perangkat Keras
const byte dhtPin = 2;   // Pin D4 (GPIO 2)
const byte ledPin = 5;   // Pin D1 (GPIO 5)
#define DHTTYPE DHT11

DHT dht(dhtPin, DHTTYPE);
WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastTelemetryTime = 0;
const unsigned long telemetryInterval = 4000; // Siklus kirim 4 detik
bool alertSentFlag = false; // Flag agar alert tidak spamming

// Fungsi Callback untuk Menerima Perintah Kendali dari Operator
void callback(char* topic, byte* payload, unsigned int length) {
  char incomingBuffer[length + 1];
  memcpy(incomingBuffer, payload, length);
  incomingBuffer[length] = '\0';

  Serial.println("----------------------------------------");
  Serial.printf("[KONTROL DITERIMA] Topik: %s | Payload: %s\n", topic, incomingBuffer);

  StaticJsonDocument<256> doc;
  DeserializationError err = deserializeJson(doc, incomingBuffer);
  if (err) {
    Serial.println("Gagal membedah payload JSON!");
    return;
  }

  // Kendali manual hanya diizinkan jika suhu normal (tidak sedang overheat darurat)
  float currentTemp = dht.readTemperature();
  if (currentTemp <= 31.0) {
    if (doc.containsKey("led")) {
      int val = doc["led"];
      digitalWrite(ledPin, val == 1 ? HIGH : LOW);
      Serial.printf("--> [MANUAL OVERRIDE] Status LED diubah: %s\n", val == 1 ? "MENYALA" : "PADAM");
    }
  } else {
    Serial.println("--> [BLOCKED] Kendali manual diabaikan! Sistem dalam status BAHAYA/PANAS.");
  }
  Serial.println("----------------------------------------");
}

// Fungsi Koneksi Ulang MQTT dengan Konfigurasi LWT
void reconnect() {
  while (!client.connected()) {
    Serial.print("Menghubungkan kembali ke Broker...");
    
    // Konfigurasi Last Will and Testament (LWT)
    // Format: client.connect(clientId, willTopic, willQos, willRetain, willMessage)
    bool connected = client.connect(
      clientId, 
      topicStatus, 1, true, 
      "{\"status\": \"OFFLINE\", \"keterangan\": \"Perangkat Terputus Tak Terduga\"}"
    );

    if (connected) {
      Serial.println("Sukses Terhubung!");
      
      // Publikasikan status ONLINE sesaat setelah terhubung
      client.publish(topicStatus, "{\"status\": \"ONLINE\", \"keterangan\": \"Perangkat Beroperasi Normal\"}", true);
      
      // Daftarkan kembali subskripsi topik kendali
      client.subscribe(topicSubscribe);
      Serial.printf("Subskripsi Topik: %s\n", topicSubscribe);
    } else {
      Serial.printf("Gagal (rc=%d). Coba lagi dalam 4 detik...\n", client.state());
      delay(4000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  dht.begin();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Menghubungkan Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi Terkoneksi!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop(); 

  unsigned long now = millis();
  if (now - lastTelemetryTime >= telemetryInterval) {
    lastTelemetryTime = now;
    
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    
    if (!isnan(t) && !isnan(h)) {
      // Tentukan kondisi lingkungan berdasarkan ambang batas 31.0°C
      String kondisiSuhu = (t > 31.0) ? "PANAS" : "NORMAL";

      // 1. Kirim Telemetri Rutin Berformat JSON
      StaticJsonDocument<256> docOut;
      docOut["node_id"] = clientId;
      docOut["suhu"] = serialized(String(t, 2));
      docOut["kelembapan"] = serialized(String(h, 2));
      docOut["status_led"] = digitalRead(ledPin);
      docOut["kondisi_suhu"] = kondisiSuhu;
      
      char buffer[256];
      serializeJson(docOut, buffer);
      client.publish(topicPublish, buffer);
      Serial.printf("[TELEMETRI TERKIRIM] %s\n", buffer);

      // 2. Logika Edge Safety Engine (Kanal Alert Mandiri & Otomasi LED)
      if (t > 31.0) {
        digitalWrite(ledPin, HIGH); // Nyalakan LED otomatis tanda bahaya
        
        // Kirim alert sekali saja saat pertama kali masuk kondisi panas (menggunakan flag)
        if (!alertSentFlag) {
          StaticJsonDocument<256> alertDoc;
          alertDoc["status"] = "BAHAYA";
          alertDoc["parameter"] = "OVERHEAT";
          alertDoc["nilai_suhu"] = serialized(String(t, 2));
          
          char alertBuffer[256];
          serializeJson(alertDoc, alertBuffer);
          client.publish(topicAlert, alertBuffer, true);
          
          Serial.printf("[WARNING ALERT] Suhu Overheat! Terkirim ke %s\n", topicAlert);
          alertSentFlag = true; // Set flag aktif
        }
      } else {
        // Reset flag jika suhu sudah kembali normal
        if (alertSentFlag) {
          alertSentFlag = false;
          Serial.println("[INFO] Suhu kembali normal. Sistem pulih.");
        }
      }
    }
  }
}