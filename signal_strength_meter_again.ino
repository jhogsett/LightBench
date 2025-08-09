#include <Arduino.h>

#include <PololuLedStrip.h>

// Create an ledStrip object and specify the pin it will use.
PololuLedStrip<6> ledStrip;

// Create a buffer for holding the colors (3 bytes per color).
#define LED_COUNT 7
rgb_color colors[LED_COUNT] = 
{
  { 0, 15, 0 }, 
  { 0, 15, 0 }, 
  { 0, 15, 0 }, 
  { 0, 15, 0 }, 
  { 15, 15, 0 }, 
  { 15, 15, 0 }, 
  { 15, 0, 0 } 
};

rgb_color empty[LED_COUNT] = 
{
  { 0, 0, 0 }, 
  { 0, 0, 0 }, 
  { 0, 0, 0 }, 
  { 0, 0, 0 }, 
  { 0, 0, 0 }, 
  { 0, 0, 0 }, 
  { 0, 0, 0 } 
};


#define INTERVAL 100
unsigned long next_time = 0;
int value = 127;
int option_contrast = 1;

void step_sm(unsigned long time)
{
	if(time < next_time)
		return;
	next_time = time + INTERVAL;

	int r = random(11) - 5;
	value += r;

	if(value > 255)
		value = 255;
	if(value < 0)
		value = 0;

	int sample = value * 2;
	int on_leds = (sample / 73) + 1;
	int remain = ((sample % 73) * 16) / 73;

	rgb_color dbuffer[LED_COUNT];
	memcpy(dbuffer, colors, on_leds * sizeof(rgb_color));
	memcpy(dbuffer + on_leds, empty, (LED_COUNT - on_leds) * sizeof(rgb_color));

	// for the last copied LED, modify it according to the remain value
	dbuffer[on_leds-1].red = (dbuffer[on_leds-1].red * remain) / 16;
	dbuffer[on_leds-1].green = (dbuffer[on_leds-1].green * remain) / 16;
	dbuffer[on_leds-1].blue = (dbuffer[on_leds-1].blue * remain) / 16;

	// modify whole display per display contrast
	for(byte i = 0; i < LED_COUNT; i++){
		dbuffer[i].red = (dbuffer[i].red * option_contrast)	/ 1;
		dbuffer[i].green = (dbuffer[i].green * option_contrast)	/ 1;
		dbuffer[i].blue = (dbuffer[i].blue * option_contrast)	/ 1;
	}

	ledStrip.write(dbuffer, LED_COUNT);
}

void setup(){

}	


void loop()
{
	while(true){
        unsigned long time = millis();

		step_sm(time);

	}
}

