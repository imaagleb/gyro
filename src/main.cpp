#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <WebSocketsServer.h>
#include <time.h>
#include "config.h"
#include "index_html.h"

// ================================================================
//  ESP32-C3 + MPU-6050 (GY-521)
//  Real-time orientation via WebSocket (30 Hz) + CSV export
//  Auto-connects to configured WiFi on every boot
// ================================================================

// --------------- Logging ---------------
#define LOG_PREFIX "log_"
#define LOG_EXT    ".bin"

Adafruit_MPU6050 mpu;
WebServer server(80);
WebSocketsServer webSocket(WS_PORT);

// ---- WiFi state machine ----
enum WifiState { ST_SCAN, ST_CONNECT, ST_WAIT, ST_CONNECTED, ST_RETRY };
WifiState wifiState = ST_SCAN;
uint32_t wifiTimer = 0;
bool timeSynced = false;

// ---- Sensor & angles ----
uint32_t lastSample = 0;
uint32_t wsLastBroadcast = 0;
float roll = 0.0f, pitch = 0.0f, yaw = 0.0f;

// ---- Log record: 24 bytes packed ----
struct LogRec {
  uint32_t sec;         // epoch seconds
  uint16_t ms;          // milliseconds within the second
  int16_t ax, ay, az;   // acceleration in mg (value / SCALE = g)
  int16_t gx, gy, gz;   // gyro in 0.02 dps (value / GYRO_SCALE = dps)
  int16_t roll, pitch, yaw; // orientation in 0.1 deg
} __attribute__((packed));

static LogRec logBuf[FLUSH_EVERY];
static uint8_t logBufN = 0;
static File logFile;
static uint16_t currentFileIdx = 0;
static uint32_t totalRecords = 0;

// Forward declaration — rotateLogFile() is used by openLogFile() below
static bool rotateLogFile();

// ================================================================
//  LED status (WS2812 on GPIO8 for DevKitM-1)
// ================================================================
static void setLED(uint8_t r, uint8_t g, uint8_t b) {
  neopixelWrite(STATUS_LED_PIN, r, g, b);
}

// ================================================================
//  LittleFS log management
// ================================================================
static void logFlush() {
  if (logBufN == 0 || !logFile) return;
  logFile.write(reinterpret_cast<const uint8_t*>(logBuf), logBufN * sizeof(LogRec));
  logFile.flush();
  logBufN = 0;
}

static void openLogFile() {
  // Find highest existing log index
  currentFileIdx = 0;
  for (int i = 0; i < 500; i++) {
    char name[20];
    snprintf(name, sizeof(name), "/%s%03d%s", LOG_PREFIX, i, LOG_EXT);
    if (LittleFS.exists(name)) currentFileIdx = i + 1;
    else break;  // no gaps assumed
  }
  rotateLogFile();
}

static bool rotateLogFile() {
  logFlush();
  if (logFile) {
    totalRecords += (uint32_t)(logFile.position() / sizeof(LogRec));
    logFile.close();
  }

  // Delete oldest file when we've hit the max
  if (currentFileIdx >= MAX_LOG_FILES) {
    char oldest[20];
    snprintf(oldest, sizeof(oldest), "/%s%03d%s",
             LOG_PREFIX, currentFileIdx - MAX_LOG_FILES, LOG_EXT);
    LittleFS.remove(oldest);
  }

  char name[20];
  snprintf(name, sizeof(name), "/%s%03d%s", LOG_PREFIX, currentFileIdx, LOG_EXT);
  logFile = LittleFS.open(name, FILE_WRITE);
  currentFileIdx++;
  return logFile == true;
}

static void appendLog(const LogRec* rec) {
  logBuf[logBufN++] = *rec;
  if (logBufN >= FLUSH_EVERY) logFlush();
}

// ================================================================
//  Time helpers
// ================================================================
static void epochToISO(uint32_t sec, uint16_t ms, char* out, size_t outLen) {
  time_t t = static_cast<time_t>(sec);
  struct tm* tm = gmtime(&t);
  if (!tm) {
    snprintf(out, outLen, "1970-01-01T00:00:00.%03d", ms);
    return;
  }
  snprintf(out, outLen, "%04d-%02d-%02dT%02d:%02d:%02d.%03d",
           tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
           tm->tm_hour, tm->tm_min, tm->tm_sec, ms);
}

// ================================================================
//  Complementary filter (roll/pitch from accel+gyro, yaw from gyro)
// ================================================================
static void updateAngles(float gx, float gy, float gz,
                         float ax, float ay, float az, float dt) {
  float rollAcc  = atan2(ay, az) * 57.2957795f;
  float pitchAcc = atan2(-ax, sqrtf(ay * ay + az * az)) * 57.2957795f;

  roll  = FILTER_ALPHA * (roll  + gx * dt) + (1.0f - FILTER_ALPHA) * rollAcc;
  pitch = FILTER_ALPHA * (pitch + gy * dt) + (1.0f - FILTER_ALPHA) * pitchAcc;
  yaw   += gz * dt;

  // Wrap yaw to [-180, 180)
  yaw = fmodf(yaw + 180.0f, 360.0f);
  if (yaw < 0.0f) yaw += 360.0f;
  yaw -= 180.0f;
}

// ================================================================
//  WiFi state machine (non-blocking)
// ================================================================
static void wifiConnectStep() {
  switch (wifiState) {

    case ST_SCAN:
      setLED(255, 255, 0);  // yellow — scanning
      WiFi.scanNetworks(true);  // async scan
      wifiState = ST_CONNECT;
      wifiTimer = millis();
      break;

    case ST_CONNECT:
      // Wait for scan to finish (up to 3 s)
      if (millis() - wifiTimer > 3000) {
        int n = WiFi.scanComplete();
        if (n >= 0) {
          bool found = false;
          for (int i = 0; i < n; i++) {
            if (WiFi.SSID(i) == WIFI_SSID) {
              found = true;
              Serial.printf("  Found AP: \"%s\" RSSI=%d\n",
                            WIFI_SSID, WiFi.RSSI(i));
              WiFi.begin(WIFI_SSID, WIFI_PASS);
              wifiState = ST_WAIT;
              wifiTimer = millis();
              break;
            }
          }
          WiFi.scanDelete();
          if (!found) {
            Serial.println("  AP not found — retrying...");
            wifiState = ST_SCAN;
            wifiTimer = millis();
          }
        }
      }
      break;

    case ST_WAIT:
      setLED(255, 128, 0);  // orange — connecting
      if (WiFi.status() == WL_CONNECTED) {
        wifiState = ST_CONNECTED;
        Serial.printf("  Connected! IP: %s\n", WiFi.localIP().toString().c_str());
        setLED(0, 255, 0);  // green

        // SNTP time sync
        configTime(0, 0, "pool.ntp.org", "time.google.com");
        if (time(nullptr) > 1700000000) {
          timeSynced = true;
          Serial.println("  Time synced (SNTP)");
        }
      } else if (millis() - wifiTimer > 12000) {
        Serial.println("  Connect timeout — retrying...");
        WiFi.disconnect(true);
        wifiState = ST_SCAN;
        wifiTimer = millis();
      }
      break;

    case ST_CONNECTED:
      if (WiFi.status() != WL_CONNECTED) {
        Serial.println("  WiFi lost — rescan");
        wifiState = ST_SCAN;
        wifiTimer = millis();
      } else if (!timeSynced && time(nullptr) > 1700000000) {
        timeSynced = true;
        Serial.println("  Time synced (SNTP, delayed)");
      }
      break;

    default:
      break;
  }
}

// ================================================================
//  Sensor sampling + WS broadcast
// ================================================================
static void sampleAndBroadcast() {
  if (millis() - lastSample < SAMPLE_MS) return;
  lastSample = millis();

  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  float ax = a.acceleration.x / 9.80665f;
  float ay = a.acceleration.y / 9.80665f;
  float az = a.acceleration.z / 9.80665f;
  float gx = g.gyro.x;
  float gy = g.gyro.y;
  float gz = g.gyro.z;

  updateAngles(gx, gy, gz, ax, ay, az, 1.0f / static_cast<float>(SAMPLE_HZ));

  // Broadcast to WebSocket clients (~30 Hz)
  if (millis() - wsLastBroadcast >= 30) {
    wsLastBroadcast = millis();
    char json[180];
    snprintf(json, sizeof(json),
             "{\"ax\":%.3f,\"ay\":%.3f,\"az\":%.3f,"
             "\"gx\":%.2f,\"gy\":%.2f,\"gz\":%.2f,"
             "\"roll\":%.1f,\"pitch\":%.1f,\"yaw\":%.1f,"
             "\"temp\":%.1f}",
             ax, ay, az, gx, gy, gz, roll, pitch, yaw, temp.temperature);
    webSocket.broadcastTXT(json);
  }

  // Store in log buffer
  LogRec rec;
  rec.ax = static_cast<int16_t>(ax * ACCEL_SCALE);
  rec.ay = static_cast<int16_t>(ay * ACCEL_SCALE);
  rec.az = static_cast<int16_t>(az * ACCEL_SCALE);
  rec.gx = static_cast<int16_t>(gx * GYRO_SCALE);
  rec.gy = static_cast<int16_t>(gy * GYRO_SCALE);
  rec.gz = static_cast<int16_t>(gz * GYRO_SCALE);
  rec.roll  = static_cast<int16_t>(roll  * 10.0f);
  rec.pitch = static_cast<int16_t>(pitch * 10.0f);
  rec.yaw   = static_cast<int16_t>(yaw   * 10.0f);

  if (timeSynced) {
    rec.sec  = static_cast<uint32_t>(time(nullptr));
    rec.ms   = static_cast<uint16_t>(millis() % 1000);
  } else {
    rec.sec  = static_cast<uint32_t>(millis() / 1000);
    rec.ms   = static_cast<uint16_t>(millis() % 1000);
  }

  appendLog(&rec);

  // Rotate log file if it's too large
  if (logFile && logFile.position() >= MAX_FILE_BYTES) {
    rotateLogFile();
  }
}

// ================================================================
//  WebSocket event handler
// ================================================================
static void wsEvent(uint8_t num, WStype_t type,
                    uint8_t* payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("[WS %u] disconnected\n", num);
      break;
    case WStype_CONNECTED:
      Serial.printf("[WS %u] connected (IP: %s)\n",
                    num, webSocket.remoteIP(num).toString().c_str());
      break;
    case WStype_TEXT:
      // "zero" command → reset yaw
      if (length > 0 && payload[0] == 'z') {
        yaw = 0.0f;
        Serial.println("  Yaw zeroed from client");
      }
      break;
    default:
      break;
  }
}

// ================================================================
//  WebServer handlers
// ================================================================
static void handleRoot() {
  server.send_P(200, "text/html", INDEX_HTML);
}

static void handleStatus() {
  char buf[220];
  uint32_t serverTime = timeSynced
                            ? static_cast<uint32_t>(time(nullptr))
                            : static_cast<uint32_t>(millis() / 1000);
  snprintf(buf, sizeof(buf),
           "{\"ip\":\"%s\","
           "\"server_time\":%u,"
           "\"log_bytes\":%lu,"
           "\"samples\":%lu,"
           "\"time_synced\":%s,"
           "\"rssi\":%d}",
           WiFi.localIP().toString().c_str(),
           serverTime,
           static_cast<unsigned long>(totalRecords * sizeof(LogRec)),
           totalRecords,
           timeSynced ? "true" : "false",
           WiFi.RSSI());
  server.send(200, "application/json", buf);
}

static void handleDownload() {
  String fromStr = server.hasArg("from") ? server.arg("from") : "0";
  String toStr   = server.hasArg("to")   ? server.arg("to")   : "999999999999";
  String tzStr   = server.hasArg("tz")   ? server.arg("tz")   : "0";

  uint64_t fromMs = strtoull(fromStr.c_str(), nullptr, 10);
  uint64_t toMs   = strtoull(toStr.c_str(), nullptr, 10);
  int32_t  tzSec  = static_cast<int32_t>(tzStr.toInt());

  // Build filename from 'from' timestamp
  time_t t = static_cast<time_t>(fromMs / 1000);
  struct tm* tm = localtime(&t);
  char fname[32];
  snprintf(fname, sizeof(fname), "gyro_%04d%02d%02d_%02d%02d.csv",
           tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
           tm->tm_hour, tm->tm_min);

  server.sendHeader("Content-Disposition",
                    String("attachment; filename=\"") + fname + "\"");
  server.sendHeader("Cache-Control", "no-cache");

  // Enable chunked transfer
  server.send(200, "text/csv", "");  // empty content → chunked mode

  const char csvHeader[] =
      "timestamp;ax_g;ay_g;az_g;gx_dps;gy_dps;gz_dps;"
      "roll_deg;pitch_deg;yaw_deg\n";
  server.sendContent(String(csvHeader));

  // Discover existing log files
  char fileNames[MAX_LOG_FILES][20];
  int nFiles = 0;
  for (int i = 0; i < currentFileIdx && nFiles < MAX_LOG_FILES; i++) {
    char n[20];
    snprintf(n, sizeof(n), "/%s%03d%s", LOG_PREFIX, i, LOG_EXT);
    if (LittleFS.exists(n)) strncpy(fileNames[nFiles++], n, 19);
  }

  // Sort file names (bubble sort, tiny array)
  for (int i = 0; i < nFiles - 1; i++)
    for (int j = 0; j < nFiles - 1 - i; j++)
      if (strcmp(fileNames[j], fileNames[j + 1]) > 0) {
        char tmp[20];
        strcpy(tmp, fileNames[j]);
        strcpy(fileNames[j], fileNames[j + 1]);
        strcpy(fileNames[j + 1], tmp);
      }

  // Stream filtered records
  char csvLine[120];
  char iso[24];
  int batch = 0;

  for (int fi = 0; fi < nFiles; fi++) {
    File f = LittleFS.open(fileNames[fi], FILE_READ);
    if (!f) continue;

    LogRec rec;
    while (f.available() &&
           f.read(reinterpret_cast<uint8_t*>(&rec), sizeof(LogRec))
           == sizeof(LogRec)) {
      uint64_t recMs = static_cast<uint64_t>(rec.sec) * 1000ULL + rec.ms;
      if (recMs < fromMs || recMs > toMs) continue;

       epochToISO(rec.sec + tzSec, rec.ms, iso, sizeof(iso));

      snprintf(csvLine, sizeof(csvLine),
               "%s;%.3f;%.3f;%.3f;"
               "%.2f;%.2f;%.2f;"
               "%.1f;%.1f;%.1f\n",
               iso,
               static_cast<double>(rec.ax) / ACCEL_SCALE,
               static_cast<double>(rec.ay) / ACCEL_SCALE,
               static_cast<double>(rec.az) / ACCEL_SCALE,
               static_cast<double>(rec.gx) / GYRO_SCALE,
               static_cast<double>(rec.gy) / GYRO_SCALE,
               static_cast<double>(rec.gz) / GYRO_SCALE,
               static_cast<double>(rec.roll) / 10.0,
               static_cast<double>(rec.pitch) / 10.0,
               static_cast<double>(rec.yaw) / 10.0);

      server.sendContent(String(csvLine));
      batch++;

      // Yield periodically to keep WiFi/WDT happy
      if (batch >= 30) {
        yield();
        batch = 0;
      }
    }
    f.close();
  }

  // Terminate chunked transfer
  server.sendContent(String());
}

// ================================================================
//  Setup
// ================================================================
void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println();
  Serial.println("========================================");
  Serial.println("  MPU-6050 Gyro Position Logger");
  Serial.println("========================================");

  // LED: blue on boot
  setLED(0, 0, 255);

  // LittleFS
  if (!LittleFS.begin(true)) {
    Serial.println("ERROR: LittleFS mount failed!");
    setLED(255, 0, 0);
    while (1) delay(100);
  }
  Serial.println("  LittleFS mounted OK");

  // I2C + MPU-6050
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN, I2C_FREQ);
  if (!mpu.begin(0x68)) {
    Serial.println("ERROR: MPU-6050 not found!");
    Serial.println("  Check wiring: SDA->20, SCL->21");
    setLED(255, 0, 0);
    while (1) delay(100);
  }
  mpu.setAccelerometerRange(ACCEL_RANGE);
  mpu.setGyroRange(GYRO_RANGE);
  Serial.printf("  MPU-6050 OK  Accel: +/-%dg  Gyro: +/-%d dps\n",
                mpu.getAccelerometerRange() == 2 ? 2
                : mpu.getAccelerometerRange() == 4 ? 4
                : mpu.getAccelerometerRange() == 8 ? 8 : 16,
                mpu.getGyroRange() == 250 ? 250
                : mpu.getGyroRange() == 500 ? 500
                : mpu.getGyroRange() == 1000 ? 1000 : 2000);

  // WebSocket
  webSocket.begin();
  webSocket.onEvent(wsEvent);
  Serial.printf("  WebSocket server on port %d\n", WS_PORT);

  // Open log file
  openLogFile();
  Serial.printf("  Logging: %s%03d%s  (records so far: %lu)\n",
                LOG_PREFIX, currentFileIdx - 1, LOG_EXT, totalRecords);

  // HTTP server
  server.on("/", handleRoot);
  server.on("/status", handleStatus);
  server.on("/download", handleDownload);
  server.begin();

  Serial.println("  HTTP server ready on port 80");
  Serial.printf("  Connect your browser to http://%s\n",
                WiFi.status() == WL_CONNECTED
                    ? WiFi.localIP().toString().c_str()
                    : "<waiting-for-WiFi>");

  // Start WiFi scan
  wifiState = ST_SCAN;
  wifiTimer = millis();

  Serial.println();
  Serial.printf("  Watch for AP: \"%s\"  (password: \"%s\")\n", WIFI_SSID, WIFI_PASS);
  Serial.println("  ========================================");
}

// ================================================================
//  Main loop
// ================================================================
void loop() {
  // WiFi connect state machine
  wifiConnectStep();

  // Sensor sampling at 30 Hz
  sampleAndBroadcast();

  // Web server & WebSocket
  server.handleClient();
  webSocket.loop();
}
