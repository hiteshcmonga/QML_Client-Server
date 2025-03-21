#ifdef Q_OS_WIN
#include <windows.h>
#include <shellapi.h>  // For ShellExecuteW
#endif
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

    #ifndef Q_OS_WIN
        // On Linux/WSL, use "python3"
        serverProcess->start("python3", QStringList() << serverScript);
    #else
        // On Windows, use "python"
        serverProcess->start("python", QStringList() << serverScript);
    #endif

    connect(serverProcess, &QProcess::readyReadStandardOutput, this, &Application::onServerOutput);
    connect(serverProcess, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
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
            // Delay the request by 1 second (1000 ms)
            QTimer::singleShot(1000, this, [this]() {
                QString url = QString("http://127.0.0.1:%1/api/v1/getQML").arg(serverPort);
                webClient->fetchQML(url, "main.qml", [this](const QString &qmlContent) {
                    if (qmlContent.isEmpty()) {
                        qDebug() << "[ERROR] Received empty QML content";
                    } else {
                        this->onQMLFetched(qmlContent);
                    }
                });
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
    if (serverProcess->state() == QProcess::Running) {
        // Graceful termination
        serverProcess->terminate();
        if (!serverProcess->waitForFinished(3000)) {
            #ifdef Q_OS_WIN
                QProcess killTask;
                killTask.start("taskkill",
                    {"/F", "/T", "/PID", QString::number(serverProcess->processId())});
                killTask.waitForFinished();
            #else
                serverProcess->kill();
            #endif
        }
    }

    if (cleanup && serverPort > 0) {
        webClient->post(QString("http://127.0.0.1:%1/api/v1/cleanup").arg(serverPort), "{}");
    }
}

void Application::onExitClicked() {
    shutdownServer();
    qDebug() << "[DEBUG] Shutdown has been initiated.";
    // Delay the quit call by 2 seconds so the debug log is visible
    QTimer::singleShot(2000, QCoreApplication::instance(), &QCoreApplication::quit);
}

void Application::onCleanupClicked() {
    shutdownServer(true);  // Ensure server is terminated first

    QString selfPath = QCoreApplication::applicationFilePath();
    QString serverDir = QDir::cleanPath(QCoreApplication::applicationDirPath() + "/../server");

#ifdef Q_OS_WIN
    QString batchContent = QString(
        "@echo off\n"
        "echo Cleanup started at %%TIME%% >> \"%%TEMP%%\\cleanup.log\"\n"
        "timeout /t 5 >nul\n"  // increased wait time for process termination
        "del /F /Q \"%1\" >> \"%%TEMP%%\\cleanup.log\" 2>&1\n"
        "if exist \"%2\" rmdir /S /Q \"%2\" >> \"%%TEMP%%\\cleanup.log\" 2>&1\n"
        "del /F /Q \"%%~f0\" >> \"%%TEMP%%\\cleanup.log\" 2>&1\n"
        "echo Cleanup finished at %%TIME%% >> \"%%TEMP%%\\cleanup.log\"\n"
    ).arg(selfPath).arg(serverDir);

    QTemporaryFile tempFile(QDir::tempPath() + "/cleanup_XXXXXX.bat");
    if (tempFile.open()) {
        tempFile.write(batchContent.toUtf8());
        tempFile.close();

        // Convert path to native format and wrap in quotes
        QString batPath = QDir::toNativeSeparators(tempFile.fileName());
        QStringList args;
        args << "/C" << QString("\"%1\"").arg(batPath);

        bool detached = QProcess::startDetached("cmd.exe", args);
        if (!detached) {
            qDebug() << "[DEBUG] Failed to start cleanup batch process";
        }
    } else {
        qDebug() << "[DEBUG] Failed to create temporary cleanup batch file";
    }
#else
    QProcess::execute("sh", {"-c", QString("sleep 2; rm -f '%1'; rm -rf '%2'")
                             .arg(selfPath).arg(serverDir)});
#endif
    QCoreApplication::quit();
}