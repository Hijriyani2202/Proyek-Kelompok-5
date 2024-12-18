#include <ESP8266WiFi.h>       // Library WiFi untuk ESP8266
#include <ESP8266WebServer.h>  // Library WebServer
#include <FS.h>                // Library untuk SPIFFS

// Konfigurasi WiFi
const char* ssid = "Galaxy A23 86BA";               // Nama jaringan WiFi
const char* password = "ggbt9894";     // Password WiFi

const int relay1 = 5;       // Pin untuk mengontrol lampu
const int pirSensor = 4;    // Pin untuk sensor PIR

unsigned long lampOnTime = 0;             // Waktu saat lampu menyala
const unsigned long lampDuration = 30000; // Durasi lampu menyala (30 detik)
bool lampStatus = false;                  // Status lampu (Mati = false, Menyala = true)

// Inisialisasi server web
ESP8266WebServer server(80);

void setup() {
  // Inisialisasi Serial
  Serial.begin(115200);

  pinMode(relay1, OUTPUT);
  pinMode(pirSensor, INPUT);
  digitalWrite(relay1, LOW);  // Lampu mati secara default

  // Inisialisasi SPIFFS
  if (!SPIFFS.begin()) {
    Serial.println("Gagal memulai SPIFFS");
    return;
  }
  Serial.println("SPIFFS berhasil dimulai");

  // Koneksi ke WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Menghubungkan ke WiFi...");
  }
  Serial.println("Terhubung ke WiFi");
  Serial.println("Alamat IP: ");
  Serial.println(WiFi.localIP());

  // Menyediakan file HTML utama
  server.on("/", HTTP_GET, []() {
    File file = SPIFFS.open("/Lampupintar.html", "r");
    if (!file) {
      server.send(404, "text/plain", "File HTML tidak ditemukan!");
      return;
    }
    server.streamFile(file, "text/html");
    file.close();
  });

  // Menyediakan file CSS
  server.on("/style.css", HTTP_GET, []() {
    File file = SPIFFS.open("/style.css", "r");
    if (!file) {
      server.send(404, "text/plain", "File CSS tidak ditemukan!");
      return;
    }
    server.streamFile(file, "text/css");
    file.close();
  });

  // API untuk menyalakan/mematikan lampu secara manual
  server.on("/toggle", HTTP_GET, []() {
    lampStatus = !lampStatus;             // Ubah status lampu
    digitalWrite(relay1, lampStatus);     // Nyalakan/matikan lampu
    Serial.println(lampStatus ? "Lampu dinyalakan manual" : "Lampu dimatikan manual");
    server.send(200, "text/plain", lampStatus ? "ON" : "OFF");
  });

  // Memulai server web
  server.begin();
  Serial.println("Server web dimulai");
}

void loop() {
  // Periksa request server
  server.handleClient();

  // Deteksi gerakan PIR
  int pirStatus = digitalRead(pirSensor);
  if (pirStatus == HIGH) {
    // Jika sensor PIR mendeteksi gerakan
    if (!lampStatus) {
      lampStatus = true;
      lampOnTime = millis();            // Simpan waktu saat lampu dinyalakan
      digitalWrite(relay1, HIGH);       // Nyalakan lampu
      Serial.println("Lampu menyala karena deteksi gerakan");
    }
  }

  // Mematikan lampu jika waktu menyala sudah melebihi durasi
  if (lampStatus && (millis() - lampOnTime >= lampDuration)) {
    lampStatus = false;
    digitalWrite(relay1, LOW);          // Matikan lampu
    Serial.println("Lampu mati setelah durasi selesai");
  }
}
