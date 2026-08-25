# dht-sensor+lcd

Termohigrómetro standalone con **ESP32 + DHT22 + pantalla OLED SSD1306 (I2C)**.
Mide temperatura y humedad y los muestra en la pantalla, con firmware
robusto: watchdog, autodetección de la dirección I2C, anti-flap de errores
y asentamiento del sensor al arranque.

> Documentación completa (conexiones, código y paso a paso) en **[index.html](index.html)**.

## Estructura

| Archivo | Qué es |
| --- | --- |
| `firmware/dht-sensor-lcd.ino` | Sketch de Arduino con todo el firmware |
| `firmware/platformio.ini` | Configuración para compilar/flashear con PlatformIO |
| `index.html` | Página web de documentación (español) |
| `LICENSE` | Licencia MIT |

## Dependencias (librerías)

- `DHT sensor library` (Adafruit)
- `Adafruit Unified Sensor`
- `Adafruit GFX Library`
- `Adafruit SSD1306`

`esp_task_wdt.h` ya viene con el core de ESP32 para Arduino (no se instala aparte).

## Cómo usar

- **Arduino IDE:** abrí `firmware/dht-sensor-lcd.ino`, instálalas librerías de arriba,
  elegí placa "ESP32 Dev Module" y subí.
- **PlatformIO:** `pio run -t upload` (y `pio device monitor` para el puerto serie).

Ver detalles de conexiones y explicación del código en `index.html`.
