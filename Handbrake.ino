/* Complete USB Joystick Example
   Teensy becomes a USB joystick with 16 or 32 buttons and 6 axis input

   You must select Joystick from the "Tools > USB Type" menu

   Pushbuttons should be connected between the digital pins and ground.
   Potentiometers should be connected to analog inputs 0 to 5.

   This example code is in the public domain.
*/

/************************************* INCLUDES *************************************/

#include <stdint.h>


#include <EEPROMex.h>
#include <EEPROMVar.h>

#include "utilities.h"
#include "RGBTools.h"

/************************************** MACROS **************************************/
#define COUNT_OF(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

// Turns Debug on and off
#define HANDBRAKE_DEBUG                   1

// Turn on verbose debugging 
#define HANDBRAKE_DEBUG_VERBOSE           0

// EEPROM abstracted address position
#define HANDBRAKE_EEPROM_ADDR             0U

// Buffer for serial data
#define HANDBRAKE_SERIAL_BUFF_SIZE        64U

#define HANDBRAKE_ADC_MAX                 1023U

#define HANDBRAKE_JOY_AXIS_MAX            1023U

// Button to send for the joystick button threshold
#define HANDBRAKE_JOY_BUTTON              1U

// Duration of button hold to enter calibration mode
#define HANDBRAKE_BUTTON_HOLD_TIME_MS     5000U

// Serial Config Parsing Timeout
#define HANDBRAKE_SERIAL_TIMEOUT          5000

// Resting value for joystick position
#define JOYSTICK_RESTING_POS              512U

#define HANDBRAKE_POSITION_MAX            100.0F
#define HANDBRAKE_POSITION_MIN            0.0F

// The hysteresis (in percentage) that in used for the button/key press
#define HANDBRAKE_THRESH_HYSTERESIS       5

#define HANDBRAKE_LED_DEFAULT_BRIGHTNESS  100

// The percentage of the total range that the calibration is narrowed by
// This will ensure that the analog axis travels from 0 to 100%
// Upper deadband is for fully pulled back position, lower deadband is for released position
#define HANDBRAKE_UPPER_DEADZONE_BAND           5U
#define HANDBRAKE_LOWER_DEADZONE_BAND           10U

// GPIO pin for the mode select button
#define HANDBRAKE_MODE_SELECT_BUTTON      12

// Handbrake Modes
#define HANDBRAKE_UNKNOWN_MODE            0x00
#define HANDBRAKE_KEYBOARD_MODE           0x01
#define HANDBRAKE_BUTTON_MODE             0x02
#define HANDBRAKE_ANALOG_MODE             0x03
#define HANDBRAKE_CALIBRATE_MODE          0x04
#define HANDBRAKE_CONFIG_MODE             0x05

#define HANDBRAKE_NUM_MODES               0x06


/*********************************** DEFINITIONS ************************************/

const char* k_mode_names[HANDBRAKE_NUM_MODES] = 
{
  "UNKNOWN", //< Mode 0 is undefined
  "KEYBOARD",
  "BUTTON",
  "ANALOG",
  "CALIBRATION",
  "CONFIG"
};

typedef struct
{
  uint32_t config_mode;
  uint32_t led_color;
  uint32_t blink_interval;
  uint8_t  duty_cycle;
}configMode_t;

/// @brief Definition of the revision number for checking to see if EEPROM data is compatable
/// Anytime the EEPROM data structure is changed, this needs to be updated
#define REVISION_NUM                      101

typedef struct
{
  uint32_t rev_number;       //< Revision number of the eeprom data struct
  uint32_t data_size;        //< Size of the data structure
  uint32_t cal_max;          //< Raw ADC value that represents 100%
  uint32_t cal_min;          //< Raw ADC value that represents 0%
  uint32_t button_threshold; //< Threshold (0%-100%) where the button is activated/deactivated
  uint16_t conf_key_code;    //< Configured key for the Keyboard Mode
  uint8_t  mode;             //< Current mode the device is in
  uint8_t  led_brightness;
  uint8_t  upper_deadband_percent;
  uint8_t  lower_deadband_percent;
} eepromData_t;

/******************************* FUNCTION DEFINITIONS *******************************/

static void handbrakeInitialConditions(void);
static void handbrakeProcessSerial(void);
static void handbrakeServiceModeButton(void);
static uint32_t handbrakeGetModeIdx(void);
static void handbrakeUpdateSettings(eepromData_t *p_data);
static void handbrakeLoadSettings(eepromData_t *p_data);
static bool handbrakeValidateSettings(const eepromData_t *p_data);
static uint32_t handbrakeProcessSerialLong(uint32_t upper_bound, uint32_t lower_bound, uint32_t origional_value);

/************************************* GLOBALS **************************************/

// Array of modes which can be cycled through by momentary press on the button
static configMode_t modes[] = 
{
  {HANDBRAKE_KEYBOARD_MODE, Color::GREEN,     0,  0}, 
  {HANDBRAKE_BUTTON_MODE,   Color::BLUE,      0,  0}, 
  {HANDBRAKE_ANALOG_MODE,   Color::RED,       0,  0}, 
  {HANDBRAKE_CONFIG_MODE,   Color::PURPLE, 1000, 90}
};

// Configure the number of buttons.  Be careful not
// to use a pin for both a digital button and analog
// axis.  The pullup resistor will interfere with
// the analog voltage.
const int numButtons = 8;  // 16 for Teensy, 32 for Teensy++

const char *gp_key_strings[] =
{ "A", "B", "C", "D", "E", "F", "G", "H", "I", "J",
  "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T",
  "U", "V", "W", "X", "Y", "Z", "1", "2", "3", "4",
  "5", "6", "7", "8", "9", "0", "ENTER", "ESC", "BACKSPACE",
  "TAB", "SPACE", "MINUS", "EQUAL", "LEFT_BRACE", "RIGHT_BRACE",
  "BACKSLASH", "NON_US_NUM", "SEMICOLON", "QUOTE", "TILDE", "COMMA",
  "PERIOD", "SLASH", "CAPS_LOCK", "F1", "F2", "F3", "F4", "F5",
  "F6", "F7", "F8", "F9", "F10", "F11", "F12", "PRINTSCREEN",
  "SCROLL_LOCK", "PAUSE", "INSERT", "HOME", "PAGE_UP", "DELETE",
  "END", "PAGE_DOWN", "RIGHT", "LEFT", "DOWN", "UP", "NUM_LOCK",
  "KEYPAD_SLASH", "KEYPAD_ASTERIX", "KEYPAD_MINUS", "KEYPAD_PLUS", "KEYPAD_ENTER", "KEYPAD_1",
  "KEYPAD_2", "KEYPAD_3", "KEYPAD_4", "KEYPAD_5", "KEYPAD_6", "KEYPAD_7", "KEYPAD_8", "KEYPAD_9",
  "KEYPAD_0", "KEYPAD_PERIOD"
};

const uint16_t g_key_codes[] =
{ KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G, KEY_H, KEY_I, KEY_J,
  KEY_K, KEY_L, KEY_M, KEY_N, KEY_O, KEY_P, KEY_Q, KEY_R, KEY_S, KEY_T,
  KEY_U, KEY_V, KEY_W, KEY_X, KEY_Y, KEY_Z, KEY_1, KEY_2, KEY_3, KEY_4,
  KEY_5, KEY_6, KEY_7, KEY_8, KEY_9, KEY_0, KEY_ENTER, KEY_ESC, KEY_BACKSPACE,
  KEY_TAB, KEY_SPACE, KEY_MINUS, KEY_EQUAL, KEY_LEFT_BRACE, KEY_RIGHT_BRACE,
  KEY_BACKSLASH, KEY_NON_US_NUM, KEY_SEMICOLON, KEY_QUOTE, KEY_TILDE, KEY_COMMA,
  KEY_PERIOD, KEY_SLASH, KEY_CAPS_LOCK, KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5,
  KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_F12, KEY_PRINTSCREEN,
  KEY_SCROLL_LOCK, KEY_PAUSE, KEY_INSERT, KEY_HOME, KEY_PAGE_UP, KEY_DELETE,
  KEY_END, KEY_PAGE_DOWN, KEY_RIGHT, KEY_LEFT, KEY_DOWN, KEY_UP, KEY_NUM_LOCK,
  KEYPAD_SLASH, KEYPAD_ASTERIX, KEYPAD_MINUS, KEYPAD_PLUS, KEYPAD_ENTER, KEYPAD_1,
  KEYPAD_2, KEYPAD_3, KEYPAD_4, KEYPAD_5, KEYPAD_6, KEYPAD_7, KEYPAD_8, KEYPAD_9,
  KEYPAD_0, KEYPAD_PERIOD
};

// Holds the last selected mode
static uint8_t g_last_mode = 0U;

/// The button press threshold
static float g_button_thresh = 50.0F;

// The key bound to the threshold crossing
static uint32_t g_bound_key = KEYPAD_8;

// Serial buffer, for processing serial data
static uint8_t g_serial_buff[HANDBRAKE_SERIAL_BUFF_SIZE];

static uint16_t g_cal_data_min = UINT16_MAX;
static uint16_t g_cal_data_max = 0U;

// initialize a common cathode LED
RGBTools rgb(RGB_R_PIN,RGB_G_PIN,RGB_B_PIN);

static eepromData_t g_saved_data = 
{
  .rev_number = 0U,
  .data_size = 0U,
  .cal_max = 0U,
  .cal_min = 0U,
  .button_threshold = 0U,
  .conf_key_code = 0U,
  .mode = 0U,
};

// ********** INPUT CONFIG ************/
// Mode switch button input pin
static const uint32_t config_button_pin = 6U;

static const uint32_t hall_analog_pin = 0U;

/**************************************  CODE ****************************************/

/**
 * @brief Initializes the Handbrake controller. Standard Arduino setup function.
 */
void setup(void) {
  // Set LED color
  rgb.setColor(Color::OFF);

  // Setup config pins
  pinMode(config_button_pin, INPUT_PULLUP);
   
  // you can print to the serial monitor while the joystick is active!
  Serial.begin(9600);

  // Set serial parsing timeout
  Serial.setTimeout(HANDBRAKE_SERIAL_TIMEOUT);

  // configure the joystick to manual send mode.  This gives precise
  // control over when the computer receives updates, but it does
  // require you to manually call Joystick.send_now().
  Joystick.useManualSend(true);

  /** @todo: Set pin mode for each button needed, not in bulk.
    * for (int i = 0; i < numButtons; i++) {
    *   pinMode(i, INPUT_PULLUP);
    * }
    */

  // Set initial joystick conditions
  handbrakeInitialConditions();


  #if HANDBRAKE_DEBUG
  delay(2500);
  Serial.println("\f*** USB Handbrake Initialized! ***");
  #endif

  // Load EEPROM data into global structure 
  handbrakeLoadSettings(&g_saved_data);
  bool settings_result = handbrakeValidateSettings(&g_saved_data);

  // Default some settings if the settings are not valid
  if(!settings_result)
  {
    // Default the deadbands
    g_saved_data.upper_deadband_percent = HANDBRAKE_UPPER_DEADZONE_BAND;
    g_saved_data.lower_deadband_percent = HANDBRAKE_LOWER_DEADZONE_BAND;
  }

  // If mode is undefined, load analog mode
  if ((g_saved_data.mode == 0U) || 
      (g_saved_data.mode > (HANDBRAKE_NUM_MODES - 1U)))
  {
    g_saved_data.mode = HANDBRAKE_ANALOG_MODE;
  }

  // Keep LED from being completely off
  if(g_saved_data.led_brightness == 0U)
  {
    g_saved_data.led_brightness = HANDBRAKE_LED_DEFAULT_BRIGHTNESS;
  }

  // Update last mode to the current mode 
  g_last_mode = g_saved_data.mode;

  // if eeprom doesn't contain valid data, write it now
  if(!settings_result)
  {
    handbrakeUpdateSettings(&g_saved_data);
  }

  // Set the LED color for the current mode
  uint32_t mode_idx = handbrakeGetModeIdx();
  rgb.setColor(modes[mode_idx].led_color, g_saved_data.led_brightness);
}

/**
 * @brief Main loop for the Handbrake controller. Standard Arduino loop function.
 */
void loop(void) {

  // Holds the mode for the last loop cycle
  static uint8_t previous_mode = 0U;

  // Always Read hall sensor
  uint16_t data = analogRead(hall_analog_pin);

  // Normal operating modes
  if ((g_saved_data.mode == HANDBRAKE_KEYBOARD_MODE) ||
      (g_saved_data.mode == HANDBRAKE_ANALOG_MODE) ||
      (g_saved_data.mode == HANDBRAKE_BUTTON_MODE))
  {
    //Apply calibration transform to the data
    float divisior = (g_saved_data.cal_max -  g_saved_data.cal_min);
    float position = HANDBRAKE_POSITION_MIN;

    // Calculate position (0.0F - 100.0F)
    // Avoid div by 0
    if(divisior > HANDBRAKE_POSITION_MIN)
    {
      // limit data from going below the calibrated min value
      // but don't modify the incoming data
      uint16_t adjusted_data = max(data, g_saved_data.cal_min);
      
      // Calculate position
      position = ((float)(adjusted_data - g_saved_data.cal_min) / divisior) * HANDBRAKE_POSITION_MAX;
    }

    // Limit position 
    if(position < HANDBRAKE_POSITION_MIN)
    {
      position = HANDBRAKE_POSITION_MIN;
    }
    else if (position > HANDBRAKE_POSITION_MAX)
    {
      position = HANDBRAKE_POSITION_MAX;
    }
    else
    {
      // Don't modify the value
    }

    // Report calculated position
    #if HANDBRAKE_DEBUG_VERBOSE
    Serial.print("data: ");
    Serial.print(data);
    Serial.print(" | position: ");
    Serial.println(position);
    #endif

    if (g_saved_data.mode == HANDBRAKE_ANALOG_MODE)
    {
      // set the Z axis position to data
      Joystick.Z((position / HANDBRAKE_POSITION_MAX) * HANDBRAKE_JOY_AXIS_MAX);
      Joystick.send_now();
    }
    else if (g_saved_data.mode == HANDBRAKE_BUTTON_MODE)
    {
      // Set the button if above the threshold
      if (position >= g_button_thresh)
      {
        Joystick.button(HANDBRAKE_JOY_BUTTON, true);
      }
      // Release the button if below the threshold with the hysteresis or if
      // position happens to become 0
      else if((position < (g_button_thresh - HANDBRAKE_THRESH_HYSTERESIS)) || (position == 0.0f))
      {
        Joystick.button(HANDBRAKE_JOY_BUTTON, false);
      }
      else
      {
        // Do nothing
      }
      Joystick.send_now();
    }
    else if (g_saved_data.mode == HANDBRAKE_KEYBOARD_MODE)
    {
      // Set keypress if above the threshold
      if (position >= g_button_thresh)
      {
        Keyboard.set_key1(g_bound_key);
      }
      // Release the button if below the threshold with the hysteresis or if
      // position happens to become 0
      else if((position < (g_button_thresh - HANDBRAKE_THRESH_HYSTERESIS)) || (position == 0.0f))
      {
        Keyboard.set_key1(0);
      }
      else
      {
        // Do nothing
      }
    }
  }
  else if(g_saved_data.mode == HANDBRAKE_CONFIG_MODE)
  {
    // Do Configuration
    // Process serial commands
    if (Serial.available() > 0)
    {
      handbrakeProcessSerial();
    }
  }
  else if(g_saved_data.mode == HANDBRAKE_CALIBRATE_MODE)
  {
    // Do calibration
    // Find limits for calibration
    g_cal_data_min = min(data, g_cal_data_min);
    g_cal_data_max = max(data, g_cal_data_max);

  }
  else
  {
    #if HANDBRAKE_DEBUG
    Serial.println("Mode changed to: UNKNOWN");
    #endif
    assert(false);
  }

  // Service the config button and mode changes
  handbrakeServiceModeButton();

  // Process events upon state changes
  // Holds the mode for the last loop cycles from mode changes
  if(previous_mode != g_saved_data.mode)
  {
    // Store the previously selected mode into last mode, so we can return if needed
    g_last_mode = previous_mode;

    // Print out the mode change info
    #if HANDBRAKE_DEBUG
      // Holds the mode for the last loop cycleG
    assert((previous_mode < HANDBRAKE_NUM_MODES) && (g_saved_data.mode < HANDBRAKE_NUM_MODES));
    Serial.print("Mode changed: ");
      // Holds the mode for the last loop cycle
    Serial.print(k_mode_names[previous_mode]);
    Serial.print(" -> ");
    Serial.println(k_mode_names[g_saved_data.mode]);
    #endif

    // If current mode is normal operating mode, save to eeprom
    if((g_saved_data.mode == HANDBRAKE_KEYBOARD_MODE) ||
       (g_saved_data.mode == HANDBRAKE_BUTTON_MODE)   ||
       (g_saved_data.mode == HANDBRAKE_ANALOG_MODE))
       {
        handbrakeUpdateSettings(&g_saved_data);
       }

    // HANDLE EVENTS FOR ENTERING MODE
    if(g_saved_data.mode == HANDBRAKE_CONFIG_MODE)
    {
      printConfigMenu();
    }
    else if(g_saved_data.mode == HANDBRAKE_CALIBRATE_MODE)
    {
      // Change LED for Calibrate mode special case
      rgb.setColor(Color::WHITE);
      rgb.blinkEnable(500, 50);

      // Reset min and max
      g_cal_data_min = UINT16_MAX;
      g_cal_data_max = 0U;
    }
    else
    {
      // Do nothing...
    }
    
    // HANDLE EVENT FOR LEAVING MODE
    // Holds the mode for the last loop cycle
    if(previous_mode == HANDBRAKE_CALIBRATE_MODE)
    {
      // Calculate the deadband
      uint32_t upper_deadband = (uint32_t)((float)(g_cal_data_max - g_cal_data_min) * 
      ((float)g_saved_data.upper_deadband_percent / 100.0f));
      uint32_t lower_deadband = (uint32_t)((float)(g_cal_data_max - g_cal_data_min) * 
      ((float)g_saved_data.lower_deadband_percent / 100.0f));

      // Calibration mode has been exited, store new calibration values
      // narrowed by the deadband value
      g_saved_data.cal_max = g_cal_data_max - upper_deadband;
      g_saved_data.cal_min = g_cal_data_min + lower_deadband;

      #if HANDBRAKE_DEBUG
      Serial.print("Raw Calibration (no deadband) (min,max): ");
      Serial.print(g_cal_data_min);
      Serial.print(",");
      Serial.println(g_cal_data_max);
      Serial.print("New Calibration (min,max): ");
      Serial.print(g_saved_data.cal_min);
      Serial.print(",");
      Serial.println(g_saved_data.cal_max);
      #endif

      // Save the data to eeprom
      handbrakeUpdateSettings(&g_saved_data);
    }

    // Reset controller on mode change
    handbrakeInitialConditions();
  }

  // Service the LED
  rgb.serviceLED();

  // Set previous mode 
  // Holds the mode for the last loop cycleus mode to current mode
  previous_mode = g_saved_data.mode;

  // a brief delay, so this runs "only" 200 times per second
  delay(5);
}

/**
 * @brief Processes and handles data coming in from the Serial interface. This is for advanced
 * configuration functions and debugging.
 */
static void handbrakeProcessSerial(void)
{
  // Process Recognized commands
  uint8_t byte = Serial.read();
  if (byte == '1')
  {

  }
  else if (byte == '2')
  {
    // @todo: When developed, the button threshold should be a 0-100% value of the calibrated range
  }
  else if (byte == '3')
  {
    Serial.print("Enter a new brightness value (1 to 255): ");
    g_saved_data.led_brightness = handbrakeProcessSerialLong(255U, 1U, g_saved_data.led_brightness);
    Serial.printf("\r\nValue set to: %d\r\n", g_saved_data.led_brightness);
  }
    else if (byte == '4')
  {
    Serial.print("Enter a new upper deadband value (0 to 100) (Default is ");
    Serial.printf("%d", HANDBRAKE_UPPER_DEADZONE_BAND);
    Serial.print(": ");
    g_saved_data.upper_deadband_percent = handbrakeProcessSerialLong(100, 0U, g_saved_data.upper_deadband_percent);
    Serial.printf("\r\nValue set to: %d\r\n", g_saved_data.upper_deadband_percent);
  }
    else if (byte == '5')
  {
    Serial.print("Enter a new lower deadband value (0 to 100) (Default is ");
    Serial.printf("%d", HANDBRAKE_LOWER_DEADZONE_BAND);
    Serial.print(": ");
    g_saved_data.lower_deadband_percent = handbrakeProcessSerialLong(100, 0U, g_saved_data.lower_deadband_percent);
    Serial.printf("\r\nValue set to: %d\r\n", g_saved_data.lower_deadband_percent);
  }
  else
  {
    printConfigMenu();
  }
}


static uint32_t handbrakeProcessSerialLong(uint32_t upper_bound, uint32_t lower_bound, 
uint32_t origional_value)
{
  uint32_t val = origional_value;
  uint32_t num_bytes = Serial.readBytesUntil('\r', &g_serial_buff[0U], 5U);
   if (num_bytes > 0)
   {
     val = strtoul((const char *)&g_serial_buff[0U], NULL, 10);
     // Keep the value bounded
     val = (val > upper_bound) ? upper_bound : val;
     val = (val < lower_bound) ? lower_bound : val;
   }
   else
   {
     Serial.print("Timed Out.");
   }
   return val;
}

/**
 * @brief Handles the button presses. Cycles modes when pressed and released, enters cal mode
 * when button is held down for a time greater than the specified threshold.
 */
static void handbrakeServiceModeButton(void)
{
  static bool button_prev_state = false;

  bool button_curr_state = (digitalRead(config_button_pin) == LOW);

  bool button_held = false;
  static unsigned long hold_time_start = 0U;
  if(button_curr_state)
  {
    if((millis() - hold_time_start) > HANDBRAKE_BUTTON_HOLD_TIME_MS)
    {
      // Change mode to Calibrate Mode
      g_saved_data.mode = HANDBRAKE_CALIBRATE_MODE;

      // Set button held, to not change modes below
      button_held = true;

      button_curr_state = false;
    }
  }
  else
  {
    hold_time_start = millis();
  }

  // Detect a change in state only on release
  if((button_curr_state ^ button_prev_state) && !button_curr_state && !button_held)
  {    
    // Find index of current mode
    uint32_t mode_idx = handbrakeGetModeIdx();

    // Advance the mode idx to the next valid mode, 
    // if current mode is not calibration
    if(g_saved_data.mode != HANDBRAKE_CALIBRATE_MODE)
    {
      if(++mode_idx >= COUNT_OF(modes))
      {
          mode_idx = 0U;
      }
    }
    // Set the current mode and the LED
    g_saved_data.mode = modes[mode_idx].config_mode;
    rgb.setColor(modes[mode_idx].led_color, g_saved_data.led_brightness);
    rgb.blinkEnable(modes[mode_idx].blink_interval, modes[mode_idx].duty_cycle);
  }

  // Store current button state
  button_prev_state = button_curr_state;
}

/**
 * @brief Writes the delta of the settings to EEPROM
 * @param p_data Pointer to the struct holding the data to be written to EEPROM
 */
static void handbrakeUpdateSettings(eepromData_t *p_data)
{
  // Populate the Rev Number and data size before saving
  p_data->rev_number = REVISION_NUM;
  p_data->data_size  = sizeof(eepromData_t);

  // Now save the data
  uint32_t num_bytes = EEPROM.updateBlock(HANDBRAKE_EEPROM_ADDR, *p_data);

  #if HANDBRAKE_DEBUG
  Serial.print("Save Settings: Bytes: ");
  Serial.println(num_bytes);
  #endif
}

/**
 * @brief Reads the settings from the EEPROM
 * @param p_data Pointer to the struct which will have EEPROM data written to it
 */
static void handbrakeLoadSettings(eepromData_t *p_data)
{
  uint32_t num_bytes = EEPROM.readBlock(HANDBRAKE_EEPROM_ADDR, *p_data);
  assert(num_bytes == sizeof(*p_data));

  #if HANDBRAKE_DEBUG
  Serial.print("Load Settings: Bytes: ");
  Serial.println(num_bytes);
  #endif
}

/**
 * @brief Validates the settings stored in the EEPROM
 * @param p_data Pointer to the struct holding the data recently read from EEPROM
 * @return Data is valid (true) or invalid (false)
 */
static bool handbrakeValidateSettings(const eepromData_t *p_data)
{
  bool result = 
    ((p_data->rev_number == REVISION_NUM) &&
    (p_data->data_size == sizeof(eepromData_t)));
  
  #if HANDBRAKE_DEBUG
  Serial.print("Validate Settings: ");
  Serial.println((result ? "VALID" : "INVALID"));
  #endif

  return result;
}

static void handbrakeInitialConditions(void)
{
  Joystick.X(JOYSTICK_RESTING_POS);
  Joystick.Y(JOYSTICK_RESTING_POS);
  Joystick.Zrotate(JOYSTICK_RESTING_POS);
  Joystick.sliderLeft(JOYSTICK_RESTING_POS);
  Joystick.sliderRight(JOYSTICK_RESTING_POS);
  Joystick.hat(-1);

  // Reset axis, button and keyboard being used
  Joystick.Z(0U);
  Joystick.button(HANDBRAKE_JOY_BUTTON, false);
  Keyboard.set_key1(0);

  Joystick.send_now();
}


static uint32_t handbrakeGetModeIdx(void)
{
    // Determine mode to find index of
    // If calibration is the active mode, find the index of the previous mode
    uint32_t mode =
      // Holds the mode for the last loop cycle
     (g_saved_data.mode == HANDBRAKE_CALIBRATE_MODE) ? g_last_mode : g_saved_data.mode; 

    // Find index of current mode
    uint32_t idx = 0U;
    while ( (idx < COUNT_OF(modes)) && (modes[idx].config_mode != mode))
    { 
        ++idx; 
    }
    // If idx is the number of elements, element wasn't found in the list.  
    if(idx == COUNT_OF(modes))
    {
      // Reset the index to 0
      idx = 0U;
    }

    return idx;
}
