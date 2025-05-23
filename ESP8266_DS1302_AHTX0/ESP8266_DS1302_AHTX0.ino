//RELAXVN
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "DS1302.h"
#include <Adafruit_AHTX0.h>

// WiFi Info
const char* ssid = "yourssid";
const char* password = "yourpassword";

// OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// DS1302 pins
const int RST_PIN = 13;  // D7
const int DAT_PIN = 12;  // D6
const int CLK_PIN = 14;  // D5
DS1302 rtc(RST_PIN, DAT_PIN, CLK_PIN);

// NTP
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 7 * 3600, 60000);

// AHT
Adafruit_AHTX0 aht;

void setup() {
  Serial.begin(115200);
  Wire.begin(4, 5); // SDA=D2, SCL=D1

  // OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED lỗi!");
    while(1);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Dang ket noi WiFi...");
  display.display();

  // WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("WiFi OK!");
  display.display();

  // DS1302
  rtc.halt(false);
  rtc.writeProtect(false);

  // NTP
  timeClient.begin();
  while (!timeClient.update()) {
    timeClient.forceUpdate();
  }

  // Set time from NTP to RTC
  time_t epochTime = timeClient.getEpochTime();
  struct tm *ptm = gmtime(&epochTime);
  rtc.setTime(ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
  rtc.setDate(ptm->tm_mday, ptm->tm_mon + 1, ptm->tm_year + 1900);

  // AHT30 init
  if (!aht.begin()) {
    Serial.println("Không tìm thấy AHT30!");
    display.println("ERR: AHT30!");
    display.display();
    while (1);
  }

  delay(1000);
}

void loop() {
  Time t = rtc.getTime();

  // Đọc nhiệt độ - độ ẩm
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);

  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.printf("Thoi gian:");

  display.setCursor(0, 12);
  display.setTextSize(2);
  display.printf("%02d:%02d:%02d\n", t.hour, t.min, t.sec);
  


  display.setCursor(0, 30);
  display.setTextSize(1);
  display.printf("Ngay     :%02d/%02d/%04d\n", t.date, t.mon, t.year);
  display.printf("Nhiet do : %.1f C\n", temp.temperature);
  display.printf("Do am    : %.1f %%\n", humidity.relative_humidity);
  display.display();

  delay(1000);
}
