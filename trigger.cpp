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
        // Don't send a trigger signal unless the previous frame has finished and the pause button is not pressed.
        if ((frameFinished == 3) && !pauseButtonPressed)
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
                this->frameFinished = 0;
                emit captureRequest();
                cout << "Sent Capture Request" << endl;

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

void Trigger::stopRequest()
{
    this->stopButtonPressed = true;
    cout << "Stop Button Pressed" << endl;
}

void Trigger::pauseRequest(bool checked)
{
    this->pauseButtonPressed = checked;
    cout << "Pause Button Pressed" << endl;
}

void Trigger::captureComplete()
{
    this->frameFinished++;
}






