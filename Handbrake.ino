/* Complete USB Joystick Example
   Teensy becomes a USB joystick with 16 or 32 buttons and 6 axis input

   You must select Joystick from the "Tools > USB Type" menu

   Pushbuttons should be connected between the digital pins and ground.
   Potentiometers should be connected to analog inputs 0 to 5.

   This example code is in the public domain.
*/

/************************************* INCLUDES *************************************/

#include "RGBTools.h"

/************************************** MACROS **************************************/
#define COUNT_OF(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

// Turns Debug on and off
#define HANDBRAKE_DEBUG                   1

#define HANDBRAKE_SERIAL_BUFF_SIZE        64U

#define HANDBRAKE_ADC_MAX                 1023U

// Serial Config Parsing Timeout
#define HANDBRAKE_SERIAL_TIMEOUT          5000

// Resting position
#define JOYSTICK_RESTING_POS              512U

#define HANDBRAKE_MODE_SELECT_BUTTON      12

#define HANDBRAKE_KEYBOARD_MODE           0x01
#define HANDBRAKE_BUTTON_MODE             0x02
#define HANDBRAKE_ANALOG_MODE             0x04
#define HANDBRAKE_CALIBRATE_MODE          0x08
#define HANDBRAKE_CONFIG_MODE             0x16

/******************************* FUNCTION DEFINITIONS *******************************/

static void handbrakeInitialConditions(void);
static void handbrakeProcessSerial(void);
static void handbrakePrintConfigMenu(void);

/************************************* GLOBALS **************************************/

// Configure the number of buttons.  Be careful not
// to use a pin for both a digital button and analog
// axis.  The pullup resistor will interfere with
// the analog voltage.
const int numButtons = 8;  // 16 for Teensy, 32 for Teensy++

const char *gp_key_strings[] =
{ "KEY_A", "KEY_B", "KEY_C", "KEY_D", "KEY_E", "KEY_F", "KEY_G", "KEY_H", "KEY_I", "KEY_J",
  "KEY_K", "KEY_L", "KEY_M", "KEY_N", "KEY_O", "KEY_P", "KEY_Q", "KEY_R", "KEY_S", "KEY_T",
  "KEY_U", "KEY_V", "KEY_W", "KEY_X", "KEY_Y", "KEY_Z", "KEY_1", "KEY_2", "KEY_3", "KEY_4",
  "KEY_5", "KEY_6", "KEY_7", "KEY_8", "KEY_9", "KEY_0", "KEY_ENTER", "KEY_ESC", "KEY_BACKSPACE",
  "KEY_TAB", "KEY_SPACE", "KEY_MINUS", "KEY_EQUAL", "KEY_LEFT_BRACE", "KEY_RIGHT_BRACE",
  "KEY_BACKSLASH", "KEY_NON_US_NUM", "KEY_SEMICOLON", "KEY_QUOTE", "KEY_TILDE", "KEY_COMMA",
  "KEY_PERIOD", "KEY_SLASH", "KEY_CAPS_LOCK", "KEY_F1", "KEY_F2", "KEY_F3", "KEY_F4", "KEY_F5",
  "KEY_F6", "KEY_F7", "KEY_F8", "KEY_F9", "KEY_F10", "KEY_F11", "KEY_F12", "KEY_PRINTSCREEN",
  "KEY_SCROLL_LOCK", "KEY_PAUSE", "KEY_INSERT", "KEY_HOME", "KEY_PAGE_UP", "KEY_DELETE",
  "KEY_END", "KEY_PAGE_DOWN", "KEY_RIGHT", "KEY_LEFT", "KEY_DOWN", "KEY_UP", "KEY_NUM_LOCK",
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


/// The current moode, defaults to undefined
static uint8_t g_current_mode = 0U;

/// The button press threshold
static uint16_t g_button_thresh = 1337U;

// The key bound to the threshold crossing
static uint32_t g_bound_key = KEYPAD_8;

// Serial buffer, for processing serial data
static uint8_t g_serial_buff[HANDBRAKE_SERIAL_BUFF_SIZE];

// initialize a common cathode LED
RGBTools rgb(3,4,5);

/**************************************  CODE ****************************************/


void setup() {
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

  // Set initial conditions
  handbrakeInitialConditions();


#ifdef HANDBRAKE_DEBUG
  delay(1000);
  Serial.println("Handbrake Initialized.");
#endif

  // @todo Load stored Key Binding from eeprom

  // @todo Load button threshold from eeprom

  // @todo Load the mode from eeprom

  // If mode is undefined, load analog mode
  if (g_current_mode == 0U)
  {
    g_current_mode = HANDBRAKE_ANALOG_MODE;
  }

  // Set LED color
  rgb.setColor(Color::WHITE);
  rgb.blinkEnable(500, 80);
}


void loop() {

  if (g_current_mode != HANDBRAKE_CONFIG_MODE)
  {
    // Read hall sensor
    uint16_t data = analogRead(1);

    if (g_current_mode == HANDBRAKE_ANALOG_MODE)
    {
      // set the Z axis position to data
      Joystick.Z(data);
      Joystick.send_now();
    }
    else if (g_current_mode == HANDBRAKE_BUTTON_MODE)
    {
      if (data >= g_button_thresh)
      {
        Joystick.button(1, true);
      }
      else
      {
        Joystick.button(1, false);
      }
      Joystick.send_now();
    }
    else if (g_current_mode == HANDBRAKE_KEYBOARD_MODE)
    {
      if (data >= g_button_thresh)
      {
        Keyboard.set_key1(g_bound_key);
      }
      else
      {
        Keyboard.set_key1(0);
      }
    }
  }
  else
  {
    // Do Configuration
  }

  // Process serial commands
  if (Serial.available() > 0)
  {
    handbrakeProcessSerial();
  }

  // Service the LED
  rgb.serviceLED();

  // a brief delay, so this runs "only" 200 times per second
  delay(5);
}

static void handbrakeProcessSerial(void)
{
  // Process Recognized commands
  uint8_t byte = Serial.read();
  if ((byte == 's') || (byte == 'S'))
  {
    Serial.print("Enter a new sensor test value (0 to 1023): ");
    uint32_t num_bytes = Serial.readBytesUntil('\r', &g_serial_buff[0U], 5U);
    if (num_bytes > 0)
    {
      uint32_t val = strtoul((const char *)&g_serial_buff[0U], NULL, 10);
      val = (val > HANDBRAKE_ADC_MAX) ? HANDBRAKE_ADC_MAX : val;

      Serial.printf("\r\nValue set to: %d\r\n", val);
    }
    else
    {
      Serial.println("Timed Out.");
    }
  }
  else if ((byte == 'k') || (byte == 'K'))
  {

  }
  else if ((byte == 'b') || (byte == 'B'))
  {

  }
  else
  {
    handbrakePrintConfigMenu();
  }
}

static void handbrakePrintConfigMenu(void)
{
  Serial.println("\f       Handbrake Config      ");
  Serial.println("=============================");
  Serial.println(" K - Set Key Binding         ");
  Serial.println(" B - Set Button Threshold    ");
  Serial.println(" S - Set Analog Value (Test) ");
}

static void handbrakeInitialConditions(void)
{
  Joystick.X(JOYSTICK_RESTING_POS);
  Joystick.Y(JOYSTICK_RESTING_POS);
  Joystick.Z(JOYSTICK_RESTING_POS);
  Joystick.Zrotate(JOYSTICK_RESTING_POS);
  Joystick.sliderLeft(JOYSTICK_RESTING_POS);
  Joystick.sliderRight(JOYSTICK_RESTING_POS);
  Joystick.hat(-1);
}

