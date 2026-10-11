#include <painlessMesh.h>
#include <ArduinoJson.h>

#define MESH_PREFIX     "LabIoTMesh"      
#define MESH_PASSWORD   "iotmeshpassword" 
#define MESH_PORT       5555

const char* nodeName = "Node-2-Cahaya"; 

Scheduler userScheduler;
painlessMesh mesh;

void sendMessage();
Task taskSendMessage(TASK_SECOND * 3, TASK_FOREVER, &sendMessage);

void sendMessage() {
  // Membaca nilai analog dari pin AO modul LDR
  int nilaiLDR = analogRead(A0);

  // Bungkus data ke format JSON sesuai modul
  StaticJsonDocument<200> doc;
  doc["tipe"] = "cahaya_node";
  doc["adc"] = nilaiLDR;

  String msg;
  serializeJson(doc, msg);
  
  // Kirim data ke jaringan mesh
  mesh.sendBroadcast(msg);

  Serial.print("[KIRIM MESH - LDR] ");
  Serial.println(msg);
}

void receivedCallback(uint32_t from, String &msg) {
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, msg);
  
  if (!error) {
    const char* tipe = doc["tipe"];
    Serial.printf("[TERIMA MESH] Dari Node: %u | Tipe: %s\n", from, tipe);
  } else {
    Serial.printf("[TERIMA MENTAH DARI %u]: %s\n", from, msg.c_str());
  }
}

void newConnectionCallback(uint32_t nodeId) {
  Serial.printf("--> Koneksi Baru Terdeteksi! Node ID: %u\n", nodeId);
}

void changedConnectionCallback() {
  Serial.println("--> Topologi rantai mesh telah diperbarui");
}

void setup() {
  Serial.begin(115200);

  mesh.setDebugMsgTypes(ERROR | STARTUP);
  mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT);
  
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);

  userScheduler.addTask(taskSendMessage);
  taskSendMessage.enable();

  Serial.printf("Node LDR [%s] Berjalan...\n", nodeName);
}

void loop() {
  mesh.update();
}