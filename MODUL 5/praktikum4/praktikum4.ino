#include <painlessMesh.h>
#include <DHT.h>
#include <ArduinoJson.h>

#define MESH_PREFIX     "LabIoTMesh"      // Nama jaringan Mesh bersama
#define MESH_PASSWORD   "iotmeshpassword" // Kata sandi jaringan Mesh
#define MESH_PORT       5555              // Port komunikasi TCP Mesh

// Konfigurasi pin dan tipe sensor DHT
#define DHTPIN          2                 // Pin D4 (GPIO 2)
#define DHTTYPE         DHT22             // Sensor DHT22
DHT dht(DHTPIN, DHTTYPE);

const char *nodeName = "Node-1"; 

Scheduler userScheduler;
painlessMesh mesh;

void sendMessage();
void receivedCallback(uint32_t from, String &msg);
void newConnectionCallback(uint32_t nodeId);

Task taskSendMessage(TASK_SECOND * 5, TASK_FOREVER, &sendMessage);

void sendMessage() {
  float suhu = dht.readTemperature();
  float kelembapan = dht.readHumidity();

  if (isnan(suhu) || isnan(kelembapan)) {
    Serial.println("Gagal membaca sensor DHT!");
    return;
  }

  StaticJsonDocument<200> jsonBuffer;
  jsonBuffer["node"] = nodeName;
  jsonBuffer["suhu"] = suhu;
  jsonBuffer["kelembapan"] = kelembapan;
  
  String msg;
  serializeJson(jsonBuffer, msg);
  
  mesh.sendBroadcast(msg);
  Serial.println("Data Node 1 Terkirim: " + msg);
}

void receivedCallback(uint32_t from, String &msg) {
  Serial.printf("Pesan diterima dari node %u: %s\n", from, msg.c_str());
}

void newConnectionCallback(uint32_t nodeId) {
  Serial.printf("Koneksi baru terdeteksi, nodeId: %u\n", nodeId);
}

void setup() {
  Serial.begin(115200);
  dht.begin();

  mesh.setDebugMsgTypes(ERROR | STARTUP);  
  mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT);
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);

  userScheduler.addTask(taskSendMessage);
  taskSendMessage.enable();
}

void loop() {
  mesh.update();
}