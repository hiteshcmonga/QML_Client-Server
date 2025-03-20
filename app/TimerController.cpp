#include "TimerController.h"

TimerController::TimerController(QObject* parent)
    : QObject(parent), elapsed(0)
{
    timer.setInterval(1000); // one-second interval
    connect(&timer, &QTimer::timeout, this, &TimerController::onTimeout);
}

void TimerController::start() {
    timer.start();
}

void TimerController::stop() {
    timer.stop();
}

void TimerController::onTimeout() {
    elapsed++;
    emit timeUpdated(elapsed);
}
