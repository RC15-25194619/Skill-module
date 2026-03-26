#include <SPI.h>
#include <SdFat.h>
#include <Wire.h>
#include <TinyGPS++.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_NeoPixel.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define CHIP_SELECT   4    

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
TinyGPSPlus gps;
SdFat SD;
SdFile dataFile;

#define STRIP1_PIN 6      
#define STRIP2_PIN 5      
#define TOTAL_LEDS 60    
#define ACTIVE_LEDS 50  
Adafruit_NeoPixel strip1(TOTAL_LEDS, STRIP1_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip2(TOTAL_LEDS, STRIP2_PIN, NEO_GRB + NEO_KHZ800);

const unsigned char heart_bmp[] PROGMEM = {
  0b00000000, 0b01100110, 0b11111111, 0b11111111,
  0b11111111, 0b01111110, 0b00111100, 0b00011000
};
const int gsrPin = A3, flexPin1 = A1, pulsePin = A2;
const int motorPin = 3, flexLedPin1 = 8;

float smoothF1 = 0;
float filterWeight = 0.1;
float baseline = 512;
bool inBeat = false;
unsigned long lastBeatTime = 0;
int bpm = 0;
int flexHistory[SCREEN_WIDTH];
int writePtr = 0;


  Wire.begin();
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.display();

  strip1.begin(); strip2.begin();
  strip1.setBrightness(40); strip2.setBrightness(40);
  strip1.show(); strip2.show();
  
  pinMode(motorPin, OUTPUT);
  pinMode(flexLedPin1, OUTPUT);

  smoothF1 = analogRead(flexPin1);
  for(int i=0; i<SCREEN_WIDTH; i++) flexHistory[i] = 63;
  Serial.println(F("GPS MODIFIED - AUTO-DELETE ENABLED"));
}


  // --- 1. Flex ---
  if (now - lastSensorRead >= 20) {
    lastSensorRead = now;
    int rawF1 = analogRead(flexPin1);
    smoothF1 = (smoothF1 * (1.0 - filterWeight)) + (rawF1 * filterWeight);
    int rawPulse = analogRead(pulsePin);
    baseline = (baseline * 0.985) + (rawPulse * 0.015);
    float acSignal = rawPulse - baseline;
    if (!inBeat && acSignal > 8.0 && (now - lastBeatTime > 450)) {
      inBeat = true;
      int instantBpm = 60000 / (now - lastBeatTime);
      if (instantBpm > 45 && instantBpm < 160) bpm = (bpm == 0) ? instantBpm : (bpm * 0.7 + instantBpm * 0.3);
      lastBeatTime = now;
    }
    if (inBeat && acSignal < 0) inBeat = false;
  }


  // --- 2. Vibration motor ---
  if (smoothF1 > 35) {
    if (now % 100 < 85) digitalWrite(motorPin, HIGH);
    else digitalWrite(motorPin, LOW);
    digitalWrite(flexLedPin1, HIGH);
  } else {
    digitalWrite(motorPin, LOW);
    digitalWrite(flexLedPin1, LOW);
  }
