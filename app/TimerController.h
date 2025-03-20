#ifndef TIMERCONTROLLER_H
#define TIMERCONTROLLER_H

#include <QObject>
#include <QTimer>

class TimerController : public QObject {
    Q_OBJECT
public:
    explicit TimerController(QObject* parent = nullptr);
    void start();
    void stop();

signals:
    void timeUpdated(int value);

private slots:
    void onTimeout();

private:
    QTimer timer;
    int elapsed;
};

#endif // TIMERCONTROLLER_H
