#include "WebClient.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

class WebClient::Private {
public:
    QNetworkAccessManager manager;
};

WebClient::WebClient(QObject* parent)
    : QObject(parent), d(new Private)
{
}

void WebClient::fetchQML(const QString &url, const QString &filename, std::function<void(const QString&)> callback) {
    QUrl qurl(url);
    QNetworkRequest request(qurl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject json;
    json["filename"] = filename;
    QJsonDocument doc(json);
    QByteArray data = doc.toJson();

    QNetworkReply* reply = d->manager.post(request, data);
    QObject::connect(reply, &QNetworkReply::finished, this, [reply, callback]() {
        if (reply->error() == QNetworkReply::NoError) {
            QString content = QString::fromUtf8(reply->readAll());
            callback(content);
        } else {
            qDebug() << "Error fetching QML:" << reply->errorString();
            callback(QString());
        }
        reply->deleteLater();
    });
}

void WebClient::post(const QString &url, const QString &jsonData) {
    QUrl qurl(url);
    QNetworkRequest request(qurl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    d->manager.post(request, jsonData.toUtf8());
}