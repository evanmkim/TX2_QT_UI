#include "trigger.h"

using namespace std;

Trigger::Trigger(QObject *parent) : QThread(parent) {}

void Trigger::run()
{
    cout << "Starting GPIO Trigger" << endl;
    msleep(3000);
    initGPIO();
}

void Trigger::initGPIO()
{
    jetsonTX2GPIONumber gLed = gpio398 ;     // Ouput
    jetsonTX2GPIONumber pushButton = gpio397 ; // Input
    // Make the button and led available in user space
    gpioExport(pushButton) ;
    gpioExport(gLed) ;
    usleep(1000000);
    gpioSetDirection(pushButton,inputPin) ;
    gpioSetDirection(gLed,outputPin) ;

    unsigned int value = low;
    int ledValue = low ;
    unsigned int prevValue = low;
    // Turn off the LED
    gpioSetValue(gLed,low);

    while((!this->stopButtonPressed))
    {
        if (frameFinished)
        {
            //gpioGetValue(pushButton, &value);
            if(prevValue == low) {
                value = high;
            } else {
                value = low;
            }

            if (value == high && prevValue == low) {
                ledValue = high;
                //gpioSetValue(gLed,on);
                this->frameFinished = false;
                emit captureRequest(true);
            } else {
                ledValue = low ;
                //gpioSetValue(gLed,off);
            }
            prevValue = value;
        }
    }

    gpioUnexport(gLed);     // unexport the LED
    gpioUnexport(pushButton);      // unexport the push button

    this->exit();
}

void Trigger::stopRequest(bool checked)
{
    this->stopButtonPressed = true;
    cout<< "Stop Button Pressed" << endl;
}

void Trigger::captureComplete(bool checked)
{
    this->frameFinished = true;
}






