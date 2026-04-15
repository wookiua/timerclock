#include <Wire.h>
#include <RTClib.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>

// --- Configuration ---
const char *ssid = "Timer_Clock_AP";
const char *password = "12345678";

RTC_DS3231 rtc;
ESP8266WebServer server(80);

const int BUTTON_PIN = D3; // Hardware reset button
uint32_t eventTimestamp;   // Unix time of the last event

// --- Helper: Padding for time strings (e.g. 5 -> 05) ---
String pad(int value) {
  if (value < 10) return "0" + String(value);
  return String(value);
}

// --- Web Interface: Main Page ---
void handleRoot() {
  DateTime now = rtc.now();
  uint32_t diff = now.unixtime() - eventTimestamp;
  
  // Basic calculation for the web display
  int days = diff / 86400;
  int hours = (diff % 86400) / 3600;
  int minutes = (diff % 3600) / 60;
  int seconds = diff % 60;
  
  String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'>";
  html += "<title>Event Counter</title></head><body>";
  html += "<h1>Time Since Last Event</h1>";
  html += "<h2>" + String(days) + "d " + String(hours) + "h " + String(minutes) + "m " + String(seconds) + "s</h2>";
  html += "<hr><p>Current RTC Time: " + pad(now.hour()) + ":" + pad(now.minute()) + "</p>";
  html += "<p><button onclick=\"location.href='/reset'\">Reset Counter Now</button></p>";
  html += "</body></html>";
  
  server.send(200, "text/html", html);
}

// --- Web Interface: Manual Reset/Set Time ---
void handleReset() {
  DateTime now = rtc.now();
  eventTimestamp = now.unixtime();
  EEPROM.put(0, eventTimestamp);
  EEPROM.commit();
  
  server.send(200, "text/plain", "Counter Reset Successfully!");
  Serial.println("Reset via Web Interface");
}

void setup() {
  Serial.begin(115200);
  EEPROM.begin(512);
  
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  // Initialize I2C and RTC
  if (!rtc.begin()) {
    Serial.println("RTC Module Not Found!");
    while (1);
  }

  // Load event time from EEPROM
  EEPROM.get(0, eventTimestamp);
  
  // Handle case if EEPROM was empty (first boot)
  if (eventTimestamp == 0xFFFFFFFF || eventTimestamp == 0) {
    eventTimestamp = rtc.now().unixtime();
    EEPROM.put(0, eventTimestamp);
    EEPROM.commit();
  }

  // Start Access Point
  WiFi.softAP(ssid, password);
  Serial.print("Access Point Started. IP: ");
  Serial.println(WiFi.softAPIP());

  // Define Server Routes
  server.on("/", handleRoot);
  server.on("/reset", handleReset);
  server.begin();
  Serial.println("HTTP Server Started");
}

void loop() {
  server.handleClient();

  // --- Physical Button Logic ---
  if (digitalRead(BUTTON_PIN) == LOW) {
    delay(50); // Debounce
    if (digitalRead(BUTTON_PIN) == LOW) {
      DateTime now = rtc.now();
      eventTimestamp = now.unixtime();
      
      EEPROM.put(0, eventTimestamp);
      EEPROM.commit();
      
      Serial.println("Hardware Reset Triggered via Button!");
      
      // Visual feedback or wait for release
      while(digitalRead(BUTTON_PIN) == LOW) { delay(10); }
    }
  }
}