#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <SoftwareSerial.h>
#include <math.h>

// MODULES
Adafruit_MPU6050 mpu;
SoftwareSerial gpsSerial(4, 3);   // GPS RX, TX

#define PHONE "+917720090001"

// CONFIG
#define SAMPLE_INTERVAL 200
#define BASELINE_SAMPLES 20
#define DELTA_THRESHOLD 8.0
#define COOLDOWN_TIME 30000
#define STILL_GYRO_LIMIT 0.5

unsigned long lastSampleTime = 0;
unsigned long lastAlertTime = 0;

float accHistory[BASELINE_SAMPLES];
int accIndex = 0;
bool baselineReady = false;

char latitude[15] = "0.0";
char longitude[15] = "0.0";
bool gpsFix = false;

void setup() {
  Serial.begin(9600);      // GSM now uses hardware Serial
  gpsSerial.begin(9600);
  Wire.begin();

  mpu.begin();

  delay(2000);

  Serial.println("AT");
  delay(500);
  Serial.println("ATE0");
  delay(500);
  Serial.println("AT+CMGF=1");

  Serial.println("System READY");
}

void loop() {

  unsigned long now = millis();
  if (now - lastSampleTime < SAMPLE_INTERVAL) return;
  lastSampleTime = now;

  updateGPS();

  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  float accMag = sqrt(
    a.acceleration.x * a.acceleration.x +
    a.acceleration.y * a.acceleration.y +
    a.acceleration.z * a.acceleration.z
  );

  float gyroMag = sqrt(
    g.gyro.x * g.gyro.x +
    g.gyro.y * g.gyro.y +
    g.gyro.z * g.gyro.z
  );

  float baseline = updateBaseline(accMag);
  float delta = fabs(accMag - baseline);

  if (baselineReady &&
      delta > DELTA_THRESHOLD &&
      gyroMag > STILL_GYRO_LIMIT &&
      (now - lastAlertTime > COOLDOWN_TIME)) {

    sendSMS();
    lastAlertTime = now;
  }
}

float updateBaseline(float val) {
  accHistory[accIndex++] = val;

  if (accIndex >= BASELINE_SAMPLES) {
    accIndex = 0;
    baselineReady = true;
  }

  float sum = 0;
  int count = baselineReady ? BASELINE_SAMPLES : accIndex;

  for (int i = 0; i < count; i++) sum += accHistory[i];
  return sum / count;
}

void updateGPS() {
  while (gpsSerial.available()) {
    char c = gpsSerial.read();
    // (Keep your existing parser here)
  }
}

void sendSMS() {

  Serial.print("AT+CMGS=\"");
  Serial.print(PHONE);
  Serial.println("\"");
  delay(300);

  Serial.print("Accident detected!\nLocation:\n");

  if (gpsFix) {
    Serial.print("https://maps.google.com/?q=");
    Serial.print(latitude);
    Serial.print(",");
    Serial.print(longitude);
  } else {
    Serial.print("GPS unavailable");
  }

  Serial.write(26);
}
