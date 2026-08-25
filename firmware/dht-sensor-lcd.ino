// ──────────────────────────────────────────────
//  ESP32 + DHT22 + SSD1306 I2C
//  Termohigrómetro standalone
// ──────────────────────────────────────────────

// ─── CONFIG ───────────────────────────────────
#define DHT_PIN              14
#define DHT_TYPE             DHT22       // DHT11 | DHT22 | DHT21
#define SCREEN_I2C_ADDR      0x3C        // fallback; se autodetecta 0x3C/0x3D
#define UPDATE_INTERVAL      5000UL      // ms entre lecturas
#define SENSOR_RETRIES       3           // intentos por ciclo antes de marcar error
#define SENSOR_RETRY_DELAY   2000UL      // DHT22 pide >=2000ms entre transacciones
#define SENSOR_FAIL_THRESHOLD 3          // fallos consecutivos para mostrar "SENSOR FALLA"
#define TEMP_DECIMALS        1
#define HUM_DECIMALS         1
#define LED_PIN              2           // LED onboard (ESP32 devkit). -1 si no tenes.
#define WDT_TIMEOUT_MS       8000        // > peor caso de readSensor (~4s)

// ─── LIBS ─────────────────────────────────────
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
#include "esp_task_wdt.h"

// ─── INSTANCIAS ───────────────────────────────
Adafruit_SSD1306 display(128, 64, &Wire, -1);
DHT dht(DHT_PIN, DHT_TYPE);

// ─── ESTADO ───────────────────────────────────
struct Reading {
  float temp;
  float hum;
  bool  valid;
};
Reading last = {0, 0, false};
unsigned long lastRead = 0;
uint8_t failStreak = 0;

// ─── AYUDANTES ────────────────────────────────
// Busca el SSD1306 en las direcciones I2C tipicas. Devuelve 0 si no hay nadie.
uint8_t scanDisplayAddress() {
  const uint8_t candidates[] = {0x3C, 0x3D};
  for (uint8_t a : candidates) {
    Wire.beginTransmission(a);
    if (Wire.endTransmission() == 0) return a;
  }
  return 0;
}

// Falla fatal: parpadea el LED y reinicia (nunca cuelga mudo).
void blinkFatal() {
  for (uint8_t i = 0; i < 20; i++) {
    if (LED_PIN >= 0) digitalWrite(LED_PIN, HIGH);
    delay(120);
    if (LED_PIN >= 0) digitalWrite(LED_PIN, LOW);
    delay(120);
  }
  ESP.restart();
}

// ─── SETUP ────────────────────────────────────
void setup() {
  Serial.begin(115200);
  delay(500);

  if (LED_PIN >= 0) {
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
  }

  // Watchdog de tarea: el loop debe "alimentarlo" o el chip reinicia.
  esp_task_wdt_config_t wdt = {
    .timeout_ms = WDT_TIMEOUT_MS,
    .idle_core_mask = (1 << 0),
    .trigger_panic = true,
  };
  esp_task_wdt_init(&wdt);
  esp_task_wdt_add(NULL);

  // Pull-up del bus de datos. La lib lo habilita, pero lo dejamos explicito.
  // DHT22 desnudo NECESITA 4.7k-10k externo entre DATA y VCC; los modulos ya lo traen.
  pinMode(DHT_PIN, INPUT_PULLUP);
  dht.begin();

  // I2C: arrancamos el bus y autodetectamos la direccion del display.
  Wire.begin();
  uint8_t addr = scanDisplayAddress();
  if (addr == 0 || !display.begin(SSD1306_SWITCHCAPVCC, addr)) {
    Serial.printf("[FATAL] SSD1306 no detectado en I2C (0x3C/0x3D).\n");
    blinkFatal();
  }
  Serial.printf("[OK] SSD1306 en 0x%02X\n", addr);

  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(24, 24);
  display.print(F("Iniciando..."));
  display.display();

  // El DHT22 tarda en asentarse tras el power-on: dejamos que estabilice
  // para no "parpadear" un error falso en la primer lectura.
  delay(1500);

  // Forzar primera lectura inmediata en el loop.
  lastRead = millis() - UPDATE_INTERVAL;
}

// ─── LECTURA SENSOR ───────────────────────────
// Ambas llamadas parsean la misma trama de 40 bits del DHT22. El time-guard
// interno (DHTLIB_DELAY = 2000ms) evita re-disparar el bus dentro de los 2s,
// asi la segunda lectura devuelve los valores cacheados. Una transaccion,
// dos valores.
Reading readSensor() {
  for (uint8_t i = 0; i < SENSOR_RETRIES; i++) {
    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if (!isnan(t) && !isnan(h)) {
      return {t, h, true};
    }
    if (i < SENSOR_RETRIES - 1) {
      delay(SENSOR_RETRY_DELAY);
    }
  }
  return {0, 0, false};
}

// ─── RENDER ───────────────────────────────────
void renderDisplay() {
  display.clearDisplay();

  display.setTextSize(1);
  display.setCursor(18, 0);
  display.print(F("ESP32 + DHT22"));

  display.drawLine(0, 10, 128, 10, WHITE);

  if (last.valid) {
    display.setTextSize(1);
    display.setCursor(0, 18);
    display.print(F("TEMP:"));

    display.setTextSize(2);
    display.setCursor(42, 14);
    display.print(last.temp, TEMP_DECIMALS);
    display.drawCircle(96, 20, 2, WHITE);
    display.setTextSize(1);
    display.setCursor(102, 14);
    display.print(F("C"));

    display.setTextSize(1);
    display.setCursor(0, 42);
    display.print(F("HUM:"));

    display.setTextSize(2);
    display.setCursor(42, 38);
    display.print(last.hum, HUM_DECIMALS);
    display.print(F("%"));
  } else {
    display.setTextSize(1);
    display.setCursor(22, 22);
    display.print(F("SENSOR FALLA"));
    display.setCursor(10, 40);
    display.print(F("Verificar cableado"));
  }

  display.display();
}

// ─── LOOP ─────────────────────────────────────
void loop() {
  esp_task_wdt_reset();   // mantenemos vivo al watchdog

  if (millis() - lastRead >= UPDATE_INTERVAL) {
    lastRead = millis();
    Reading r = readSensor();

    if (r.valid) {
      bool recovered = !last.valid;
      failStreak = 0;
      last = r;
      Serial.printf("T: %.1f C | H: %.1f%%\n", last.temp, last.hum);
      if (recovered) Serial.printf("[OK] sensor recuperado\n");
    } else {
      failStreak++;
      // Anti-flap: mientras el fallo sea esporadico, seguimos mostrando
      // el ultimo valor bueno. Solo marcamos error tras N fallos seguidos.
      if (failStreak >= SENSOR_FAIL_THRESHOLD) {
        last.valid = false;
      }
      Serial.printf("DHT: fallo de lectura (racha %u/%u)\n",
                    failStreak, SENSOR_FAIL_THRESHOLD);
    }

    renderDisplay();
  }
}
