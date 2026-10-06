#include <ESP8266WiFi.h>
#include <espnow.h>

// MAC Address Board Penerima (Punya riri)
uint8_t receiver1[] = {0xBC, 0xDD, 0xC2, 0x7A, 0x23, 0xD2};

// Struktur data yang mau dikirim
typedef struct struct_pesan {
  int perintahId;
  int nilaiParameter;
} struct_pesan;

struct_pesan paketKirim;
unsigned long previousMillis = 0;
const long interval = 2000; // Kirim data setiap 2 detik

// Callback saat data selesai dikirim
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Status Kirim: ");
  if (sendStatus == 0) {
    Serial.println("Berhasil Diterima Temanmu!");
  } else {
    Serial.println("Gagal (Tidak Terjangkau)");
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  if (esp_now_init() != 0) {
    Serial.println("Gagal Inisialisasi ESP-NOW!");
    return;
  }

  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_register_send_cb(OnDataSent);
  
  // Daftarkan Node rir sebagai peer
  esp_now_add_peer(receiver1, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
  Serial.println("Controller ESP-NOW Siap Mengirim!");
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    paketKirim.perintahId = 101;
    paketKirim.nilaiParameter = random(10, 100); // Angka acak

    // Kirim data ke riri
    esp_now_send(receiver1, (uint8_t *) &paketKirim, sizeof(paketKirim));
  }
}