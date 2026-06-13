/*
====================================================
ESP32 ACCESS POINT + WEBSERVER
MPU6050 + BMP180
====================================================

ESP32 creates WiFi hotspot:
SSID = ESP32_Project
PASS = 12345678

ESP32 IP:
192.168.4.1

Provides a simple JSON API at:
http://192.168.4.1/data
====================================================
*/

// ---------------- LIBRARIES ----------------
#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>
#include <MPU6050_tockn.h>

// ---------------- ACCESS POINT ----------------
const char* ap_ssid = "ESP32_Project";
const char* ap_pass = "12345678";

// ---------------- WEBSERVER ----------------
WebServer server(80);

// Two I2C buses
TwoWire I2CBMP = TwoWire(0);
TwoWire I2CMPU = TwoWire(1);

// Sensors
Adafruit_BMP085 bmp;
MPU6050 mpu(I2CMPU);

// ---------------- VERSION ----------------
String version = "3.0.0";

// =================================================
// START ACCESS POINT
// =================================================
void startAP()
{
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ap_ssid, ap_pass);

  IPAddress IP = WiFi.softAPIP();

  Serial.println("AP STARTED");
  Serial.print("ESP32 AP IP: ");
  Serial.println(IP);
}

// =================================================
// WEBSERVER ENDPOINTS
// =================================================
void handleRoot() {
  server.send(200, "text/plain", "ESP32 Sensor Server is Running. Go to /data for JSON telemetry.");
}

void handleData() {
  // Read latest sensor data
  mpu.update();

  float ax = mpu.getAccX();
  float ay = mpu.getAccY();
  float az = mpu.getAccZ();

  float gx = mpu.getGyroX();
  float gy = mpu.getGyroY();
  float gz = mpu.getGyroZ();

  float temp = bmp.readTemperature();
  float pressure = bmp.readPressure() / 100.0;
  float altitude = bmp.readAltitude();

  // Create JSON string
  String json = "{";
  json += "\"status\":\"RUNNING\",";
  json += "\"version\":\"" + version + "\",";
  
  // MPU6050
  json += "\"mpu6050\":{";
  json += "\"accel\":{\"x\":" + String(ax, 2) + ",\"y\":" + String(ay, 2) + ",\"z\":" + String(az, 2) + "},";
  json += "\"gyro\":{\"x\":" + String(gx, 2) + ",\"y\":" + String(gy, 2) + ",\"z\":" + String(gz, 2) + "}";
  json += "},";

  // BMP180
  json += "\"bmp180\":{";
  json += "\"temp\":" + String(temp, 2) + ",";
  json += "\"pressure\":" + String(pressure, 2) + ",";
  json += "\"altitude\":" + String(altitude, 2);
  json += "}";
  json += "}";

  // Send response
  server.send(200, "application/json", json);
}

// =================================================
// SETUP
// =================================================
void setup()
{
  Serial.begin(115200);

  // -------- I2C --------
  I2CBMP.begin(18, 19);   // BMP180
  I2CMPU.begin(21, 22);   // MPU6050

  // -------- BMP180 --------
  Serial.println("Initializing BMP180...");
  if (!bmp.begin(0x77, &I2CBMP)) {
    Serial.println("Could not find a valid BMP180 sensor, check wiring!");
    // If it fails, we won't block forever, so the AP still starts
  }

  // -------- MPU6050 --------
  Serial.println("Initializing MPU6050...");
  mpu.begin();
  mpu.calcGyroOffsets(true);

  // -------- START AP --------
  startAP();

  // -------- START SERVER --------
  server.on("/", handleRoot);
  server.on("/data", HTTP_GET, handleData);
  server.begin();
  Serial.println("HTTP Server Started");
}

// =================================================
// LOOP
// =================================================
void loop()
{
  server.handleClient();
  delay(10); // Small delay to prevent watchdog issues
}
