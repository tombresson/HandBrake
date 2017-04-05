/*
* RGBTools
* Version 1.5 July, 2016
* 
* Use this code with RGB-LEDs.
*/

#ifndef RGBTools_H
#define RGBTools_H

#ifdef DEBUGGING
	#define analogWrite(pin,val) (printf("set %d to %d\n", pin, val))
	#define delay(val) (printf("delay %lu\n", val))
	#include <stdio.h>
	#include <stdint.h>
#else
	#include <Arduino.h>
#endif


enum Mode { COMMON_ANODE, COMMON_CATHODE };

class RGBTools
{
	public:
		RGBTools(uint8_t r, uint8_t g, uint8_t b);
		RGBTools(uint8_t r, uint8_t g, uint8_t b, Mode mode);
    void setColor(uint8_t r, uint8_t g, uint8_t b);
		void setColor(uint8_t r, uint8_t g, uint8_t b, uint8_t brightness);
    void setColor(uint32_t color);
		void setColor(uint32_t color, uint8_t brightness);
		void fadeTo(uint8_t r, uint8_t g, uint8_t b, int steps, int duration);
    void blinkEnable(uint32_t rate, uint8_t duty_cycle);
    void blinkDisable(void);
    void serviceLED(void);
		
	private:
		// pins for colors
		uint8_t r_pin, g_pin, b_pin;

		// saves current state (color)
		volatile uint8_t curr_r = 0U;
		volatile uint8_t curr_g = 0U;
		volatile uint8_t curr_b = 0U;

		// output mode for common cathode or anode RGB LEDs
		uint8_t mode = COMMON_ANODE;

    uint32_t blink_rate_ms = 0U;
    uint8_t  blink_duty_cycle = 100U;
    bool     blink_enabled = false;
    unsigned long last_blink_time = 0U;
    bool     blink_led_state = true;

    void serviceBlink(void);

};

class Color {
  public:
    static const uint32_t OFF = 0x000000;
    static const uint32_t RED = 0xFF0000;
    static const uint32_t GREEN = 0x00FF00;
    static const uint32_t BLUE = 0x0000FF;
    static const uint32_t WHITE = 0xFFFFFF;
    static const uint32_t PURPLE = 0xFF00FF;
    static const uint32_t AQUAMARINE = 0x7FFFD4;
    static const uint32_t AIRFORCEBLUE = 0x5D8AA8;
    static const uint32_t AMARANTH = 0xE52B50;
    static const uint32_t ASPARAGUS = 0x87A96B;
};

#endif
