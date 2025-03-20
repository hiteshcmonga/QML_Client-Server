#ifndef APPLICATION_H
#define APPLICATION_H

#include <QObject>
#include <QProcess>
#include <QString>
#include <QQuickView>
#include <QQmlEngine>
#include <QQmlContext>
#include "TimerController.h"
#include "WebClient.h"

class Application : public QObject {
    Q_OBJECT
    Q_PROPERTY(int timerValue READ timerValue NOTIFY timerValueChanged)
    Q_PROPERTY(QString usbList READ usbList NOTIFY usbListChanged)
    Q_PROPERTY(QString buttonText READ buttonText NOTIFY buttonTextChanged)

public:
    explicit Application(QObject* parent = nullptr);
    ~Application();
    void start();

    // Property readers
    int timerValue() const { return m_timerValue; }
    QString usbList() const { return m_usbList; }
    QString buttonText() const { return m_buttonText; }

signals:
    void timerValueChanged();
    void usbListChanged();
    void buttonTextChanged();

public slots:
    void onStartStopClicked();
    void onListUSBClicked();
    void onExitClicked();
    void onCleanupClicked();

private slots:  // Add this section
    void onServerOutput();
    void onQMLFetched(const QString &qmlContent);

private:
    QQuickView* view;
    QProcess* serverProcess;
    int serverPort;
    TimerController* timerController;
    WebClient* webClient;
    bool timerRunning;
    int m_timerValue;
    QString m_usbList;
    QString m_buttonText;

    void launchServer();
    void shutdownServer(bool cleanup = false);
};

#endif // APPLICATION_H