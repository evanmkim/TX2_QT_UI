#ifndef TRIGGER
#define TRIGGER
#define QT_THREAD_SUPPORT

#include <QThread>
#include <QObject>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <iostream>
#include <memory>
#include <string>
#include "jetsonGPIO.h"
#include "sys/types.h"
#include "sys/sysinfo.h"
#include <sys/time.h>
#include <unistd.h> //for sleep
#include <chrono> //for time

class Trigger: public QThread
{
    Q_OBJECT

public:
    explicit Trigger(QObject *parent = 0);

    //Trigger() {}

    bool initGPIO();

protected:
    void run();
};

#endif // TRIGGER

