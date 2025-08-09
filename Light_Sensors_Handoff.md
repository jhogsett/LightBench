# Light Sensors Test Bench - Copilot Handoff Document

## Project Overview
This is an Arduino Nano (ATmega328P) project implementing a multi-sensor light measurement system with LED strip visualization. The user has provided a complete specification document and working LED meter example code.

## Key Files to Reference
1. **Main Spec**: `c:\Users\jhogs\OneDrive\Documents\Light Sensors Test Bench.md` - Complete project specification
2. **LED Example**: `c:\Users\jhogs\OneDrive\Documents\signal_strength_meter_again.ino` - Working 7-LED meter implementation

## Critical Implementation Requirements

### Hardware Configuration
- Arduino Nano (ATmega328P)
- I2C sensors on A4/A5 (default pins)
- NeoPixel strip on Pin 6 (8 LEDs)
- TCS34725 white LED control on Pin 9
- Serial at 115200 baud

### Memory Constraints (CRITICAL)
- RAM usage: NEVER exceed 85%
- Flash usage: NEVER exceed 99%
- NO dynamic allocation except small stack arrays
- NO Arduino String class

### Sensors & Libraries (Exact versions specified)
- BH1750FVI: https://github.com/claws/BH1750
- TCS34725: https://github.com/DFRobot/DFRobot_TCS34725
- MLX90614: https://github.com/DFRobot/DFRobot_MLX90614
- NeoPixel: https://github.com/adafruit/Adafruit_NeoPixel

### LED Strip Implementation (From Working Example)
The user provided a working 7-LED meter in `signal_strength_meter_again.ino` that demonstrates:
- Color palette: [4 green, 3 amber, 1 red] for 8-LED version
- Smoothing algorithm with fractional intensity on last LED
- Contrast control (0-15 range)
- Memory-efficient fixed arrays

### Three Operation Modes
1. **Light Level**: Continuous sampling, logarithmic LED meter display
2. **RGB Color**: 1Hz display, 10Hz sampling with averaging, color history on strip
3. **Temperature**: 1Hz display, 10Hz sampling with averaging, FLIR "Iron" palette, serial output

### Development Workflow
- Must commit to git before major changes
- Must ask permission before deleting files
- Must test on actual hardware
- Must verify memory usage

## Implementation Priority Order
1. Create PlatformIO project structure
2. Implement basic LED strip control (adapt from working example)
3. Add sensor initialization and basic reading
4. Implement serial menu system
5. Add each operation mode incrementally
6. Test memory usage at each step

## Critical Notes
- User has working LED meter code to reference
- Must use Adafruit NeoPixel library (not Pololu)
- Logarithmic scaling: Base 10, configurable via macros
- All ranges must be macro-configurable
- Code must be uploaded and tested on hardware

## Next Steps
1. Create new PlatformIO workspace
2. Set up project with proper libraries
3. Begin with LED strip implementation using provided example as reference
4. Test early and often on hardware

The user is experienced and has provided excellent specifications. Follow them precisely, especially memory constraints and the requirement for hardware testing.
