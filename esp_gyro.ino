// GyroSynth — прошивка для ESP + гироскоп (MPU-6050)
// Отправляет углы X, Y, Z в COM-порт (Serial) в формате:
// X:12.34 Y:-5.67 Z:0.00

#include <Wire.h>

// ================= НАСТРОЙКИ ПИНОВ =================
// Измени PIN_SDA и PIN_SCL под свою плату и подключение:
#if defined(ESP32)
  // ESP32-C3 Mini: SDA=8, SCL=9 (по твоему подключению)
  #define PIN_SDA 8
  #define PIN_SCL 9
#else
  // ESP8266 (NodeMCU / Wemos): SDA=D2(GPIO4), SCL=D1(GPIO5)
  #define PIN_SDA 4
  #define PIN_SCL 5
#endif

// ================= MPU6050 =================
const uint8_t MPU_ADDR = 0x68;
const uint8_t REG_PWR_MGMT_1 = 0x6B;
const uint8_t REG_ACCEL_XOUT_H = 0x3B;

int16_t acX, acY, acZ;

void setup() {
  Serial.begin(115200);
  while (!Serial) { ; } // ждём открытие порта

  Wire.begin(PIN_SDA, PIN_SCL);

  // Пробуждаем MPU6050 (выход из sleep mode)
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(REG_PWR_MGMT_1);
  Wire.write(0x00);
  Wire.endTransmission(true);

  delay(100);
}

void loop() {
  // Читаем 6 байт акселерометра (X, Y, Z)
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(REG_ACCEL_XOUT_H);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 6, true);

  if (Wire.available() >= 6) {
    acX = Wire.read() << 8 | Wire.read();
    acY = Wire.read() << 8 | Wire.read();
    acZ = Wire.read() << 8 | Wire.read();
  }

  // Углы наклона (упрощённый расчёт через акселерометр)
  float angleX = atan2((float)acY, (float)acZ) * 180.0 / PI;
  float angleY = atan2((float)-acX, sqrt((float)acY * acY + (float)acZ * acZ)) * 180.0 / PI;
  float angleZ = 0.0; // Yaw не измеряется акселерометром

  // Отправляем в COM-порт
  Serial.print("X:");
  Serial.print(angleX, 2);
  Serial.print(" Y:");
  Serial.print(angleY, 2);
  Serial.print(" Z:");
  Serial.println(angleZ, 2);

  delay(50); // 20 Гц
}
