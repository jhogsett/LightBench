/**
 * Light Sensors Test Bench
 * 
 * Arduino Nano-based multi-sensor display system with 8-LED NeoPixel strip
 * 
 * Hardware:
 * - Arduino Nano (ATmega328P)
 * - BH1750FVI Light Level Sensor (I2C)
 * - TCS34725 RGB Color Sensor (I2C) 
 * - MLX90614 Non-Contact IR Temperature Sensor (I2C)
 * - 8-pixel Adafruit NeoPixel strip (Pin 6)
 * - TCS34725 White LED control (Pin 9)
 * 
 * Serial Monitor: 115200 baud
 */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_NeoPixel.h>
#include <BH1750.h>
#include <Adafruit_TCS34725.h>
#include <Adafruit_MLX90614.h>
#include <math.h>

// Pin definitions
#define NEOPIXEL_PIN 6
#define TCS_LED_PIN 9
#define LED_COUNT 8

// Configuration macros
#define LIGHT_MIN_LUX 1
#define LIGHT_MAX_LUX 65536
#define TEMP_MIN_C 0
#define TEMP_MAX_C 100
#define CONTRAST_MIN 0
#define CONTRAST_MAX 15
#define METER_RANGE 512  // 0-511 for 8-segment meter

// Timing constants
#define FAST_SAMPLE_INTERVAL 100    // Light level sampling (ms)
#define SLOW_SAMPLE_INTERVAL 100    // For averaging (10 samples/sec)
#define DISPLAY_INTERVAL 1000       // Display update interval (ms)

// Operation modes
enum OperationMode {
  MODE_LIGHT_LEVEL = 0,
  MODE_RGB_COLOR = 1,
  MODE_TEMPERATURE = 2
};

// Global variables
Adafruit_NeoPixel strip(LED_COUNT, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);
BH1750 lightMeter;
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_614MS, TCS34725_GAIN_1X);
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

OperationMode currentMode = MODE_LIGHT_LEVEL;
uint8_t contrast = 8;  // Default contrast (0-15)

// LED color palette for light meter: 4 green, 3 amber, 1 red
uint32_t meterColors[LED_COUNT] = {
  strip.Color(0, 255, 0),    // Green
  strip.Color(0, 255, 0),    // Green  
  strip.Color(0, 255, 0),    // Green
  strip.Color(0, 255, 0),    // Green
  strip.Color(255, 191, 0),  // Amber
  strip.Color(255, 191, 0),  // Amber
  strip.Color(255, 191, 0),  // Amber
  strip.Color(255, 0, 0)     // Red
};

// RGB color history for color mode
uint32_t colorHistory[LED_COUNT];

// Temperature history for temperature mode
float tempHistory[LED_COUNT];

// Timing variables
unsigned long lastFastSample = 0;
unsigned long lastSlowSample = 0;
unsigned long lastDisplay = 0;

// Sampling arrays for averaging
float tempSamples[10];
uint16_t colorSamples[10][3];  // R, G, B
uint8_t colorSampleIndex = 0;
uint8_t tempSampleIndex = 0;

// Function declarations
void showMenu();
void handleSerialInput();
void runLightLevelMode(unsigned long currentTime);
void runRGBColorMode(unsigned long currentTime);
void runTemperatureMode(unsigned long currentTime);
void displayMeter(int value);
void displayColorHistory();
void displayTemperatureHistory();
uint32_t temperatureToIronColor(float temp);
uint32_t applyContrast(uint32_t color);

void setup() {
  Serial.begin(115200);
  Wire.begin();
  
  // Initialize LED strip
  strip.begin();
  strip.show();
  
  // Initialize TCS LED control pin
  pinMode(TCS_LED_PIN, OUTPUT);
  digitalWrite(TCS_LED_PIN, LOW);
  
  // Initialize sensors
  if (!lightMeter.begin()) {
    Serial.println(F("Error: BH1750 sensor not found"));
  }
  
  if (!tcs.begin()) {
    Serial.println(F("Error: TCS34725 sensor not found"));
  }
  
  if (!mlx.begin()) {
    Serial.println(F("Error: MLX90614 sensor not found"));
  }
  
  // Initialize arrays
  memset(colorHistory, 0, sizeof(colorHistory));
  memset(tempHistory, 0, sizeof(tempHistory));
  memset(tempSamples, 0, sizeof(tempSamples));
  memset(colorSamples, 0, sizeof(colorSamples));
  
  // Show startup menu
  showMenu();
}

void loop() {
  unsigned long currentTime = millis();
  
  // Handle serial input
  if (Serial.available()) {
    handleSerialInput();
  }
  
  // Execute current mode
  switch (currentMode) {
    case MODE_LIGHT_LEVEL:
      runLightLevelMode(currentTime);
      break;
    case MODE_RGB_COLOR:
      runRGBColorMode(currentTime);
      break;
    case MODE_TEMPERATURE:
      runTemperatureMode(currentTime);
      break;
  }
}

void showMenu() {
  Serial.println(F("\n=== Light Sensors Test Bench ==="));
  Serial.println(F("Select operation mode:"));
  Serial.println(F("L - Light Level Mode"));
  Serial.println(F("C - RGB Color Mode"));
  Serial.println(F("T - Temperature Mode"));
  Serial.println(F("+ - Increase Contrast"));
  Serial.println(F("- - Decrease Contrast"));
  Serial.print(F("Current contrast: "));
  Serial.println(contrast);
  Serial.println();
}

void handleSerialInput() {
  char input = Serial.read();
  
  switch (input) {
    case 'L':
    case 'l':
      currentMode = MODE_LIGHT_LEVEL;
      Serial.println(F("Switched to Light Level Mode"));
      break;
    case 'C':
    case 'c':
      currentMode = MODE_RGB_COLOR;
      Serial.println(F("Switched to RGB Color Mode"));
      digitalWrite(TCS_LED_PIN, HIGH);  // Turn on white LED
      // Clear color sample arrays for clean start
      memset(colorSamples, 0, sizeof(colorSamples));
      colorSampleIndex = 0;
      break;
    case 'T':
    case 't':
      currentMode = MODE_TEMPERATURE;
      Serial.println(F("Switched to Temperature Mode"));
      digitalWrite(TCS_LED_PIN, LOW);   // Turn off white LED
      // Clear temperature sample arrays for clean start
      memset(tempSamples, 0, sizeof(tempSamples));
      tempSampleIndex = 0;
      break;
    case '+':
      if (contrast < CONTRAST_MAX) {
        contrast++;
        Serial.print(F("Contrast: "));
        Serial.println(contrast);
      }
      break;
    case '-':
      if (contrast > CONTRAST_MIN) {
        contrast--;
        Serial.print(F("Contrast: "));
        Serial.println(contrast);
      }
      break;
    case '?':
    case 'h':
    case 'H':
      showMenu();
      break;
  }
}

void runLightLevelMode(unsigned long currentTime) {
  if (currentTime - lastFastSample >= FAST_SAMPLE_INTERVAL) {
    lastFastSample = currentTime;
    
    float lux = lightMeter.readLightLevel();
    if (lux >= 0) {
      // Convert to logarithmic scale and map to meter range
      float logLux = log10(constrain(lux, LIGHT_MIN_LUX, LIGHT_MAX_LUX));
      float logMin = log10(LIGHT_MIN_LUX);
      float logMax = log10(LIGHT_MAX_LUX);
      
      int meterValue = (int)((logLux - logMin) / (logMax - logMin) * METER_RANGE);
      meterValue = constrain(meterValue, 0, METER_RANGE - 1);
      
      displayMeter(meterValue);
    }
  }
}

void runRGBColorMode(unsigned long currentTime) {
  // Sample colors at 10Hz for averaging
  if (currentTime - lastSlowSample >= SLOW_SAMPLE_INTERVAL) {
    lastSlowSample = currentTime;
    
    uint16_t r, g, b, c;
    tcs.getRawData(&r, &g, &b, &c);
    
    colorSamples[colorSampleIndex][0] = r;
    colorSamples[colorSampleIndex][1] = g;
    colorSamples[colorSampleIndex][2] = b;
    
    colorSampleIndex = (colorSampleIndex + 1) % 10;
  }
  
  // Update display at 1Hz - average the samples and add to history
  if (currentTime - lastDisplay >= DISPLAY_INTERVAL) {
    lastDisplay = currentTime;
    
    // Calculate average color from the last 10 samples
    uint32_t avgR = 0, avgG = 0, avgB = 0;
    for (int i = 0; i < 10; i++) {
      avgR += colorSamples[i][0];
      avgG += colorSamples[i][1];
      avgB += colorSamples[i][2];
    }
    avgR /= 10;
    avgG /= 10;
    avgB /= 10;
    
    // Normalize to 0-255 range using ratio-based approach for better color representation
    uint8_t red, green, blue;
    if (avgR + avgG + avgB > 0) {
      // Calculate ratios and scale them appropriately
      float total = avgR + avgG + avgB;
      red = (uint8_t)constrain((avgR * 255.0) / total, 0, 255);
      green = (uint8_t)constrain((avgG * 255.0) / total, 0, 255);
      blue = (uint8_t)constrain((avgB * 255.0) / total, 0, 255);
      
      // Apply brightness scaling based on overall intensity
      float brightness = constrain(total / 30000.0, 0.1, 1.0);  // Adjust this divisor as needed
      red = (uint8_t)(red * brightness);
      green = (uint8_t)(green * brightness);
      blue = (uint8_t)(blue * brightness);
    } else {
      red = green = blue = 0;
    }
    
    uint32_t currentColor = strip.Color(red, green, blue);
    
    // Shift history - move each color one position to the right
    for (int i = LED_COUNT - 1; i > 0; i--) {
      colorHistory[i] = colorHistory[i - 1];
    }
    colorHistory[0] = currentColor;
    
    displayColorHistory();
  }
}

void runTemperatureMode(unsigned long currentTime) {
  // Sample temperature at 10Hz for averaging
  if (currentTime - lastSlowSample >= SLOW_SAMPLE_INTERVAL) {
    lastSlowSample = currentTime;
    
    float temp = mlx.readObjectTempC();
    tempSamples[tempSampleIndex] = temp;
    tempSampleIndex = (tempSampleIndex + 1) % 10;
  }
  
  // Update display and serial output at 1Hz
  if (currentTime - lastDisplay >= DISPLAY_INTERVAL) {
    lastDisplay = currentTime;
    
    // Calculate average temperature
    float avgTemp = 0;
    for (int i = 0; i < 10; i++) {
      avgTemp += tempSamples[i];
    }
    avgTemp /= 10;
    
    // Output to serial
    Serial.print(F("Temperature: "));
    Serial.print(avgTemp, 1);
    Serial.println(F("°C"));
    
    // Shift history
    for (int i = LED_COUNT - 1; i > 0; i--) {
      tempHistory[i] = tempHistory[i - 1];
    }
    tempHistory[0] = avgTemp;
    
    displayTemperatureHistory();
  }
}

void displayMeter(int value) {
  // Calculate how many complete LEDs to light
  int segmentSize = METER_RANGE / LED_COUNT;
  int onLeds = (value / segmentSize) + 1;
  int remain = ((value % segmentSize) * 16) / segmentSize;
  
  onLeds = constrain(onLeds, 0, LED_COUNT);
  
  // Clear all pixels
  strip.clear();
  
  // Set full intensity LEDs
  for (int i = 0; i < onLeds - 1; i++) {
    uint32_t color = applyContrast(meterColors[i]);
    strip.setPixelColor(i, color);
  }
  
  // Set the smoothed last LED
  if (onLeds > 0) {
    uint32_t baseColor = meterColors[onLeds - 1];
    uint8_t r = (uint8_t)((baseColor >> 16) & 0xFF);
    uint8_t g = (uint8_t)((baseColor >> 8) & 0xFF);
    uint8_t b = (uint8_t)(baseColor & 0xFF);
    
    r = (r * remain) / 16;
    g = (g * remain) / 16;
    b = (b * remain) / 16;
    
    uint32_t smoothColor = strip.Color(r, g, b);
    smoothColor = applyContrast(smoothColor);
    strip.setPixelColor(onLeds - 1, smoothColor);
  }
  
  strip.show();
}

void displayColorHistory() {
  for (int i = 0; i < LED_COUNT; i++) {
    uint32_t color = applyContrast(colorHistory[i]);
    strip.setPixelColor(i, color);
  }
  strip.show();
}

void displayTemperatureHistory() {
  for (int i = 0; i < LED_COUNT; i++) {
    uint32_t color = temperatureToIronColor(tempHistory[i]);
    color = applyContrast(color);
    strip.setPixelColor(i, color);
  }
  strip.show();
}

uint32_t temperatureToIronColor(float temp) {
  // FLIR Iron color palette mapping (0-100°C)
  temp = constrain(temp, TEMP_MIN_C, TEMP_MAX_C);
  float normalized = temp / (TEMP_MAX_C - TEMP_MIN_C);
  
  uint8_t r, g, b;
  
  if (normalized < 0.25) {
    // Black to purple
    r = (uint8_t)(normalized * 4 * 128);
    g = 0;
    b = (uint8_t)(normalized * 4 * 255);
  } else if (normalized < 0.5) {
    // Purple to red
    float local = (normalized - 0.25) * 4;
    r = (uint8_t)(128 + local * 127);
    g = 0;
    b = (uint8_t)(255 * (1 - local));
  } else if (normalized < 0.75) {
    // Red to yellow
    float local = (normalized - 0.5) * 4;
    r = 255;
    g = (uint8_t)(local * 255);
    b = 0;
  } else {
    // Yellow to white
    float local = (normalized - 0.75) * 4;
    r = 255;
    g = 255;
    b = (uint8_t)(local * 255);
  }
  
  return strip.Color(r, g, b);
}

uint32_t applyContrast(uint32_t color) {
  uint8_t r = (uint8_t)((color >> 16) & 0xFF);
  uint8_t g = (uint8_t)((color >> 8) & 0xFF);
  uint8_t b = (uint8_t)(color & 0xFF);
  
  r = (r * contrast) / CONTRAST_MAX;
  g = (g * contrast) / CONTRAST_MAX;
  b = (b * contrast) / CONTRAST_MAX;
  
  return strip.Color(r, g, b);
}
