# Light Sensors Test Bench

Arduino Nano-based multi-sensor display system with 8-LED NeoPixel strip for visualizing sensor data.

## Hardware Requirements

- Arduino Nano (ATmega328P)
- BH1750FVI Light Level Sensor
- TCS34725 RGB Color Sensor w/ White LED
- MLX90614 Non-Contact IR Temperature Sensor
- 8-pixel Adafruit NeoPixel LED strip
- Breadboard and jumper wires

## Wiring Diagram

| Component | Arduino Pin | Notes |
|-----------|-------------|-------|
| All I2C Sensors | A4 (SDA), A5 (SCL) | Connect all sensors to I2C bus |
| NeoPixel Strip | Pin 6 | Data input |
| TCS34725 LED | Pin 9 | White LED control (PWM) |
| Power | 5V, GND | Connect to all components |

## Operation Modes

### Light Level Mode (L)
- Displays logarithmic light intensity as an LED meter
- Color progression: Green → Amber → Red
- Smooth transitions with fractional LED brightness
- High-speed continuous sampling

### RGB Color Mode (C)
- Shows current detected color on first LED
- Previous 7 color readings scroll through remaining LEDs
- 10 samples per second averaged, displayed at 1Hz
- Automatically enables TCS34725 white LED

### Temperature Mode (T)
- Current temperature shown on first LED using FLIR Iron palette
- Previous 7 temperature readings scroll through remaining LEDs
- 10 samples per second averaged, displayed at 1Hz
- Temperature values printed to serial monitor

## Serial Commands

Connect at 115200 baud and use these single-character commands:

- `L` - Switch to Light Level Mode
- `C` - Switch to RGB Color Mode  
- `T` - Switch to Temperature Mode
- `+` - Increase LED contrast
- `-` - Decrease LED contrast
- `?` or `H` - Show help menu

## Build Instructions

1. Install PlatformIO IDE or CLI
2. Clone this repository
3. Open project in PlatformIO
4. Build and upload to Arduino Nano

```bash
# Using PlatformIO CLI
pio run --target upload
```

## Libraries Used

- [BH1750](https://github.com/claws/BH1750) by Christopher Laws
- [DFRobot_TCS34725](https://github.com/DFRobot/DFRobot_TCS34725) by DFRobot
- [DFRobot_MLX90614](https://github.com/DFRobot/DFRobot_MLX90614) by DFRobot
- [Adafruit NeoPixel](https://github.com/adafruit/Adafruit_NeoPixel) by Adafruit

## Memory Usage

The code is optimized for Arduino Nano's limited resources:
- No dynamic memory allocation
- No Arduino String class usage
- Optimized build flags for size reduction
- Target: <85% RAM usage, <99% Flash usage

## Development Notes

- All timing uses non-blocking `millis()` based approach
- Sensor error handling included
- Fixed-size arrays for efficient memory usage
- Logarithmic scaling for light sensor provides good dynamic range
- FLIR Iron color palette provides intuitive temperature visualization
