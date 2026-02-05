#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <SoftwareSerial.h>
#include <math.h>

// ================== MODULES ==================
Adafruit_MPU6050 mpu;
SoftwareSerial gpsSerial(4, 3);   // GPS RX, TX
SoftwareSerial sim800(7, 8);      // GSM RX, TX

// ================== CONFIG ==================
const char PHONE[] = "+919970141444";   // NO SPACES

#define SAMPLE_INTERVAL     200      // ms
#define BASELINE_SAMPLES    20
#define DELTA_THRESHOLD     8.0      // m/s^2 sudden deviation
#define COOLDOWN_TIME       30000    // 30 sec
#define STILL_GYRO_LIMIT    0.5
#define STILL_ACC_TOL       1.5

// ================== STATE ==================
enum State { NORMAL, ALERT_SENT };
State systemState = NORMAL;

unsigned long lastSampleTime = 0;
unsigned long lastAlertTime  = 0;

// ================== SENSOR DATA ==================
float accHistory[BASELINE_SAMPLES];
int accIndex = 0;
bool baselineReady = false;

// ================== GPS DATA ==================
char latitude[15]  = "0.0";
char longitude[15] = "0.0";
bool gpsFix = false;

// ================== SETUP ==================
void setup() {
  Serial.begin(115200);
  gpsSerial.begin(9600);
  sim800.begin(9600);
  Wire.begin();

  if (!mpu.begin()) {
    Serial.println("MPU6050 not found!");
    while (1);
  }

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  sim800.println("AT");
  delay(300);
  sim800.println("ATE0");
  delay(300);
  sim800.println("AT+CMGF=1");

  Serial.println("System READY");
}

// ================== LOOP ==================
void loop() {
  unsigned long now = millis();

  // Non-blocking sample timing
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

  Serial.print("Acc: "); Serial.print(accMag, 2);
  Serial.print(" Stable: "); Serial.print(baseline, 2);
  Serial.print(" Δ: "); Serial.println(delta, 2);

  // ================== ACCIDENT DETECTION ==================
  if (baselineReady &&
      delta > DELTA_THRESHOLD &&
      gyroMag > STILL_GYRO_LIMIT &&
      (now - lastAlertTime > COOLDOWN_TIME)) {

    Serial.println("🚨 ACCIDENT DETECTED");
    sendSMS();
    lastAlertTime = now;
    systemState = ALERT_SENT;
  }
}

// ================== BASELINE ==================
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

// ================== GPS ==================
void updateGPS() {
  while (gpsSerial.available()) {
    char c = gpsSerial.read();
    static char line[120];
    static int idx = 0;

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
  char latRaw[15], lonRaw[15], ns, ew;

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
  if (dir == 'S' || dir == 'W') dec *= -1;
  dtostrf(dec, 8, 6, out);
}

// ================== SMS ==================
void sendSMS() {
  sim800.print("AT+CMGS=\"");
  sim800.print(PHONE);
  sim800.println("\"");
  delay(200);

  sim800.print("Accident detected!\nLocation:\n");
  if (gpsFix) {
    sim800.print("https://maps.google.com/?q=");
    sim800.print(latitude);
    sim800.print(",");
    sim800.print(longitude);
  } else {
    sim800.print("GPS unavailable");
  }

  sim800.write(26);
  Serial.println("SMS SENT");
}
