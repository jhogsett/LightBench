# Derivative Projects Design Document

This document outlines design considerations and technical approaches for two derivative projects based on the Light Sensors Test Bench.

## Project 1: Candle To Music (VS1053 MIDI Controller)

### Concept
Transform real candle flame data into MIDI music using the VS1053 audio codec module, creating an ambient musical experience that responds to flame characteristics.

### Hardware Additions
- **VS1053 MP3/MIDI Codec Module** (SPI interface)
  - Pins required: MOSI, MISO, SCK, XCS, XDCS, DREQ, RST
  - Suggested pins on Nano: D10-D13 (SPI), D2 (DREQ interrupt), D7-D8 (control)
- **Audio Output**: 3.5mm jack or small speaker
- **Optional**: Volume potentiometer on analog pin A0

### Sensor Data Mapping Strategy

#### Light Level → Musical Dynamics
```cpp
// Map light intensity to MIDI velocity (volume)
uint8_t luxToVelocity(float lux) {
  // Candle range typically 10-100 lux
  int velocity = map(constrain(lux, 10, 100), 10, 100, 40, 127);
  return constrain(velocity, 40, 127);  // Minimum audible level
}
```

#### RGB Color → Harmonic Content
```cpp
// Map color ratios to chord progressions
struct ChordProgression {
  uint8_t rootNote;
  uint8_t chordType;  // Major, minor, diminished
  uint8_t voicing;    // Inversion, extensions
};

ChordProgression colorToChord(uint8_t r, uint8_t g, uint8_t b) {
  float total = r + g + b;
  if (total == 0) return {60, 0, 0}; // C major fallback
  
  // Warm colors (red dominant) → Minor keys
  // Cool colors (blue dominant) → Major keys  
  // Green balance → Sus chords
  float redRatio = r / total;
  float blueRatio = b / total;
  
  if (redRatio > 0.5) return {57, 1, 0};      // A minor (warm/intimate)
  if (blueRatio > 0.4) return {60, 0, 0};     // C major (cool/bright)
  return {62, 2, 0};                          // D sus (balanced/floating)
}
```

#### Temperature → Tempo & Rhythm
```cpp
// Map flame temperature to musical tempo
uint16_t tempToTempo(float tempC) {
  // Candle flame ~1000°C, room temp ~25°C
  // Map temperature changes to tempo (60-120 BPM)
  if (tempC > 30) {
    // Active flame detected
    return map(constrain(tempC, 30, 50), 30, 50, 80, 120);
  }
  return 60; // Slow ambient when no flame
}
```

### Musical Implementation
- **Base Drone**: Continuous low notes based on average light level
- **Melody Layer**: Pentatonic scales triggered by light fluctuations
- **Percussion**: Subtle ambient sounds on temperature spikes
- **Harmony**: Chord pads that evolve with color changes

### Code Architecture Changes
```cpp
// Add to main.cpp
#include <VS1053.h>

class CandleMusicController {
private:
  VS1053 vs1053;
  uint8_t currentScale[5] = {60, 62, 65, 67, 70}; // C pentatonic
  unsigned long lastNoteTime = 0;
  
public:
  void updateMusic(float lux, uint8_t r, uint8_t g, uint8_t b, float temp);
  void playAmbientDrone(uint8_t velocity);
  void triggerMelodyNote(float intensity);
};
```

### Memory Considerations
- VS1053 library: ~2-4KB flash
- MIDI data tables: ~1-2KB flash  
- Total estimated: +6KB flash, +200 bytes RAM
- **Recommendation**: Upgrade to Arduino Nano Every (ATmega4809) for more memory

---

## Project 2: Real Candle to Simulated Candle

### Concept
Create a realistic LED candle simulation that mimics real flame behavior using sensor data to drive multiple LED strips in a 3D printed flame form.

### Hardware Additions
- **Multiple LED Strips**: 3-4 independent WS2812B strips (different lengths)
  - Inner core: 16 LEDs (bright center)
  - Middle ring: 24 LEDs (main flame body)  
  - Outer ring: 32 LEDs (flame tips/flicker)
  - Base glow: 8 LEDs (wax pool simulation)
- **Additional GPIO**: Use analog pins A0-A3 as digital outputs
- **3D Printed Enclosure**: Translucent flame-shaped diffuser

### Sensor Data Interpretation

#### Light Level → Overall Brightness
```cpp
// Map real candle brightness to LED intensity
uint8_t luxToBrightness(float lux) {
  // Real candle: 10-100 lux → LED brightness 20-255
  return map(constrain(lux, 5, 150), 5, 150, 20, 255);
}
```

#### RGB Color → Flame Color Temperature
```cpp
// Convert RGB ratios to realistic flame colors
uint32_t realFlameColor(uint8_t r, uint8_t g, uint8_t b, uint8_t brightness) {
  // Flame color progression: Deep red → Orange → Yellow → White
  float colorTemp = calculateColorTemperature(r, g, b);
  
  if (colorTemp < 2000) {
    // Deep red (flame base)
    return strip.Color(brightness, brightness * 0.3, 0);
  } else if (colorTemp < 2500) {
    // Orange (main flame)
    return strip.Color(brightness, brightness * 0.6, brightness * 0.1);
  } else {
    // Yellow-white (flame tips)
    return strip.Color(brightness, brightness * 0.8, brightness * 0.4);
  }
}
```

#### Temperature → Flicker Dynamics
```cpp
// Use temperature fluctuations for realistic flicker
class FlameFlicker {
private:
  float baseIntensity;
  float flickerAmount;
  unsigned long lastUpdate;
  
public:
  void updateFromTemp(float tempC) {
    float tempChange = abs(tempC - previousTemp);
    flickerAmount = map(tempChange * 100, 0, 500, 5, 40); // 5-40% flicker
  }
  
  uint8_t getFlickerBrightness(uint8_t baseBrightness) {
    float noise = sin(millis() * 0.01) * 0.3 + sin(millis() * 0.03) * 0.2;
    float flicker = 1.0 + (noise * flickerAmount / 100.0);
    return constrain(baseBrightness * flicker, 20, 255);
  }
};
```

### Multi-Strip Flame Algorithm
```cpp
class RealisticFlame {
private:
  Adafruit_NeoPixel innerCore, middleRing, outerRing, baseGlow;
  FlameFlicker flicker;
  
public:
  void updateFlame(float lux, uint8_t r, uint8_t g, uint8_t b, float temp) {
    uint8_t baseBrightness = luxToBrightness(lux);
    uint32_t flameColor = realFlameColor(r, g, b, baseBrightness);
    
    // Inner core: Brightest, most stable
    updateInnerCore(flameColor, baseBrightness * 0.9);
    
    // Middle ring: Main flame body with moderate flicker  
    updateMiddleRing(flameColor, baseBrightness * 0.7);
    
    // Outer ring: Tips with maximum flicker
    updateOuterRing(flameColor, baseBrightness * 0.5);
    
    // Base glow: Warm ambient light
    updateBaseGlow(warmWhite, baseBrightness * 0.3);
  }
  
private:
  void updateInnerCore(uint32_t color, uint8_t brightness) {
    for (int i = 0; i < innerCore.numPixels(); i++) {
      uint8_t flickerBright = flicker.getFlickerBrightness(brightness);
      innerCore.setPixelColor(i, scaleColor(color, flickerBright));
    }
    innerCore.show();
  }
};
```

### 3D Design Considerations
- **Material**: Translucent PLA or PETG for optimal light diffusion
- **Shape**: Realistic candle flame profile with varying wall thickness
  - Thinner walls at tips for brighter effect
  - Thicker base for more diffused glow
- **LED Mounting**: Internal structure to position strips at different heights
- **Ventilation**: Small holes for heat dissipation
- **Base Design**: Resembles candle wax with space for electronics

### Performance Optimizations
```cpp
// Optimize for multiple strip updates
void updateAllStrips() {
  // Use DMA or interrupt-driven updates to avoid blocking
  // Stagger strip updates across different loop cycles
  static uint8_t stripUpdate = 0;
  
  switch(stripUpdate % 4) {
    case 0: updateInnerCore(); break;
    case 1: updateMiddleRing(); break; 
    case 2: updateOuterRing(); break;
    case 3: updateBaseGlow(); break;
  }
  stripUpdate++;
}
```

### Memory Requirements
- 4 LED strips @ ~3 bytes/LED = ~240 bytes RAM for pixel data
- Flame algorithms: ~1KB flash
- **Total**: +2KB flash, +300 bytes RAM
- **Status**: Fits within current Arduino Nano limits

---

## Shared Technical Considerations

### 1. Sensor Calibration
Both projects will need candle-specific calibration:
- **Light sensor range**: 5-150 lux (candle operating range)
- **Color temperature**: 1500K-2500K (typical candle spectrum)
- **Temperature response**: Focus on 25-50°C range for flame detection

### 2. Responsiveness vs Stability
```cpp
// Adaptive filtering based on sensor noise
class AdaptiveFilter {
  float alpha = 0.7; // Default smoothing
  
  float filter(float newValue, float oldValue) {
    float change = abs(newValue - oldValue);
    // Reduce filtering for rapid changes (flame movement)
    if (change > threshold) alpha = 0.9; // More responsive
    else alpha = 0.7; // More stable
    
    return alpha * newValue + (1 - alpha) * oldValue;
  }
};
```

### 3. Power Considerations
- **VS1053 Module**: ~50mA active, ~1mA standby
- **Multiple LED strips**: Up to 1-2A at full brightness
- **Recommendation**: External 5V supply for LED projects

### 4. Enclosure Design
Both projects should consider:
- **Sensor positioning**: Clear line of sight to flame
- **Heat management**: Adequate ventilation
- **User interface**: Accessible contrast/mode controls
- **Cable management**: Clean routing for additional modules

### 5. Software Architecture
```cpp
// Modular design for easy project switching
class SensorProcessor {
  virtual void processSensorData(float lux, RGB color, float temp) = 0;
};

class MusicController : public SensorProcessor { /* ... */ };
class FlameSimulator : public SensorProcessor { /* ... */ };

// Factory pattern for project selection
SensorProcessor* createProcessor(ProjectType type) {
  switch(type) {
    case MUSIC: return new MusicController();
    case FLAME: return new FlameSimulator(); 
    default: return new StandardDisplay();
  }
}
```

### Next Steps for Implementation
1. **Prototype individual components** (VS1053 or multi-strip LEDs)
2. **Validate sensor ranges** with actual candles
3. **Create modular firmware** that can switch between projects
4. **Design and print enclosures** for target form factors
5. **Performance testing** with complete hardware setups

Both projects leverage the existing sensor foundation while adding compelling new interaction modalities - one auditory, one visual - creating immersive experiences that bridge the digital and physical worlds.
