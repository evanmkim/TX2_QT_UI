#ifndef TRIGGERMODE_H
#define TRIGGERMODE_H


#include <QTimer>
#include <QString>
#include <QThread>
#include "jetsonGPIO.h"


class triggerMode : public QThread
{
    Q_OBJECT

public:
    triggerMode(QObject *parent = 0);

protected:
    void run();
    bool initCAM();
    void testfunction();

private:
    jetsonTX1GPIONumber ButtonSigPin = gpio184;
    bool stopButtonPressed = false;
    QTimer *timer;

public slots:
    void prepareStop(bool stopButton);


};

#endif // TRIGGERMODE_H
