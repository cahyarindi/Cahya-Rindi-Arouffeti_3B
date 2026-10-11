#include <painlessMesh.h>
#include <ArduinoJson.h>

#define MESH_PREFIX "LabIoTMesh"
#define MESH_PASSWORD "iotmeshpassword"
#define MESH_PORT 3333

#define LED_PIN 12  // D6 (GPIO 12)

Scheduler userScheduler;
painlessMesh mesh;

float suhuTerakhir = NAN;
int adcTerakhir = -1;

void updateLED() {
  bool suhuKritis = !isnan(suhuTerakhir)
                    && suhuTerakhir > 31.0;

  bool kondisiGelap = adcTerakhir >= 0
                      && adcTerakhir < 300;

  if (suhuKritis || kondisiGelap) {
    digitalWrite(LED_PIN, HIGH);
    Serial.println("[AKTUATOR] LED MENYALA");
  } else {
    digitalWrite(LED_PIN, LOW);
    Serial.println("[AKTUATOR] LED MATI");
  }
}

void receivedCallback(uint32_t from, String &msg) {
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, msg);

  if (error) {
    Serial.println("[ERROR] JSON tidak valid");
    return;
  }

  const char* tipe = doc["tipe"] | "";

  if (strcmp(tipe, "suhu_node") == 0) {
    if (!doc.containsKey("suhu")) {
      Serial.println("[ERROR] Data suhu tidak tersedia");
      return;
    }

    suhuTerakhir = doc["suhu"].as<float>();

    Serial.printf(
      "[TERIMA SUHU] Node %u: %.2f C\n",
      from,
      suhuTerakhir
    );
  } else if (strcmp(tipe, "cahaya_node") == 0) {
    if (!doc.containsKey("adc")) {
      Serial.println("[ERROR] Data ADC tidak tersedia");
      return;
    }

    adcTerakhir = doc["adc"].as<int>();

    Serial.printf(
      "[TERIMA CAHAYA] Node %u: ADC %d\n",
      from,
      adcTerakhir
    );
  } else {
    Serial.println("[INFO] Tipe telemetri tidak dikenal");
    return;
  }

  updateLED();
}

void newConnectionCallback(uint32_t nodeId) {
  Serial.printf(
    "--> Koneksi Baru Terdeteksi! Node ID: %u\n",
    nodeId
  );
}

void changedConnectionCallback() {
  Serial.println(
    "--> Topologi mesh berubah: koneksi node diperbarui"
  );
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

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

  // Node 3 tidak membuat atau mengaktifkan task pengiriman.
  Serial.println("Node 3 Hub Aktuator aktif");
  Serial.println("Menunggu telemetri dari Node 1 dan Node 2");
}

void loop() {
  mesh.update();
}
