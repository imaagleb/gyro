#pragma once

// ============================================================
// ESP32-C3 + MPU-6050 (GY-521) — Gyro Position Logger
// ============================================================
// Wiring (ESP32-C3 DevKitM-1):
//   GY-521 VCC → 3.3V
//   GY-521 GND → GND
//   GY-521 SDA → GPIO20
//   GY-521 SCL → GPIO21
//   GY-521 AD0 → GND  (I2C addr 0x68)
// ============================================================

// --- WiFi ---
// Change these to match your iPhone hotspot credentials
#define WIFI_SSID     "iPhone"
#define WIFI_PASS     ""

// Fallback AP if hotspot is not found
#define FALLBACK_AP_SSID  "GYRO_LOG"
#define FALLBACK_AP_PASS  "12345678"

// --- I2C ---
#define I2C_SDA_PIN  20
#define I2C_SCL_PIN  21
#define I2C_FREQ     400000L

// --- Sampling ---
#define SAMPLE_HZ       30
#define SAMPLE_MS       (1000 / SAMPLE_HZ)

// --- Gyro range: ±250, ±500, ±1000, ±2000 dps ---
#define GYRO_RANGE      GYRO_RANGE_500DPS
#define GYRO_SCALE      50.0f   // value * 0.02 = dps

// --- Accel range: ±2, ±4, ±8, ±16 g ---
#define ACCEL_RANGE     ACCELRANGE_4G
#define ACCEL_SCALE     1000.0f  // value * 0.001 = g

// --- Complementary filter ---
#define FILTER_ALPHA    0.98f

// --- WebSocket ---
#define WS_PORT         81

// --- Logging (LittleFS) ---
#define FLUSH_EVERY     16          // records to buffer before writing
#define MAX_FILE_BYTES  700000      // ~700 KB per log file
#define MAX_LOG_FILES   3           // keep last 3 files

// --- Status LED (WS2812 RGB on GPIO8 for DevKitM-1) ---
#define STATUS_LED_PIN  8
