#include "Application.h"
#include "USBHelper.h"
#include <QDir>
#include <QDebug>
#include <QQmlContext>
#include <QTemporaryFile>
#include <QTimer>
#include <QUrl>
#include <QFile>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QQuickStyle>

Application::Application(QObject* parent)
    : QObject(parent),
      m_buttonText("Start"),
      view(new QQuickView()),
      serverProcess(new QProcess(this)),
      serverPort(0),
      timerController(new TimerController(this)),
      webClient(new WebClient(this)),
      timerRunning(false),
      m_timerValue(0),
      m_usbList("")
{
    QQuickStyle::setStyle("Basic");
    view->rootContext()->setContextProperty("app", this);

    // Keep existing import paths
    view->engine()->addImportPath(QCoreApplication::applicationDirPath());
    view->engine()->addImportPath("qrc:/qt/qml");

    connect(timerController, &TimerController::timeUpdated, this, [this](int value) {
        m_timerValue = value;
        emit timerValueChanged();
    });

    launchServer();
}

Application::~Application() {
    shutdownServer();
    delete view;
}

void Application::launchServer() {
    QDir exeDir(QCoreApplication::applicationDirPath());
    exeDir.cdUp();
    QString serverScript = exeDir.absoluteFilePath("server/server.py");
    serverScript = QDir::toNativeSeparators(serverScript);

    qDebug() << "[DEBUG] Launching server from:" << serverScript;

    serverProcess->setProcessChannelMode(QProcess::MergedChannels);
    serverProcess->start("python", QStringList() << serverScript);

    connect(serverProcess, &QProcess::readyReadStandardOutput, this, &Application::onServerOutput);
    connect(serverProcess, &QProcess::errorOccurred, [this](QProcess::ProcessError error) {
        qDebug() << "[ERROR] Server process error:" << error << serverProcess->errorString();
    });
}

void Application::onServerOutput() {
    QString output = QString::fromUtf8(serverProcess->readAllStandardOutput());
    qDebug() << "[SERVER LOG]" << output;

    if (output.contains("::SERVER_PORT::")) {
        QString portStr = output.split("::SERVER_PORT::").last().split("\n").first().trimmed();
        bool ok;
        serverPort = portStr.toInt(&ok);
        if (ok) {
            qDebug() << "[DEBUG] Server running on port:" << serverPort;
            QString url = QString("http://127.0.0.1:%1/api/v1/getQML").arg(serverPort);
            webClient->fetchQML(url, "main.qml", [this](const QString &qmlContent) {
                if (qmlContent.isEmpty()) {
                    qDebug() << "[ERROR] Received empty QML content";
                } else {
                    this->onQMLFetched(qmlContent);
                }
            });
        }
    }
}

void Application::onQMLFetched(const QString &qmlContent) {
    QTemporaryFile tempFile(QDir::tempPath() + "/XXXXXX.qml");
    if (tempFile.open()) {
        tempFile.write(qmlContent.toUtf8());
        tempFile.close();
        qDebug() << "[DEBUG] QML temp file:" << tempFile.fileName();

        // Load the QML file
        view->setSource(QUrl::fromLocalFile(tempFile.fileName()));

        // Enable dynamic resizing
        view->setResizeMode(QQuickView::SizeRootObjectToView);

        if (view->status() == QQuickView::Error) {
            qDebug() << "[QML ERRORS]";
            for (const auto &error : view->errors()) {
                qDebug() << error.toString();
            }
        } else {
            // NEW: Set an initial window size
            view->resize(640, 480);

            view->show();
            qDebug() << "[DEBUG] QML view shown";
        }
    }
}

void Application::start() {
    view->setResizeMode(QQuickView::SizeRootObjectToView);
}

void Application::onStartStopClicked() {
    if (!timerRunning) {
        timerController->start();
        timerRunning = true;
        m_buttonText = "Stop";
    } else {
        timerController->stop();
        timerRunning = false;
        m_buttonText = "Start";
    }
    emit buttonTextChanged();  // Changed from updateButtonText
    qDebug() << "Button text changed to:" << m_buttonText;
}

void Application::onListUSBClicked() {
    qDebug() << "Listing USB Devices...";

    // Clear previous data first
    m_usbList.clear();
    emit usbListChanged();
    // Get new data
    m_usbList = USBHelper::listDevices();
    qDebug() << "USB Devices Received:" << m_usbList;
    emit usbListChanged();
}

void Application::shutdownServer(bool cleanup) {
    if (serverPort > 0) {
        QString url = QString("http://127.0.0.1:%1/api/v1/").arg(serverPort);
        url += (cleanup ? "cleanup" : "shutdown");
        webClient->post(url, "{}");
    }
    if (serverProcess->state() == QProcess::Running) {
        serverProcess->terminate();
        serverProcess->waitForFinished(3000);
    }
}

void Application::onExitClicked() {
    shutdownServer();
    QCoreApplication::quit();
}

void Application::onCleanupClicked() {
    shutdownServer(true);
    QString serverScript = QDir::currentPath() + "/server/server.py";
    QFile::remove(serverScript);
    QString selfPath = QCoreApplication::applicationFilePath();
    QString cleanupCmd = QString("bash -c 'sleep 3 && rm -f \"%1\"'").arg(selfPath);
    qDebug() << "Scheduling self-deletion with command:" << cleanupCmd;
    system(cleanupCmd.toStdString().c_str());
    QCoreApplication::quit();
}