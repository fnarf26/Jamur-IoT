#include <DHT.h>
#include <WiFi.h>
#include <HTTPClient.h>

// Definisikan pin sensor
#define DHT22_PIN_1 14
#define DHT22_PIN_2 27
#define LED_PIN 2 // LED indikator

#define DHT22_TYPE DHT22

// Buat objek untuk masing-masing sensor
DHT dht1(DHT22_PIN_1, DHT22_TYPE);
DHT dht2(DHT22_PIN_2, DHT22_TYPE);

// Ganti dengan kredensial WiFi kamu
const char* ssid = "BOLEH";
const char* password = "";

// Firebase URL
String firebaseHost = "https://jamur-iot-default-rtdb.asia-southeast1.firebasedatabase.app/";

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Menyambung ke WiFi...");
  }
  Serial.println("Terhubung ke WiFi!");

  dht1.begin();
  dht2.begin();
}

void loop() {
  delay(2000);

  float suhu1 = dht1.readTemperature();
  float kelembapan1 = dht1.readHumidity();
  float suhu2 = dht2.readTemperature();
  float kelembapan2 = dht2.readHumidity();

  bool valid1 = !(isnan(suhu1) || isnan(kelembapan1));
  bool valid2 = !(isnan(suhu2) || isnan(kelembapan2));

  float rataSuhu = 0, rataKelembapan = 0;
  int jumlahValid = 0;

  if (valid1) {
    Serial.print("DHT1 => Suhu: ");
    Serial.print(suhu1);
    Serial.print(" *C, Kelembapan: ");
    Serial.print(kelembapan1);
    Serial.println(" %");
    rataSuhu += suhu1;
    rataKelembapan += kelembapan1;
    jumlahValid++;
  } else {
    Serial.println("❌ Gagal membaca data dari DHT2");
  }

  if (valid2) {
    Serial.print("DHT2 => Suhu: ");
    Serial.print(suhu2);
    Serial.print(" *C, Kelembapan: ");
    Serial.print(kelembapan2);
    Serial.println(" %");
    rataSuhu += suhu2;
    rataKelembapan += kelembapan2;
    jumlahValid++;
  } else {
    Serial.println("❌ Gagal membaca data dari DHT2");
  }

  if (jumlahValid > 0) {
    rataSuhu /= jumlahValid;
    rataKelembapan /= jumlahValid;

    Serial.print("Rata-rata Suhu: ");
    Serial.print(rataSuhu);
    Serial.print(" *C\tRata-rata Kelembapan: ");
    Serial.print(rataKelembapan);
    Serial.println(" %");

    if (rataSuhu > 32.0) {
      digitalWrite(LED_PIN, HIGH);
      Serial.println("🔥 Suhu tinggi! LED Menyala.");
    } else {
      digitalWrite(LED_PIN, LOW);
      Serial.println("✅ Suhu normal. LED Mati.");
    }

    // Kirim data ke Firebase
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      String url = firebaseHost + "/sensorData/2508120398124.json";

      String jsonData = "{";
      if (valid1) {
        jsonData += "\"suhuDHT1\": " + String(suhu1) + 
                    ", \"kelembapanDHT1\": " + String(kelembapan1) + ", ";
      }
      if (valid2) {
        jsonData += "\"suhuDHT2\": " + String(suhu2) + 
                    ", \"kelembapanDHT2\": " + String(kelembapan2) + ", ";
      }
      jsonData += "\"rataSuhu\": " + String(rataSuhu) +
                  ", \"rataKelembapan\": " + String(rataKelembapan) + "}";

      http.begin(url);
      http.addHeader("Content-Type", "application/json");

      int httpResponseCode = http.PATCH(jsonData);

      if (httpResponseCode > 0) {
        Serial.print("✅ Data terkirim ke Firebase. Kode: ");
        Serial.println(httpResponseCode);
      } else {
        Serial.print("❌ Gagal kirim ke Firebase. Kode: ");
        Serial.println(httpResponseCode);
      }

      http.end();
    } else {
      Serial.println("⚠️ WiFi tidak terhubung saat kirim data.");
    }
  } else {
    Serial.println("❗ Tidak ada data valid dari kedua sensor.");
    digitalWrite(LED_PIN, LOW);
  }

  delay(5000);
}
