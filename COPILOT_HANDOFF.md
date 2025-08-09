# Light Sensors Test Bench - Copilot Handoff Document

## Project Overview
Arduino Nano-based multi-sensor display system with 8-LED NeoPixel strip for visualizing sensor data through three operational modes.

## Hardware Configuration
- **Microcontroller**: Arduino Nano (ATmega328P)
- **I2C Sensors**: 
  - BH1750FVI Light Level Sensor (A4/A5 pins)
  - TCS34725 RGB Color Sensor (A4/A5 pins)
  - MLX90614 Non-Contact IR Temperature Sensor (A4/A5 pins)
- **LED Strip**: 8-pixel Adafruit NeoPixel (Pin 6)
- **Additional**: TCS34725 White LED control (Pin 9, PWM)

## Required Libraries
1. **BH1750** by Christopher Laws: https://github.com/claws/BH1750
2. **DFRobot_TCS34725** by DFRobot: https://github.com/DFRobot/DFRobot_TCS34725
3. **DFRobot_MLX90614** by DFRobot: https://github.com/DFRobot/DFRobot_MLX90614
4. **Adafruit_NeoPixel** by Adafruit: https://github.com/adafruit/Adafruit_NeoPixel

## Critical Development Constraints
- **NO** dynamic memory allocation (except small stack arrays)
- **NO** Arduino String class usage
- **RAM usage < 85%**
- **Flash usage < 99%**
- Git checkpoint required before major changes
- Agent cannot delete files without permission

## Three Operation Modes

### 1. Light Level Mode
- **Display**: LED meter showing logarithmic light intensity
- **Color Pattern**: 4 Green → 3 Amber → 1 Red LEDs
- **Algorithm**: 8-segment meter with smoothing on last-lit LED
- **Sampling**: High continuous rate
- **Range**: 1-65,536 lux (BH1750), mapped to 0-511 meter units
- **Formula**: `meter_value = (int)(log10(lux_reading) * scale_factor)`

### 2. RGB Color Mode
- **Display**: Current color on LED 0, previous 7 colors on LEDs 1-7
- **Sampling**: 10 samples/sec averaged, display at 1 sample/sec
- **White LED**: Control via Pin 9 (HIGH = on)

### 3. Temperature Mode
- **Display**: Current temp on LED 0 (FLIR Iron palette), previous 7 on LEDs 1-7
- **Sampling**: 10 samples/sec averaged, display at 1 sample/sec
- **Range**: 0°C to 100°C
- **Serial Output**: Temperature readings to serial monitor (1/sec)

## LED Meter Algorithm (Critical Implementation Detail)

```cpp
// Based on working example from signal_strength_meter_again.ino
// Color palette: 4 green, 3 amber, 1 red
// Total range divided into 8 segments
int on_leds = (input_value / segment_size) + 1;
int remain = ((input_value % segment_size) * 16) / segment_size;

// Apply base colors to first (on_leds-1) LEDs at full intensity
// Scale last LED: final_color = (base_color * remain) / 16
// Apply contrast: all_colors = (color * contrast_0_to_15) / 15
```

## Serial Menu System
- **Baud Rate**: 115200
- **Interface**: Single-character commands for mode switching
- **Options**: Light Level, RGB Color, Temperature modes
- **Contrast Control**: Adjustable 0-15 range

## Implementation Priority
1. Create PlatformIO project structure
2. Set up basic sensor initialization
3. Implement LED meter algorithm (use Adafruit NeoPixel, not Pololu)
4. Add serial menu system
5. Implement all three sensor modes
6. Test and validate memory usage
7. Upload and verify on actual hardware

## Key Macros to Define
- Light level scale range (1-65536)
- Temperature range (0-100°C)
- Scale factors for logarithmic mapping
- Contrast range (0-15)
- LED color definitions

## Files to Reference
- Original specification: `Light Sensors Test Bench.md`
- Working LED algorithm: `signal_strength_meter_again.ino`

## Project Status
- Specifications complete ✅
- Ready to implement ⏳
