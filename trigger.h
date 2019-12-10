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
    bool pauseButtonPressed = false;
    // Initialized to true before the first capture
    int frameFinished = 3;

public:
    explicit Trigger(QObject *parent = 0);
    void initGPIO();

protected:
    void run();

signals:
    void captureRequest();

public slots:
    void stopRequest();
    void pauseRequest(bool);
    void captureComplete();
};

#endif // TRIGGER

