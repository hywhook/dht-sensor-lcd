# dht-sensor+lcd

![Build](https://github.com/hywhook/dht-sensor-lcd/actions/workflows/build.yml/badge.svg)

Termohigrómetro standalone con **ESP32 + DHT22 + pantalla OLED SSD1306 (I2C)**.
Mide temperatura y humedad y los muestra en la pantalla, con firmware
robusto: watchdog, autodetección de la dirección I2C, anti-flap de errores
y asentamiento del sensor al arranque.

**Características**
- 🌡️ Temperatura y humedad en pantalla OLED 128×64 (I2C).
- 🛡️ Watchdog de tarea (TWDT): el ESP32 se reinicia solo si el firmware se traba.
- 🔍 Autodetección de la dirección I2C (0x3C / 0x3D).
- 🔁 Anti-flap: no muestra "SENSOR FALLA" por un fallo esporádico.
- ⚡ Asentamiento de 1.5s al arranque para evitar errores falsos.
- 📟 Salida por puerto serie a 115200 baudios.

> 📖 Documentación completa (conexiones, código y paso a paso) en **[index.html](index.html)**.
> 🌐 Podés ver la página montada en: **https://hywhook.github.io/dht-sensor-lcd/**

## Estructura

| Archivo | Qué es |
| --- | --- |
| `firmware/dht-sensor-lcd.ino` | Sketch de Arduino con todo el firmware |
| `firmware/platformio.ini` | Configuración para compilar/flashear con PlatformIO |
| `.github/workflows/build.yml` | CI que compila el firmware con PlatformIO en cada push |
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
