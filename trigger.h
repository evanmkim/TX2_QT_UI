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

class Trigger: public QThread
{
    Q_OBJECT

private:
    bool stopButtonPressed = false;
    // Initialized to true before the first capture
    bool frameFinished = true;

public:
    explicit Trigger(QObject *parent = 0);

    bool initGPIO();

    int getKey();

protected:
    void run();

signals:
    void captureRequest(bool);

public slots:
    void stopRequest(bool);
    void captureComplete(bool);
};

#endif // TRIGGER

