#pragma once

#include <QObject>
#include <QGeoServiceProviderFactory>
#include <QGeoCodeReply>
#include <QGeoCodingManagerEngine>
#include <QNetworkReply>

class PhotonGeoCodeReply : public QGeoCodeReply {
    Q_OBJECT
public:
    explicit PhotonGeoCodeReply(QNetworkReply* reply, int limit, QObject* parent);
    ~PhotonGeoCodeReply();

    void abort() override;

private:
    void onNetworkReplyFinished();
    void onNetworkReplyError(QNetworkReply::NetworkError error);

    QNetworkReply* m_reply;
};


class PhotonGeoCodingManagerEngine : public QGeoCodingManagerEngine
{
    Q_OBJECT
public:
    explicit PhotonGeoCodingManagerEngine(const QVariantMap &parameters, QGeoServiceProvider::Error *error, QString *errorString);
    ~PhotonGeoCodingManagerEngine();

    PhotonGeoCodeReply* geocode(const QGeoAddress &address, const QGeoShape &bounds) override;
    PhotonGeoCodeReply* geocode(const QString &address, int limit, int offset, const QGeoShape &bounds) override;
    QGeoCodeReply* reverseGeocode(const QGeoCoordinate &coordinate, const QGeoShape &bounds) override;

private:
    QUrl m_baseUrl;
    QByteArray m_userAgent;
    QNetworkAccessManager m_networkManager;
};
