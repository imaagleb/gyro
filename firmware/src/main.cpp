// GyroSynth — прошивка для ESP32-C3 Mini (PlatformIO)
// Гироскоп MPU-6050: SDA -> GPIO 8, SCL -> GPIO 9
// Отправляет углы в COM-порт: X:12.34 Y:-5.67 Z:0.00

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

// ================= НАСТРОЙКИ ПИНОВ =================
#define PIN_SDA 8
#define PIN_SCL 9

Adafruit_MPU6050 mpu;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }

  Wire.begin(PIN_SDA, PIN_SCL);

  if (!mpu.begin(0x68, &Wire)) {
    Serial.println("MPU6050 not found!");
    while (1) delay(10);
  }

  mpu.setAccelerometerRange(MPU6050_RANGE_4_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  delay(100);
}

void loop() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // Углы наклона через акселерометр
  float angleX = atan2(a.acceleration.y, a.acceleration.z) * 180.0 / PI;
  float angleY = atan2(-a.acceleration.x, sqrt(a.acceleration.y * a.acceleration.y + a.acceleration.z * a.acceleration.z)) * 180.0 / PI;
  float angleZ = 0.0; // yaw не измеряется акселерометром

  Serial.print("X:");
  Serial.print(angleX, 2);
  Serial.print(" Y:");
  Serial.print(angleY, 2);
  Serial.print(" Z:");
  Serial.println(angleZ, 2);

  delay(50); // 20 Гц
}