#include "trigger.h"

using namespace std;

Trigger::Trigger(QObject *parent) : QThread(parent) {}

void Trigger::run()
{
    cout << "Starting GPIO Trigger" << endl;
    msleep(3000);
    initGPIO();
}

int getkey() {
    int character;
    struct termios orig_term_attr;
    struct termios new_term_attr;

    /* set the terminal to raw mode */
    tcgetattr(fileno(stdin), &orig_term_attr);
    memcpy(&new_term_attr, &orig_term_attr, sizeof(struct termios));
    new_term_attr.c_lflag &= ~(ECHO|ICANON);
    new_term_attr.c_cc[VTIME] = 0;
    new_term_attr.c_cc[VMIN] = 0;
    tcsetattr(fileno(stdin), TCSANOW, &new_term_attr);

    /* read a character from the stdin stream without blocking */
    /*   returns EOF (-1) if no character is available */
    character = fgetc(stdin);

    /* restore the original terminal attributes */
    tcsetattr(fileno(stdin), TCSANOW, &orig_term_attr);

    return character;
}

bool Trigger::initGPIO()
{
    jetsonTX2GPIONumber redLED = gpio398 ;     // Ouput
    jetsonTX2GPIONumber pushButton = gpio397 ; // Input
    // Make the button and led available in user space
    gpioExport(pushButton) ;
    gpioExport(redLED) ;
    usleep(1000000);
    gpioSetDirection(pushButton,inputPin) ;
    gpioSetDirection(redLED,outputPin) ;

    // Flash the LED 3 times
//    for(int i=0; i<3; i++){
//        //cout << "Setting the LED on" << endl;
//        gpioSetValue(redLED, on);
//        usleep(1000000);         // on for 200ms
//        //cout << "Setting the LED off" << endl;
//        gpioSetValue(redLED, off);
//        usleep(1000000);         // off for 200ms
//    }

    unsigned int value = low;
    int ledValue = low ;
    // Turn off the LED
    gpioSetValue(redLED,low) ;
    while((!this->stopButtonPressed)) {
        gpioGetValue(pushButton, &value);
        // Useful for debugging
        // Only trigger a request when we have received the signal that the current capture and display has finished
        if (value==high && frameFinished) {
            cout << "Button " << value << endl;
            ledValue = high ;
            gpioSetValue(redLED,on);
            emit captureRequest(true);
            this->frameFinished = false;
        } else {
            cout << "Button " << value << endl;
            if (ledValue != low) {
                ledValue = low ;
                gpioSetValue(redLED,off) ;
            }

        }
        usleep(1000); // sleep for a millisecond
    }
    gpioUnexport(redLED);     // unexport the LED
    gpioUnexport(pushButton);      // unexport the push button
    return 0;
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






