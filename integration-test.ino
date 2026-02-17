#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <SoftwareSerial.h>
#include <math.h>
#include <string.h>

Adafruit_MPU6050 mpu;
SoftwareSerial gpsSerial(4, 3);  // GPS RX, TX

#define PHONE "+917720090001"
#define ACC_THRESHOLD 11.5
#define COOLDOWN_TIME 20000

unsigned long lastAlertTime = 0;

char latitude[15]  = "0.0";
char longitude[15] = "0.0";
bool gpsFix = false;

void setup() {

  Serial.begin(9600);       // GSM
  gpsSerial.begin(9600);    // GPS
  Wire.begin();

  if (!mpu.begin()) {
    while (1);
  }

  delay(3000);

  Serial.println("AT");
  delay(1000);
  Serial.println("ATE0");
  delay(1000);
  Serial.println("AT+CMGF=1");
  delay(1000);
}

void loop() {

  updateGPS();

  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  float acc = sqrt(
    a.acceleration.x * a.acceleration.x +
    a.acceleration.y * a.acceleration.y +
    a.acceleration.z * a.acceleration.z
  );

  unsigned long now = millis();

  if (acc > ACC_THRESHOLD && (now - lastAlertTime > COOLDOWN_TIME)) {
    sendSMS();
    lastAlertTime = now;
  }

  delay(300);
}

void updateGPS() {

  static char line[120];
  static int idx = 0;

  while (gpsSerial.available()) {

    char c = gpsSerial.read();

    if (c == '\n') {
      line[idx] = '\0';
      parseGPRMC(line);
      idx = 0;
    } else if (idx < 119) {
      line[idx++] = c;
    }
  }
}

void parseGPRMC(char *nmea) {

  if (strncmp(nmea, "$GPRMC", 6) != 0) return;

  char *token = strtok(nmea, ",");
  int field = 0;

  char latRaw[15] = "";
  char lonRaw[15] = "";
  char ns = 'N';
  char ew = 'E';

  while (token) {
    field++;

    if (field == 3 && token[0] != 'A') return;
    if (field == 4) strcpy(latRaw, token);
    if (field == 5) ns = token[0];
    if (field == 6) strcpy(lonRaw, token);
    if (field == 7) ew = token[0];

    token = strtok(NULL, ",");
  }

  convertCoord(latRaw, ns, latitude);
  convertCoord(lonRaw, ew, longitude);
  gpsFix = true;
}

void convertCoord(char *raw, char dir, char *out) {

  float val = atof(raw);
  int deg = (int)(val / 100);
  float min = val - deg * 100;
  float dec = deg + min / 60.0;

  if (dir == 'S' || dir == 'W')
    dec *= -1;

  dtostrf(dec, 8, 6, out);
}

void sendSMS() {

  Serial.println("AT+CMGF=1");
  delay(1000);

  Serial.print("AT+CMGS=\"");
  Serial.print(PHONE);
  Serial.println("\"");
  delay(1500);

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
  delay(7000);
}
