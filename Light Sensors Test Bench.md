## Light Sensors Test Bench

### Top Level Description

- An Arduino Nano with several connected light sensors communicating via I2C, with an 8-pixel Neopixel addressable LED strip
- An operational menu is presented via the Serial monitor
- The LED light strip is used to display data collected from the sensors

### Sensor Details

- BH1750FVI Light Level Sensor
- TCS34725 RGB Color Sensor w/ White LED (on Pin 9)
- MLX90614 Non-Contact IR Temperature Sensor

### Working Library Details

- https://github.com/claws/BH1750 BH1750 by Christopher Laws
- https://github.com/DFRobot/DFRobot_TCS34725 DFRobot_TCS34725 by DFRobot
- https://github.com/DFRobot/DFRobot_MLX90614 DFRobot_MLX90614 by DFRobot
- https://github.com/adafruit/Adafruit_NeoPixel Adafruit Neopixel by Adafruit

### Development Priorities

- No dynamically allocated objects except for small arrays on the stack
- No use of the Arduino _String_ class
- RAM usage must never exceed 85%
- Flash usage must never exceed 99%
- A git checkpoint must be committed ahead of major changes if the code is currently in a working condition
- The Copilot agent is not authorized to delete files, except for temporary test and debug files, without first requesting permission
- In the event of major file corruption during a copilot update, the agent must ask permission before deleting the corrupted file
- Making a request about a feature does not mean that the copilot agent should immediately start implementing the feature 

### High Level Application Features

- Present an operational menu via the serial monitor at 115200 baud, allowing changing operation modes
- Show Details from the current operation mode on the LED Strip 

### Operation Modes

- Light Level
    - Use the LED strip to show the current light level
        - use green, yellow and red LEDs to represent increasing intensity
        - Use smoothing on the last-lit LED, simuluating a continuous meter
        - Convert the read light level to a logarithmic value so the LED strip displays something at very low and very high light levels
    - Sample at a very high continuous rate
- RGB Color
    - Use the LED strip to show the current read RGB color on the leading LED
        - Use the remaining 7 LEDs to show a sliding view of the previous 7 color readings
    - Display the sampled color at one sample per second
        - Sample the colors at 10 samples a second and average the detected color
- Non-Conect Temperature
    - Use the LED strip to show the current read temperature on the leading LED
        - Use a standard FLIR "Iron" color palette to represent the read temperature (like black body radiation)
        - Use the remaining 7 LEDs to show a sliding view of the previous 7 temperature readings
    - Display the sampled temperature at one sample per second
        - Sample the temperature at 10 samples a second and average the detected temperature
    - Also write the sample temperature once per second to the serial monitor

### Operation Menu

- Present a title explaining what it is for
- Offer a single-character option to press to switch between the three modes
    - Light Level
    - RGB Color
    - Temperature
    
### Project Setup Notes

- Create a new Arduino Project using Platformio
- The code must be uploaded and checked on the device to ensure a proper build

### Technical Clarifications

- Based on classic Nano with ATmega328P
- Uses default I2C A4 and A5 pins
- Neopixel connected to Pin 6 by default but other pins are available if necessary
- The Arduino Pin #9 (with PWM) is connected to the TCS34725 breakout LED pin, which, when set to HIGH will light the white LED

### Implemenation Details

- Light Level Scale - the light level scale should match that of the BH1750 device 1 to 65,536
    - This range should be changable via macros
- Temperature Range - the "iron" palette temperature range should be 0°C to 100°C
    - This range should be changable via macros
- Smoothing Algorithm For the "smoothing on the last-lit LED"
    - See Below Section

### LED Strip Meter Implementation Details

For the Light Level mode, the LED strip functions as a logarithmic light meter with the following characteristics:

#### Color Palette and Layout
- 8-LED strip with fixed color palette: [Green, Green, Green, Green, Green, Amber, Amber, Red]
- Each LED position has a predefined base color with maximum intensity (e.g., {0,15,0} for green)
- Colors are defined as RGB values in the range 0-15 for compatibility with various LED libraries

#### Smoothing Algorithm
- Total meter range is divided into 8 segments (one per LED)
- Each segment represents (total_range / 8) input units
- When displaying a value:
  1. Calculate how many complete LEDs should be lit: `on_leds = (input_value / segment_size) + 1`
  2. Calculate remainder: `remain = ((input_value % segment_size) * 16) / segment_size`
  3. Display first `on_leds-1` LEDs at full intensity
  4. Display the last lit LED at reduced intensity based on remainder value
  5. Turn off all remaining LEDs

#### Intensity Calculation
- The "remainder" LED has its RGB values scaled: `final_color = (base_color * remain) / 16`
- This creates smooth transitions as values increase, simulating an analog meter needle position

#### Contrast Control
- Global contrast multiplier applied to all LED RGB values after intensity calculation
- Allows brightness adjustment without changing the relative color relationships
- Should be configurable via serial menu or compile-time macro

#### Range Mapping
- For BH1750 sensor (1-65536 lux range), map logarithmically to 0-511 meter units
- Use: `meter_value = (int)(log10(lux_reading) * scale_factor)`
- Scale factor should be adjustable via macro to fine-tune display sensitivity

#### Memory Efficiency
- Use fixed-size arrays for color palettes (no dynamic allocation)
- Create temporary display buffer only during LED updates
- Copy base colors and modify in-place for efficiency

### Questions

- 8-LED Extension: Should the 8th LED be red, or extend the pattern (e.g., 5 green, 2 amber, 1 red)?
    - have 4 green, 3 amber, 1 red

- Library Choice: The example uses PololuLedStrip but your spec mentions Adafruit_NeoPixel. Should I plan to convert the algorithm to work with Adafruit's library?
    - Yes please use the NeoPixel library not Pololu

- Logarithmic Mapping: What base should be used for the logarithmic conversion of lux values? Base 10 seems most intuitive for light levels.
    - Base 10 sounds good

- Contrast Range: What should be the valid range for the contrast multiplier? (e.g., 0.1 to 2.0, or integer values 1-10?)

    - The range should be 0-15

### Project Structure

    - Nothing is set up yet except this document
    - I am anticipating asking the Copilot agent to proceed creating a new workspace and Platformio project, then begin implementing
    - Once the project is set up I will create a new git repo for it


