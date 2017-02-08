/************************************* INCLUDES *************************************/

#include "RGBTools.h"

#include "utilities.h"

/************************************** MACROS **************************************/

/*********************************** DEFINITIONS ************************************/
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