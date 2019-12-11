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
#include <termios.h>

class Trigger: public QObject
{
    Q_OBJECT

public:

    Trigger();
    int frameFinished;
    bool stopButtonPressed;
    bool pauseButtonPressed;

protected:
    void run();

signals:
    void captureRequest();
    void finished();

public slots:
    void startSession();
};

#endif // TRIGGER

