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

    #ifdef HANDBRAKE_DEBUG
    Serial.print("Mode changed to: UNKNOWN");
    #endif
    rgb.setColor(Color::RED);
    rgb.blinkEnable(250U, 50U);

    // Fail to here
    while(1)
    {
        rgb.serviceLED();
        delay(5);
    }

}