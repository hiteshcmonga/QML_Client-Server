#include <QGuiApplication>
#include "Application.h"

int main(int argc, char *argv[]) {
    qputenv("QT_DEBUG_PLUGINS", "1");  // Keep debug plugins
    QGuiApplication app(argc, argv);

    qDebug() << "[DEBUG] Application starting...";
    Application application;  // Constructor handles all initialization
    return app.exec();
}