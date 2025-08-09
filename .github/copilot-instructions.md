# Light Sensors Test Bench - Copilot Instructions

This is an Arduino Nano project for a multi-sensor display system using PlatformIO.

## Development Constraints
- NO dynamic memory allocation (except small stack arrays)
- NO Arduino String class usage  
- RAM usage must stay under 85%
- Flash usage must stay under 99%
- Use fixed-size arrays and efficient algorithms
- Git checkpoint required before major changes

## Hardware Configuration
- Arduino Nano (ATmega328P)
- I2C sensors on A4/A5: BH1750, TCS34725, MLX90614
- NeoPixel strip on Pin 6 (8 LEDs)
- TCS34725 white LED control on Pin 9

## Key Implementation Details
- LED meter algorithm uses 8-segment smoothing with remainder calculation
- Color palette: 4 green, 3 amber, 1 red LEDs for light meter
- Temperature uses FLIR Iron color palette (black→purple→red→yellow→white)
- All modes use non-blocking timing with millis()
- Serial interface at 115200 baud with single-character commands

## Code Organization
- main.cpp contains all functionality
- Libraries: BH1750, DFRobot_TCS34725, DFRobot_MLX90614, Adafruit_NeoPixel
- Three operation modes: Light Level, RGB Color, Temperature
- Contrast control (0-15 range) affects all LED outputs

When making changes, always verify memory usage and test on actual hardware.
