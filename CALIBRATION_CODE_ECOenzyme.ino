#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPL6aZ9xTv_9"
#define BLYNK_TEMPLATE_NAME "EcoEnzyme"
#define BLYNK_AUTH_TOKEN "F1mV-3l7jU5s9WEVOjI00Xhursh5FrRs"


#include <SPI.h>
#include <WiFi.h>
#include <BlynkSimpleWifi.h>


#include <DHT.h>
#include "HX711.h"

// WiFi & Blynk
char ssid[] = "POCO X7";             // Ganti dengan SSID WiFi kamu
char pass[] = "babi12345678";         // Ganti dengan password WiFi kamu

// DHT
#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// MQ-135 dan pH Sensor
#define MQ135_PIN A0
#define PH_PIN A0 // Note: hanya satu pin analog di ESP8266, gunakan MUX jika pakai dua sensor analog

// HX710B
#define HX_DT 4
#define HX_SCK 5
HX711 scale;

// Kalibrasi
float pH4Voltage = 3.1;
float pH7Voltage = 2.5;
float pHStep = (pH4Voltage - pH7Voltage) / 2;
float R0 = 76.63;
#define LOADCELL_TO_GRAMS 2280.0
#define PRESSURE_AREA_IN2 1.0

void setup() {
  Serial.begin(115200);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  dht.begin();

  scale.begin(HX_DT, HX_SCK);
  if (scale.is_ready()) {
    Serial.println("HX710B Tersambung.");
    scale.set_scale(LOADCELL_TO_GRAMS);
    scale.tare();
  } else {
    Serial.println("HX710B tidak terbaca.");
  }
}

float calculatePH(int adcValue) {
  float voltage = adcValue * (5.0 / 1023.0);
  return 7.0 - ((voltage - pH7Voltage) / pHStep);
}

float calculatePPM(int adcValue) {
  float voltage = adcValue * (5.0 / 1023.0);
  float resistance = ((5.0 * 10.0) / voltage) - 10.0;
  float ratio = resistance / R0;
  return 116.6020682 * pow(ratio, -2.769034857);
}

void loop() {
  Blynk.run();

  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  int rawAnalog = analogRead(A0);
  
  float co2PPM = calculatePPM(rawAnalog) / 100;
  float pHValue = calculatePH(rawAnalog);

  float weightInGrams = scale.get_units(10);
  if (weightInGrams < 0) weightInGrams = 0;  // Hindari nilai negatif

float weightInLbs = (weightInGrams * 0.00220462);
float pressurePSI = (weightInLbs / PRESSURE_AREA_IN2) / 1000;


  if (!isnan(humidity) && !isnan(temperature)) {
    float correctedTemp = temperature*10;

    Blynk.virtualWrite(V0, correctedTemp);  // Suhu
    Blynk.virtualWrite(V1, humidity);       // Kelembaban
    Blynk.virtualWrite(V2, co2PPM);         // CO2
    Blynk.virtualWrite(V3, pHValue);        // pH
    Blynk.virtualWrite(V4, pressurePSI);    // Tekanan

    Serial.println("Data terkirim ke Blynk:");
    Serial.print("Temp: "); Serial.println(correctedTemp);
    Serial.print("Hum : "); Serial.println(humidity);
    Serial.print("CO2 : "); Serial.println(co2PPM);
    Serial.print("pH  : "); Serial.println(pHValue);
    Serial.print("PSI : "); Serial.println(pressurePSI);
  } else {
    Serial.println("Gagal membaca DHT");
  }

  delay(2000);
}
