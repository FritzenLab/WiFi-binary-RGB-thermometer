 // https://openweathermap.org/ to get your API KEY
 // WS2812b neopixels: https://fritzenlab.net/2024/07/14/ws2812b-addressable-rgb-led-with-esp32-c6/
 // Generate true random numbers with ESP32: https://fritzenlab.net/2025/12/06/generate-random-numbers-with-esp32/
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h> // Parsing JSON from OpenWeatherMap

#define lm35_PIN 4 // Use GPIO 4 (A4)

#include <Adafruit_NeoPixel.h>      
#define D_in 3  
#define qtdeLeds 6
Adafruit_NeoPixel pixels(qtdeLeds, D_in);

#include <esp_system.h> // Required for esp_random()

// 20 predefined RGB colors
const uint8_t colorPalette[20][3] = {
  {255,   0,   0},   // Red
  {0,   255,   0},   // Green
  {0,     0, 255},   // Blue
  {255, 255,   0},   // Yellow
  {0,   255, 255},   // Cyan
  {255,   0, 255},   // Magenta
  {255, 165,   0},   // Orange
  {128,   0, 128},   // Purple
  {255, 105, 180},   // Pink
  {50,  205,  50},   // Lime
  {0,   128, 128},   // Teal
  {0,     0, 128},   // Navy
  {128,   0,   0},   // Maroon
  {128, 128,   0},   // Olive
  {139,  69,  19},   // Brown
  {255, 215,   0},   // Gold
  {135, 206, 235},   // Sky Blue
  {138,  43, 226},   // Violet
  {255, 127,  80},   // Coral
  {128, 128, 128}    // Gray
};


float temp = 0;
unsigned long updateTimer = 0;
bool gotTemperature = false;
int intTemp= 0;
int bitsToTurnON[6]= {0}; // Here lives the information of which LEDs WS2812b will turn ON at any given time
int bitsConverted[6]= {0};
uint32_t red, green, blue= 0;
bool firstPass= true;
int displayTemp = 0;
unsigned long updateColor = 0;
const char* ssid     = " ";
const char* password = " ";
String apiKey        = " ";
String city          = " ";
String countryCode   = "BR"; 

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected!");

  pinMode(D_in, OUTPUT);      //Configura o pino 6 como saída
  pixels.begin();             //Inicia o objeto "pixels"
  updateTimer= millis();
}

void loop() {  
  displayTemp = readTemperature();
  // Change color every 5 minutes
  if (millis() - updateColor >= 300000) {
    updateColor = millis();
    getRandomColor();
    turnLEDsON(displayTemp);
  }  
}

bool getTemperatureOnline() {
  if (WiFi.status() != WL_CONNECTED) {
    return false; // Don't try if WiFi is down
  }

  HTTPClient http;
  String serverPath = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "," + countryCode + "&APPID=" + apiKey + "&units=metric";
  
  http.begin(serverPath);
  int httpResponseCode = http.GET();
  
  if (httpResponseCode == 200) {
    String payload = http.getString();
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, payload);
    
    temp = doc["main"]["temp"];
    temp = round(temp);
    //Serial.print("Online Temp Found: ");
    http.end();
    return true; 
  } else {
    Serial.print("HTTP Error: ");
    Serial.println(httpResponseCode);
    http.end();
    return false;
  }
}

void getTemperatureOffline() {
  float sumSensor = 0;
  // Reduced delay from 5000ms to 50ms so the serial monitor actually responds
  for (int i = 0; i < 10; i++) {
    sumSensor += analogRead(lm35_PIN);
    delay(50); 
  }  
  float averageADC = sumSensor / 10.0;
  // ESP32 LM35 Calculation: (ADC_Value * Reference_Voltage / Max_ADC_Steps) / 10mv per degree
  float milliVolts = (averageADC * 3300.0) / 4095.0;
  temp = round(milliVolts / 10.0);  
}
int readTemperature() {
  if (millis() - updateTimer >= 900000 || firstPass == true) {
    updateTimer = millis();
    ensureWiFi();
    if (!getTemperatureOnline()) {
      Serial.println("Online failed. Switching to LM35...");
      getTemperatureOffline();
    }
    intTemp = int(temp);
    Serial.print("Temperature: ");
    Serial.println(intTemp);
    getRandomColor();
    turnLEDsON(intTemp);
    firstPass = false;
  }

  return intTemp;
}
void convertToBinary(int value, int bitsConverted[]) {
  for (int led = 0; led < qtdeLeds; led++) {
    int bitIndex = (qtdeLeds - 1) - led;  // MSB on LED 0
    bitsConverted[led] = bitRead(value, bitIndex);
  }
  // Debug print (MSB → LSB)
  for (int i = 0; i < qtdeLeds; i++) {
    Serial.print(bitsConverted[i]);
  }  
}

void turnLEDsON(int tempToConvert) {
  convertToBinary(tempToConvert, bitsToTurnON);
  pixels.clear();
  for (int i = 0; i < 6; i++) {
    if (bitsToTurnON[i] == 1) {
      pixels.setPixelColor(i, pixels.Color(red, green, blue));
    }
  }
  pixels.show();
}

void getRandomColor() {
  int idx = -1;

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    http.setConnectTimeout(3000);
    http.setTimeout(3000);

    http.begin("https://aisenseapi.com/services/v1/random_number/0/19");

    int httpResponseCode = http.GET();

    if (httpResponseCode == HTTP_CODE_OK) {
      String payload = http.getString();

      DynamicJsonDocument doc(128);
      DeserializationError error = deserializeJson(doc, payload);

      if (!error && doc["random_number"].is<int>()) {
        int apiNumber = doc["random_number"].as<int>();

        if (apiNumber >= 0 && apiNumber <= 19) {
          idx = apiNumber;

          Serial.print("API color index: ");
          Serial.println(idx);
        }
      }
    }

    http.end();
  }

  // Fallback only if the API request or response failed.
  if (idx == -1) {
    idx = randomRange(0, 20);
    Serial.print("Local random color index: ");
    Serial.println(idx);
  }

  red   = colorPalette[idx][0];
  green = colorPalette[idx][1];
  blue  = colorPalette[idx][2];
}


// Returns a random number in the range [0, bound)
// Avoids modulo bias using rejection sampling
uint32_t randomBounded(uint32_t bound) {
  if (bound == 0) return 0;  // avoid division by zero

  uint32_t x;
  uint32_t limit = UINT32_MAX - (UINT32_MAX % bound);

  do {
    x = esp_random();
  } while (x >= limit);  // Retry until unbiased

  return x % bound;
}

// Returns a random number in the range [minVal, maxVal)
uint32_t randomRange(uint32_t minVal, uint32_t maxVal) {
  if (maxVal <= minVal) return minVal;  
  return minVal + randomBounded(maxVal - minVal);
}
bool ensureWiFi() {
  int attempts = 0;
  const int maxAttempts = 5;

  if (WiFi.status() == WL_CONNECTED) {
    return true;
  }

  Serial.println("\nWiFi disconnected. Attempting to reconnect...");
  WiFi.disconnect(); // Clear existing failed states
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED && attempts < maxAttempts) {
    delay(2000); // Wait 2 seconds per attempt
    attempts++;
    Serial.print("Retry ");
    Serial.print(attempts);
    Serial.println("/5...");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Reconnected successfully!");
    return true;
  } else {
    Serial.println("WiFi reconnection failed.");
    return false;
  }
}
