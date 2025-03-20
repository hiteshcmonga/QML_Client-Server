#ifndef WEBCLIENT_H
#define WEBCLIENT_H

#include <QObject>
#include <QString>
#include <functional>

class WebClient : public QObject {
    Q_OBJECT
public:
    explicit WebClient(QObject* parent = nullptr);
    void fetchQML(const QString &url, const QString &filename, std::function<void(const QString&)> callback);
    void post(const QString &url, const QString &jsonData);

private:
    class Private;
    Private* d;
};

#endif // WEBCLIENT_H
