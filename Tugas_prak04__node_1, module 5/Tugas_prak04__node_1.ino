#include <painlessMesh.h>
#include <DHT.h>
#include <ArduinoJson.h>

#define MESH_PREFIX "LabIoTMesh"
#define MESH_PASSWORD "mutiacantik"
#define MESH_PORT 4444

#define DHTPIN 2
#define DHTTYPE DHT11  // Ganti DHT22 jika memakai DHT22

DHT dht(DHTPIN, DHTTYPE);

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
    Serial.println("[ERROR] Gagal membaca sensor DHT");
    return;
  }

  StaticJsonDocument<200> doc;
  doc["tipe"] = "suhu_node";
  doc["suhu"] = suhu;
  doc["kelembapan"] = kelembapan;

  String msg;
  serializeJson(doc, msg);

  mesh.sendBroadcast(msg);

  Serial.print("[KIRIM SUHU] ");
  Serial.println(msg);
}

void receivedCallback(uint32_t from, String &msg) {
  Serial.printf("[PESAN MESH] Dari Node %u: %s\n",
                from, msg.c_str());
}

void newConnectionCallback(uint32_t nodeId) {
  Serial.printf("--> Koneksi Baru: %u\n", nodeId);
}

void changedConnectionCallback() {
  Serial.println("--> Topologi mesh berubah");
}

void setup() {
  Serial.begin(115200);
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
  mesh.onChangedConnections(&changedConnectionCallback);

  userScheduler.addTask(taskSendMessage);
  taskSendMessage.enable();

  Serial.println("Node 1 Sensor DHT aktif");
}

void loop() {
  mesh.update();
}
