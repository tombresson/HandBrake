/************************************* INCLUDES *************************************/

#include "RGBTools.h"

#include "utilities.h"

/************************************** MACROS **************************************/

/*********************************** DEFINITIONS ************************************/

/**
 * @brief A function simply to display the configuration menu via the serial terminal
 */
void printConfigMenu(void)
{
  Serial.println("\f       Handbrake Config          ");
  Serial.println("================================== ");
  Serial.println(" 1 - Set Key Binding               ");
  Serial.println(" 2 - Set Button Threshold          ");
  Serial.println(" 3 - Set LED Brightness            ");
  Serial.println(" 4 - Set Upper (Pulled) Deadband   ");
  Serial.println(" 5 - Set Lower (Released) Deadband ");
  Serial.println(" D - Set ALL to DEFAULT ");
}

/**
 * @brief Function to read serial data, echo back the recieved bytes and time out if no byte are 
 recieved within the timeout period.
 * @param[out] p_buff Buffer for the serial data to be written
 * @param[in] len The total length of the buffer
 * @param[in] timeout The time, in milliseconds, that the function should wait before timing out
 * @return The number of bytes written. If the function timesout, a 0s will be returned regardless
 * of how many bytes were actually recieved or written to the buffer.
 */
uint32_t serialReadBytes(uint8_t *p_buff, uint32_t len, uint32_t timeout)
{
    unsigned long timeout_time = millis() + timeout;
    uint8_t idx = 0;
    bool exit_loop = false;
    bool timed_out = false;
    // Leave one index position for null character in string
    while ((idx < len - 1U) && !exit_loop && !timed_out) {
        if (Serial.available() > 0) 
        {
            // read the incoming byte:
            uint8_t byte = Serial.read();
            // If a new line or carage return is rx'd, exit_loop
            if ((byte == '\n') || (byte == '\r')) 
            {
                exit_loop = true;
            }
            else
            {
                // Store the byte
                p_buff[idx] = byte;

                // Reset the timeout
                timeout_time = millis() + timeout;
            }

            // Write the byte out to the terminal
            Serial.write(p_buff[idx++]);
        }

        // Check to see if we're timed out
        timed_out = ((int32_t)(millis() - timeout_time) >= 0);
    }

    // Only return the num of bytes if not timed out
    return (!timed_out ? idx : 0U);
}

///@brief Error handler
void error(void)
{
    // initialize a common cathode LED
    RGBTools rgb(RGB_R_PIN,RGB_G_PIN,RGB_B_PIN);
    rgb.setColor(Color::RED);
    rgb.blinkEnable(250U, 50U);

    // Fail to here
    while(1)
    {
        rgb.serviceLED();
        delay(5);
    }

}

// handle diagnostic informations given by assertion and abort program execution:
void __assert(const char *__func, const char *__file, int __lineno, const char *__sexp) {
    // transmit diagnostic informations through serial link. 
    Serial.println(__func);
    Serial.println(__file);
    Serial.println(__lineno, DEC);
    Serial.println(__sexp);
    Serial.flush();
    
    // abort program execution
    error();
}