

/**
   @class RGBTools

   @brief Use this code with RGB-LEDs

   @detail This class implements methods to control a RGB LED with Arduino
*/

#include "RGBTools.h"

/**
   constructor; saves the pins

   @param[in] r The Arduino pin number of the red pin of the LED
   @param[in] g The Arduino pin number of the green pin of the LED
   @param[in] b The Arduino pin number of the blue pin of the LED
*/
RGBTools::RGBTools(uint8_t r, uint8_t g, uint8_t b) {
  this->r_pin = r;
  this->g_pin = g;
  this->b_pin = b;

  pinMode(r, OUTPUT);
  pinMode(g, OUTPUT);
  pinMode(b, OUTPUT);
}

/**
   Constructor using the Mode enum in header file.
   Ex: RGBTools(9, 10, 11, COMMON_ANODE);

   @param[in] r The Arduino pin number of the red pin of the LED
   @param[in] g The Arduino pin number of the green pin of the LED
   @param[in] b The Arduino pin number of the blue pin of the LED
   @param[in] mode The mode of the RGB (common cathode / common anode)
*/
RGBTools::RGBTools(uint8_t r, uint8_t g, uint8_t b, Mode mode) {
  this->r_pin = r;
  this->g_pin = g;
  this->b_pin = b;

  this->mode = mode;
}

/**
   Set LED-color to custom color instantly

   @param[in] r The red value ([0-255])
   @param[in] g The green value ([0-255])
   @param[in] b The blue value ([0-255])
*/
void RGBTools::setColor(uint8_t r, uint8_t g, uint8_t b) {

  // set color of LED
  if (mode == COMMON_CATHODE)
  {
    analogWrite(r_pin, r);
    analogWrite(g_pin, g);
    analogWrite(b_pin, b);
  }
  else
  {
    analogWrite(r_pin, 256 - r);
    analogWrite(g_pin, 256 - g);
    analogWrite(b_pin, 256 - b);
  }

  // set color and save state if color indicates not off
  if ((r != 0U) || (b != 0U) || (g != 0U))
  {
    this->curr_r = r;
    this->curr_g = g;
    this->curr_b = b;
  }
}

/**
   Set LED-color to custom color instantly Uses a 32 bit value to set a color.

   @param[in] color The 32 bit color value (eg. 0xFF0000 for red)
*/
void RGBTools::setColor(uint32_t color) {
  uint8_t red =   (uint8_t)   ((color & 0xFF0000) >> 16);
  uint8_t green = (uint8_t)   ((color & 0x00FF00) >> 8);
  uint8_t blue = (uint8_t)     (color & 0x0000FF);
  setColor(red, green, blue);
}

/**
   Fades to custom color in specific time in specific steps

   @param[in] r The red value ([0-255])
   @param[in] g The green value ([0-255])
   @param[in] b The blue value ([0-255])
   @param[in] steps The number of steps to use
   @param[in] duration The duration of the fading in ms
*/
void RGBTools::fadeTo(uint8_t r, uint8_t g, uint8_t b, int steps, int duration) {
  // calculate difference to target
  uint8_t diff_r = r - curr_r;
  uint8_t diff_g = g - curr_g;
  uint8_t diff_b = b - curr_b;

  // calculate the width of each step multiplied by 100 as to not lose decimal data
  int steps_r = (diff_r * 100 / steps);
  int steps_g = (diff_g * 100 / steps);
  int steps_b = (diff_b * 100 / steps);

  // calculate delay between steps
  int delay_value = (duration / steps);

  // loop through the steps (i: distance to target)
  for (int i = steps - 1; i >= 0; i--) {

    // set color of current step
    this->setColor(
      r - (i * steps_r) / 100,	// red target minus distance to target steps (divided by 100 as to offset steps_r calculations)
      g - (i * steps_g) / 100,	// green
      b - (i * steps_b) / 100 	// blue
    );

    // delay until next step
    delay((unsigned long) delay_value);
  }
}

/**
   @brief Enables blinking with a specific duty cycle and rate
   @param[in] blink_rate The rate, in ms, in which the LED will blink
   @param[in] duty_cycle The duty cycle (0-100) of the blinking. 0 will be
   completly off, and 100 will be completely on.
*/
void RGBTools::blinkEnable(uint32_t rate, uint8_t duty_cycle)
{
  // Only blink if the rate is greater than zero
  if(rate > 0U)
  {
    if (duty_cycle == 100U)
    {
      // Disable blinking
      this->blink_enabled = false;

      // turn LED back on fully
      this->setColor(this->curr_r, this->curr_g, this->curr_b);

      // Reset LED state
      this->blink_led_state = true;
    }
    else if ( duty_cycle == 0U)
    {
      this->setColor(Color::OFF);
    }
    else
    {
      this->blink_rate_ms = rate;

      // limit duty cycle to 100
      this->blink_duty_cycle = min(duty_cycle, 100);

      this->blink_enabled = true;
      this->blink_led_state = true;
    }
  }
  else
  {
    blinkDisable();
  }
}

void RGBTools::blinkDisable(void)
{
  // Disable blinking
  this->blink_enabled = false;

  // turn LED back on fully
  this->setColor(this->curr_r, this->curr_g, this->curr_b);

  // Reset LED state
  this->blink_led_state = true;
}

void RGBTools::serviceBlink(void)
{
  // check to see if it's time to blink the LED; that is, if the
  // difference between the current time and last time you blinked
  // the LED is bigger than the interval at which you want to
  // blink the LED.

  unsigned long currentMillis = millis();

  uint32_t on_time  = (uint32_t)((float)this->blink_rate_ms *
                                 ((float)this->blink_duty_cycle / 100.0F));
  uint32_t off_time = (uint32_t)((float)this->blink_rate_ms *
                                 ((100.0F - (float)this->blink_duty_cycle) / 100.0F));

  if (((currentMillis - this->last_blink_time) >= on_time) && this->blink_led_state)
  {
    // save the last time you blinked the LED
    this->last_blink_time = currentMillis;

    // Change the LED state
    this->setColor(Color::OFF);
    this->blink_led_state = false;
  }
  else if (((currentMillis - this->last_blink_time) >= off_time) && !(this->blink_led_state))
  {
    // save the last time you blinked the LED
    this->last_blink_time = currentMillis;

    // Change the LED state
    this->setColor(this->curr_r, this->curr_g, this->curr_b);
    this->blink_led_state = true;
  }
  else
  {
    // Do nothing...
  }
}

void RGBTools::serviceLED(void)
{
  //this->serviceFade();
  if (this->blink_enabled)
  {
    this->serviceBlink();
  }
}

