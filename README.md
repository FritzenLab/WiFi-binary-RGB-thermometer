
# 🌡️ ESP32 Binary Weather Station

A dual-mode temperature monitor that fetches data from the **OpenWeatherMap API** or falls back to an **LM35 analog sensor** if the internet is down. The temperature is displayed in **6-bit binary** using a strip of WS2812B (NeoPixel) LEDs.

## 🚀 Features

* **Smart Fetching:** Updates every 15 minutes to stay within API limits.
* **Fail-Safe:** Automatically switches to an LM35 analog sensor if WiFi fails.
* **Binary Display:** Converts Celsius to 6-bit binary (supports up to 63°C).
* **Dynamic Colors:** Generates a new random RGB color for every update.
* **Unbiased Randomness:** Uses the ESP32 hardware True Random Number Generator (`esp_random`).

## 🛠️ Hardware Requirements

* **Microcontroller:** ESP32 (DevKit V1).
* **Sensor:** LM35 Linear Temperature Sensor.
* **Display:** 6x WS2812B RGB LEDs (NeoPixels).
* **Power:** 5V Power source (ensure common ground between ESP32 and LEDs).

## 📌 Pinout

| Component | ESP32 Pin | Note |
| --- | --- | --- |
| **WS2812B Data** | GPIO 18 | Avoid using RX/TX pins |
| **LM35 Out** | GPIO 34 | Analog-only pin for best stability |
| **LM35 VCC** | 3.3V |  |
| **LM35 GND** | GND |  |

## 💻 Installation

1. Clone this repository.
2. Install the following libraries in your Arduino IDE:
* `Adafruit_NeoPixel`
* `ArduinoJson`


3. Enter your WiFi credentials and OpenWeatherMap API Key in the code:
```cpp
const char* ssid     = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";
String apiKey        = "YOUR_API_KEY";

```


4. Upload to your ESP32.

## 🔢 How to Read the Binary

The 6 LEDs represent the temperature in Celsius. Read them from left to right (LED 5 to LED 0).

| LED 5 (32) | LED 4 (16) | LED 3 (8) | LED 2 (4) | LED 1 (2) | LED 0 (1) |
| --- | --- | --- | --- | --- | --- |
| 0 | 1 | 1 | 0 | 1 | 0 |

*Example above: *
