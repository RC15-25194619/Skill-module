#include <SPI.h>
#include <SdFat.h>
#include <Wire.h>
#include <TinyGPS++.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_NeoPixel.h>

const int gsrPin = A3, flexPin1 = A1, pulsePin = A2;
const int motorPin = 3, flexLedPin1 = 8;

// --- sensor core variables ---
float smoothF1 = 0;
float filterWeight = 0.1;
float baseline = 512;
bool inBeat = false;
unsigned long lastBeatTime = 0;
int bpm = 0;

// --- timing ---
unsigned long lastSensorRead = 0;

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);

  pinMode(motorPin, OUTPUT);
  pinMode(flexLedPin1, OUTPUT);

  smoothF1 = analogRead(flexPin1);
}

void loop() {
  unsigned long now = millis();

  // --- sensor reading ---
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
      if (instantBpm > 45 && instantBpm < 160) {
        bpm = (bpm == 0) ? instantBpm : (bpm * 0.7 + instantBpm * 0.3);
      }
      lastBeatTime = now;
    }

    if (inBeat && acSignal < 0) inBeat = false;
  }

  // --- motor + small LED on pin 8 ---
  if (smoothF1 > 35) {
    if (now % 100 < 85) digitalWrite(motorPin, HIGH);
    else digitalWrite(motorPin, LOW);

    digitalWrite(flexLedPin1, HIGH);
  } else {
    digitalWrite(motorPin, LOW);
    digitalWrite(flexLedPin1, LOW);
  }
}