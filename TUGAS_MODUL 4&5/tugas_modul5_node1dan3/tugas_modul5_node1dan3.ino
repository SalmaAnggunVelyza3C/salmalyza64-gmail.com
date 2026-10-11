void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}

#include <painlessMesh.h>
#include <DHT.h>
#include <ArduinoJson.h>

#define MESH_PREFIX   "LabIoTMesh"
#define MESH_PASSWORD "iotmeshpassword"
#define MESH_PORT     5555

// DHT22
#define DHTPIN  2       // OUT ke D4 (GPIO 2)
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// LED Node 3
#define LED_PIN 12      // D6 (GPIO 12)

const char *nodeName = "Node-1+3";

Scheduler userScheduler;
painlessMesh mesh;

void sendMessage();

Task taskSendMessage(
  TASK_SECOND * 3,
  TASK_FOREVER,
  &sendMessage
);

void sendMessage() {
  float suhu = dht.readTemperature();
  float kelembapan = dht.readHumidity();

  if (isnan(suhu) || isnan(kelembapan)) {
    Serial.println("Gagal membaca DHT22! Cek kabel.");
    return;
  }

  // Rule engine LED berdasarkan suhu
  if (suhu > 31.0) {
    digitalWrite(LED_PIN, HIGH);
    Serial.println("LED MENYALA: suhu > 31 C");
  } else {
    digitalWrite(LED_PIN, LOW);
    Serial.println("LED MATI: suhu normal");
  }

  // Format JSON telemetri
  StaticJsonDocument<200> doc;
  doc["tipe"] = "suhu_node";
  doc["node"] = nodeName;
  doc["suhu"] = suhu;
  doc["kelembapan"] = kelembapan;

  String msg;
  serializeJson(doc, msg);

  // Siarkan data melalui mesh
  mesh.sendBroadcast(msg);

  Serial.println("Data terkirim: " + msg);
}

void receivedCallback(uint32_t from, String &msg) {
  Serial.printf("Pesan dari node %u: %s\n",
                from, msg.c_str());
}

void newConnectionCallback(uint32_t nodeId) {
  Serial.printf("Node terhubung: %u\n", nodeId);
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  dht.begin();

  mesh.setDebugMsgTypes(ERROR | STARTUP);
  mesh.init(
    MESH_PREFIX,
    MESH_PASSWORD,
    &userScheduler,
    MESH_PORT
  );

  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);

  userScheduler.addTask(taskSendMessage);
  taskSendMessage.enable();

  Serial.println("Node gabungan DHT22 + LED siap");
}

void loop() {
  mesh.update();
}