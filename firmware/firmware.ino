#include <Wire.h>
#include <RTClib.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// --- Settings ---
RTC_DS3231 rtc;
ESP8266WebServer server(80);
Adafruit_SSD1306 display(128, 64, &Wire, -1);

const int BUTTON_PIN = D3; // Button for START and RESET
uint32_t eventTimestamp;   // Storage for your event date

// Helper for 05:01:09 format
String pad(int v) { return (v < 10) ? "0" + String(v) : String(v); }

void updateScreen(int d, int h, int m, int s) {
  display.clearDisplay();
  display.setTextSize(1); // Standard simple size
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  
  display.println("EVENT TIMER");
  display.println("---------------------");
  display.print("Days: ");    display.println(d);
  display.print("Hours: ");   display.println(h);
  display.print("Minutes: "); display.println(m);
  display.print("Seconds: "); display.println(s);
  
  display.display();
}

void handleRoot() {
  DateTime now = rtc.now();
  uint32_t diff = now.unixtime() - eventTimestamp;
  
  // Simple HTML interface
  String html = "<h1>Event Tracker</h1>";
  html += "<h2>Passed: " + String(diff / 86400) + " days</h2>";
  html += "<p><a href='/reset'><button>RESTART NOW</button></a></p>";
  html += "<form action='/set'><input type='date' name='d'><input type='submit' value='SET DATE'></form>";
  
  server.send(200, "text/html", html);
}

void handleReset() {
  eventTimestamp = rtc.now().unixtime();
  EEPROM.put(0, eventTimestamp);
  EEPROM.commit();
  server.send(Header("Location", "/"), 303, ""); // Redirect back to home
}

void setup() {
  EEPROM.begin(512);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  rtc.begin();

  EEPROM.get(0, eventTimestamp);
  if (eventTimestamp == 0 || eventTimestamp == 0xFFFFFFFF) eventTimestamp = rtc.now().unixtime();

  WiFi.softAP("My_Event_Timer", "12345678");
  server.on("/", handleRoot);
  server.on("/reset", handleReset);
  server.begin();
}

void loop() {
  server.handleClient();

  DateTime now = rtc.now();
  uint32_t diff = now.unixtime() - eventTimestamp;
  
  // Update screen with single font size
  updateScreen(diff / 86400, (diff % 86400) / 3600, (diff % 3600) / 60, diff % 60);

  // Physical Button Logic (Start/Reset)
  if (digitalRead(BUTTON_PIN) == LOW) {
    delay(50);
    if (digitalRead(BUTTON_PIN) == LOW) {
      eventTimestamp = rtc.now().unixtime();
      EEPROM.put(0, eventTimestamp);
      EEPROM.commit();
      while(digitalRead(BUTTON_PIN) == LOW);
    }
  }
}