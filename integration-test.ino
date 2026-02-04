#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <SoftwareSerial.h>

Adafruit_MPU6050 mpu;

// SIM800 on D7 (RX) and D8 (TX)
SoftwareSerial sim800(7, 8);

const char phone[] = "+917720090001";  // <-- Replace with your phone number
const float ACC_THRESHOLD = 15.0;      // threshold in m/s² (adjust as needed)

void setup() {
  Serial.begin(115200);
  sim800.begin(9600);
  Wire.begin();

  Serial.println("Initializing...");

  // Initialize MPU6050
  if (!mpu.begin(0x68)) {
    if (!mpu.begin(0x69)) {
      Serial.println("MPU6050 not found!");
      while (1) delay(10);
    }
  }
  Serial.println("MPU6050 ready");

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  delay(100);

  // Initialize SIM800
  sendAT("AT", 1000);       // basic check
  sendAT("ATE0", 500);      // disable echo
  sendAT("AT+CMGF=1", 500); // SMS text mode
  Serial.println("SIM800 ready");
}

void loop() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // calculate magnitude of acceleration vector
  float acc = sqrt(a.acceleration.x*a.acceleration.x +
                   a.acceleration.y*a.acceleration.y +
                   a.acceleration.z*a.acceleration.z);

  Serial.print("Acc: "); Serial.println(acc);

  // If sudden acceleration above threshold
  if (acc > ACC_THRESHOLD) {
    Serial.println("Impact detected! Sending SMS...");
    sendSMS("Accident detected! Please check rider.");
    delay(15000); // avoid spamming SMS
  }

  delay(300);
}

void sendAT(const char *cmd, unsigned long waitMs) {
  sim800.println(cmd);
  unsigned long start = millis();
  while (millis() - start < waitMs) {
    if (sim800.available()) {
      Serial.write(sim800.read()); // print SIM800 response to Serial Monitor
    }
  }
}

void sendSMS(const char *msg) {
  sim800.println("AT+CMGF=1");
  delay(500);
  sim800.print("AT+CMGS=\"");
  sim800.print(phone);
  sim800.println("\"");
  delay(500);
  sim800.print(msg);
  delay(500);
  sim800.write(26); // Ctrl+Z ends SMS
  delay(5000);
  Serial.println("SMS sent attempt finished.");
}
