#include "trigger.h"

using namespace std;

Trigger::Trigger() {
    this->stopButtonPressed = false;
    this->pauseButtonPressed = false;
    this->frameFinished = 3;
}

void Trigger::startSession()
{
    jetsonTX2GPIONumber gLed = gpio398 ;     // Ouput
    jetsonTX2GPIONumber pushButton = gpio397 ; // Input
    // Make the button and led available in user space
    gpioExport(pushButton) ;
    gpioExport(gLed) ;
    this->thread()->usleep(1000000);
    gpioSetDirection(pushButton,inputPin) ;
    gpioSetDirection(gLed,outputPin) ;

    unsigned int value = low;
    int ledValue = low ;
    unsigned int prevValue = low;
    // Turn off the LED
    gpioSetValue(gLed,low);

    while((!this->stopButtonPressed))
    {
        // Don't send a trigger signal unless the previous frame has finished and the pause button is not pressed.
        if ((frameFinished == 3) && !pauseButtonPressed)
        {
            this->thread()->msleep(25);
            //gpioGetValue(pushButton, &value);

            // Toggle trigger on and off
            if(prevValue == low) {
                value = high;
            } else {
                value = low;
            }

            if (value == high && prevValue == low) {
                ledValue = high;
                //gpioSetValue(gLed,on);
                this->frameFinished = 0;
                emit captureRequest();
                //cout << "Sent Capture Request" << endl;

            } else {
                ledValue = low ;
                //gpioSetValue(gLed,off);
            }
            prevValue = value;
        }
    }
    gpioUnexport(gLed);     // unexport the LED
    gpioUnexport(pushButton);      // unexport the push button
}

void Trigger::endSession() {
    emit finished();
}

void Trigger::restartSession() {
    this->stopButtonPressed = false;
    this->pauseButtonPressed = false;
    this->frameFinished = 3;

    cout << "About to restart Trigger" << endl;
    this->startSession();
}
