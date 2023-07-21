#ifndef TANKERKOENIGAPIREQUEST_H
#define TANKERKOENIGAPIREQUEST_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QScopedPointer>
#include <QGeoRectangle>

#include "fuelpriceprovider.h"

class TankerKoenigProvider : public FuelPriceProvider
{
    Q_OBJECT
    Q_PROPERTY(QString userAgent READ userAgent WRITE setUserAgent NOTIFY userAgentChanged)
public:
    explicit TankerKoenigProvider(QObject *parent = nullptr);

    QGeoRectangle boundingBox() const;

    QString userAgent() const { return m_userAgent; }
    void setUserAgent(const QString& value);

    FuelPriceReply* list(
            const QGeoCoordinate& coordinate, double radius,
            FuelPriceProvider::Fuel spirit, FuelPriceProvider::Sorting sorting);

signals:
    void userAgentChanged();

private:
    QNetworkAccessManager m_network;
    QString m_apiKey;
    QString m_userAgent;
};

class TankerKoenigPriceReply : public FuelPriceReply
{
    Q_OBJECT
public:
    explicit TankerKoenigPriceReply(
            const QGeoCoordinate& coordinate, double radius,
            FuelPriceProvider::Fuel fuel, FuelPriceProvider::Sorting sorting,
            QNetworkReply* reply);

private:
    QNetworkReply* m_reply;

    void onNetworkReplyFinished();
    void onNetworkReplyError();
};

#endif // TANKERKOENIGAPIREQUEST_H
