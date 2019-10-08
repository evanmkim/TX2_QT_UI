#include "trigger.h"

using namespace std;

Trigger::Trigger(QObject *parent) : QThread(parent) {}

void Trigger::run()
{
    cout << "Starting GPIO Trigger" << endl;
    msleep(3000);
    initGPIO();
}

bool Trigger::initGPIO()
{
    cout << "Testing the GPIO Pins" << endl;

    jetsonTX2GPIONumber redLED = gpio398 ;     // Ouput
    jetsonTX2GPIONumber pushButton = gpio397 ; // Input
    // Make the button and led available in user space
    gpioExport(pushButton) ;
    gpioExport(redLED) ;
    gpioSetDirection(pushButton,inputPin) ;
    gpioSetDirection(redLED,outputPin) ;
    // Reverse the button wiring; this is for when the button is wired
    // with a pull up resistor
    // gpioActiveLow(pushButton, true);


    // Flash the LED 5 times
    for(int i=0; i<5; i++){
        cout << "Setting the LED on" << endl;
        gpioSetValue(redLED, on);
        usleep(1000000);         // on for 200ms
        cout << "Setting the LED off" << endl;
        gpioSetValue(redLED, off);
        usleep(1000000);         // off for 200ms
    }
}






