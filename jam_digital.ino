
// Connect: Vcc-3.3V, Gnd-Gnd, SCL-D22, SDA-D21 (OLED 0.96 inch)
// HC-SR04: Trig = D5, Echo = D18
// Code No: 03
// by sabar24
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include "time.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Ultrasonic HC-SR04 pins
#define TRIG_PIN 5
#define ECHO_PIN 18

// Eye position and size
int leftEyeX = 40;
int rightEyeX = 80;
int eyeY = 18;
int eyeWidth = 25;
int eyeHeight = 30;


// Target positions (for smooth movement)
int targetLeftEyeX = leftEyeX;
int targetRightEyeX = rightEyeX;
int moveSpeed = 2;

// Blinking and animation variables
int blinkState = 0;
int blinkDelay = 2000;
unsigned long lastBlinkTime = 0;
unsigned long moveTime = 0;
int expression = 0;

// WiFi + NTP Time Setup
const char* ssid     = "HUAWEI-2.4G-Gaz6";       // Ganti dengan WiFi kamu
const char* password = "58YnbSpT";   // Ganti dengan password kamu
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 8 * 3600;   // WITA = UTC+8
const int   daylightOffset_sec = 0;

bool timeInitialized = false;

void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.display();
  delay(1500);
  display.clearDisplay();

  // WiFi connect untuk kalibrasi waktu
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 20);
  display.print("Connecting WiFi...");
  display.display();

  unsigned long wifiStart = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - wifiStart < 10000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Connected!");
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
      timeInitialized = true;
      Serial.println("Time synchronized from NTP!");
    } else {
      Serial.println("Failed to get time from NTP");
    }
    WiFi.disconnect(true); // matikan WiFi setelah sinkronisasi
  } else {
    Serial.println("\nWiFi not connected. Time not updated.");
  }
}

// ===========================================================
// Hitung jarak HC-SR04
// ===========================================================
float getDistanceCM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH, 25000);
  float distance = duration * 0.034 / 2;
  return distance;
}

// ===========================================================
// Tampilkan jam digital
// ===========================================================
void drawClock() {
  struct tm timeinfo;
  display.clearDisplay();

  if (!getLocalTime(&timeinfo)) {
    display.setTextSize(1);
    display.setCursor(10, 25);
    display.print("No Time Data");
    
  } else {
     
    display.setTextColor(WHITE);
    display.setTextSize(2);
    display.setCursor(15, 30);
    char timeStr[9];
    strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);
    display.print(timeStr);
    
  }
  display.display();
}

// ===========================================================
// Loop utama
// ===========================================================
void loop() {
  float distance = getDistanceCM();

  if (distance > 70 || distance <= 0) {
    // ====== MODE MATA ROBOT ======
    unsigned long currentTime = millis();

    // Blinking
    if (currentTime - lastBlinkTime > blinkDelay && blinkState == 0) {
      blinkState = 1;
      lastBlinkTime = currentTime;
    } else if (currentTime - lastBlinkTime > 400 && blinkState == 1) {
      blinkState = 0;
      lastBlinkTime = currentTime;
    }

    // Random movement
    if (currentTime - moveTime > random(2000, 5000) && blinkState == 0) {
      int eyeMovement = random(0, 3);
      if (eyeMovement == 1) {
        targetLeftEyeX = 30;
        targetRightEyeX = 60;
      } else if (eyeMovement == 2) {
        targetLeftEyeX = 50;
        targetRightEyeX = 80;
      } else {
        targetLeftEyeX = 40;
        targetRightEyeX = 70;
      }
      moveTime = currentTime;
    }

    // Smooth transition
    if (leftEyeX != targetLeftEyeX)
      leftEyeX += (targetLeftEyeX - leftEyeX) / moveSpeed;
    if (rightEyeX != targetRightEyeX)
      rightEyeX += (targetRightEyeX - rightEyeX) / moveSpeed;

    display.clearDisplay();
    if (blinkState == 0)
      drawExpression(leftEyeX, eyeY, eyeWidth, eyeHeight, expression);
    else
      display.fillRect(leftEyeX, eyeY + eyeHeight / 2 - 2, eyeWidth, 4, WHITE);

    if (blinkState == 0)
      drawExpression(rightEyeX, eyeY, eyeWidth, eyeHeight, expression);
    else
      display.fillRect(rightEyeX, eyeY + eyeHeight / 2 - 2, eyeWidth, 4, WHITE);

    display.display();
    delay(50);

    // Ganti ekspresi
    if (currentTime - moveTime > random(3000, 7000)) {
      expression = random(0, 4);
      moveTime = currentTime;
    }

  } else {
    // ====== MODE JAM DIGITAL ======
    drawClock();
    delay(1000);
  }
}

// ===========================================================
// Fungsi ekspresi mata (tidak diubah)
// ===========================================================
void drawExpression(int eyeX, int eyeY, int eyeWidth, int eyeHeight, int exp) {
  display.fillRoundRect(eyeX, eyeY, eyeWidth, eyeHeight, 5, WHITE);
  switch (exp) {
    case 0:
      break;
    case 1:
      display.fillRect(eyeX + 5, eyeY + 18, eyeWidth - 10, 4, WHITE);
      break;
    case 2:
      display.fillRect(eyeX + 5, eyeY + eyeHeight - 12, eyeWidth - 10, 4, WHITE);
      break;
    case 3:
      display.fillRect(eyeX + 5, eyeY + 7, eyeWidth - 10, 4, WHITE);
      break;
  }
}
