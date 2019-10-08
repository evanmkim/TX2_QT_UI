#ifndef JETSONGPIO_H_
#define JETSONGPIO_H_

 /****************************************************************
 * Constants
 ****************************************************************/

#define SYSFS_GPIO_DIR "/sys/class/gpio"
#define POLL_TIMEOUT (3 * 1000) /* 3 seconds */
#define MAX_BUF 64

typedef unsigned int jetsonGPIO ;
typedef unsigned int pinDirection ;
typedef unsigned int pinValue ;

enum pinDirections {
    inputPin  = 0,
    outputPin = 1
} ;

enum pinValues {
    low = 0,
    high = 1,
    off = 0,  // synonym for things like lights
    on = 1
}  ;

enum jetsonGPIONumber {
    gpio57  =  57,    // J3A1 - Pin 50
    gpio160 = 160,	  // J3A2 - Pin 40
    gpio161 = 161,    // J3A2 - Pin 43
    gpio162 = 162,    // J3A2 - Pin 46
    gpio163 = 163,    // J3A2 - Pin 49
    gpio164 = 164,    // J3A2 - Pin 52
    gpio165 = 165,    // J3A2 - Pin 55
    gpio166 = 166     // J3A2 - Pin 58
}  ;

enum jetsonTX1GPIONumber {
       gpio36 = 36,      // J21 - Pin 32 - Unused - AO_DMIC_IN_CLK
       gpio37 = 37,      // J21 - Pin 16 - Unused - AO_DMIC_IN_DAT
       gpio38 = 38,      // J21 - Pin 13 - Bidir  - GPIO20/AUD_INT
       gpio63 = 63,      // J21 - Pin 33 - Bidir  - GPIO11_AP_WAKE_BT
       gpio184 = 481,    // J21 - Pin 18 - Input  - GPIO16_MDM_WAKE_AP
       gpio186 = 186,    // J21 - Pin 31 - Input  - GPIO9_MOTION_INT
       gpio187 = 187,    // J21 - Pin 37 - Output - GPIO8_ALS_PROX_INT
       gpio219 = 219,    // J21 - Pin 29 - Output - GPIO19_AUD_RST
} ;

enum jetsonTX2GPIONumber {
       gpio397 = 397,      // J21 - Pin 13 - GPIO_GEN2
       gpio398 = 398,      // J21 - Pin 29 - GPIO5
       gpio298 = 298,      // J21 - Pin 31 - GPIO6
       gpio389 = 389,      // J21 - Pin 33 - GPIO13
       gpio395 = 395,      // J21 - Pin 35 - GPIO19
       gpio388 = 388,      // J21 - Pin 37 - GPIO26
       gpio297 = 297,      // J21 - Pin 32 - GPIO12
       gpio467 = 467,      // J21 - Pin 36 - GPIO16
       gpio394 = 394,      // J21 - Pin 38 - GPIO20
       gpio393 = 393,      // J21 - Pin 40 - GPIO21
} ;


int gpioExport ( jetsonGPIO gpio ) ;
int gpioUnexport ( jetsonGPIO gpio ) ;
int gpioSetDirection ( jetsonGPIO, pinDirection out_flag ) ;
int gpioSetValue ( jetsonGPIO gpio, pinValue value ) ;
int gpioGetValue ( jetsonGPIO gpio, unsigned int *value ) ;
int gpioSetEdge ( jetsonGPIO gpio, char *edge ) ;
int gpioOpen ( jetsonGPIO gpio ) ;
int gpioClose ( int fileDescriptor ) ;
int gpioActiveLow ( jetsonGPIO gpio, unsigned int value ) ;



#endif // JETSONGPIO_H_






//#ifndef JETSONGPIO_H_
//#define JETSONGPIO_H_

// /****************************************************************
// * Constants
// ****************************************************************/
 
//#define SYSFS_GPIO_DIR "/sys/class/gpio"
//#define POLL_TIMEOUT (3 * 1000) /* 3 seconds */
//#define MAX_BUF 64

//typedef unsigned int jetsonGPIO ;
//typedef unsigned int pinDirection ;
//typedef unsigned int pinValue ;

//enum pinDirections {
//	inputPin  = 0,
//	outputPin = 1
//} ;

//enum pinValues {
//    low = 0,
//    high = 1,
//    off = 0,  // synonym for things like lights
//    on = 1
//}  ;

//enum jetsonGPIONumber {
//    gpio57  =  57,    // J3A1 - Pin 50
//	gpio160 = 160,	  // J3A2 - Pin 40
//	gpio161 = 161,    // J3A2 - Pin 43
//	gpio162 = 162,    // J3A2 - Pin 46
//	gpio163 = 163,    // J3A2 - Pin 49
//	gpio164 = 164,    // J3A2 - Pin 52
//	gpio165 = 165,    // J3A2 - Pin 55
//	gpio166 = 166     // J3A2 - Pin 58
//}  ;

//enum jetsonTX1GPIONumber {
//       gpio36 = 36,      // J21 - Pin 32 - Unused - AO_DMIC_IN_CLK
//       gpio37 = 37,      // J21 - Pin 16 - Unused - AO_DMIC_IN_DAT
//       gpio38 = 38,      // J21 - Pin 13 - Bidir  - GPIO20/AUD_INT
//       gpio63 = 63,      // J21 - Pin 33 - Bidir  - GPIO11_AP_WAKE_BT
//       gpio184 = 481,    // J21 - Pin 18 - Input  - GPIO16_MDM_WAKE_AP
//       gpio186 = 186,    // J21 - Pin 31 - Input  - GPIO9_MOTION_INT
//       gpio187 = 187,    // J21 - Pin 37 - Output - GPIO8_ALS_PROX_INT
//       gpio219 = 219,    // J21 - Pin 29 - Output - GPIO19_AUD_RST
//} ;

//enum jetsonTX2GPIONumber {
//       gpio398 = 398,      // J21 - Pin 29 - GPIO5
//       gpio298 = 298,      // J21 - Pin 31 - GPIO6
//       gpio389 = 389,      // J21 - Pin 33 - GPIO13
//       gpio395 = 395,      // J21 - Pin 35 - GPIO19
//       gpio388 = 388,      // J21 - Pin 37 - GPIO26
//       gpio297 = 297,      // J21 - Pin 32 - GPIO12
//       gpio467 = 467,      // J21 - Pin 36 - GPIO16
//       gpio394 = 394,      // J21 - Pin 38 - GPIO20
//       gpio393 = 393,      // J21 - Pin 40 - GPIO21
//} ;


//int gpioExport ( jetsonGPIO gpio ) ;
//int gpioUnexport ( jetsonGPIO gpio ) ;
//int gpioSetDirection ( jetsonGPIO, pinDirection out_flag ) ;
//int gpioSetValue ( jetsonGPIO gpio, pinValue value ) ;
//int gpioGetValue ( jetsonGPIO gpio, unsigned int *value ) ;
//int gpioSetEdge ( jetsonGPIO gpio, char *edge ) ;
//int gpioOpen ( jetsonGPIO gpio ) ;
//int gpioClose ( int fileDescriptor ) ;
//int gpioActiveLow ( jetsonGPIO gpio, unsigned int value ) ;



//#endif // JETSONGPIO_H_
