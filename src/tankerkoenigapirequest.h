#ifndef TANKERKOENIGAPIREQUEST_H
#define TANKERKOENIGAPIREQUEST_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QScopedPointer>
#include <QGeoRectangle>

#include "fuelpriceprovider.h"
#include "config.h"

struct TankerKoenigApiKey {
    constexpr static const char* get() {
        return TANKERKOENIG_APIKEY;
    }
};

class TankerKoenigProvider : public FuelPriceProvider
{
    Q_OBJECT
    Q_PROPERTY(QString userAgent READ userAgent WRITE setUserAgent NOTIFY userAgentChanged)
public:
    explicit TankerKoenigProvider(QObject *parent = nullptr);

    QGeoRectangle boundingBox() const override;

    QString userAgent() const { return m_userAgent; }
    void setUserAgent(const QString& value);

    FuelPriceReply* list(
            const QGeoCoordinate& coordinate, double radius,
            FuelPriceProvider::Fuel spirit, FuelPriceProvider::Sorting sorting
            ) override;

    StationDetailsReply *stationForId(const QString &id) override;

    StationUpdatesReply* pricesForStations(
            const QStringList& ids) override;

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
    void onNetworkReplyFinished();
    void onNetworkReplyError();
};

class TankerKoenigStationDetailsReply : public StationDetailsReply
{
    Q_OBJECT
public:
    explicit TankerKoenigStationDetailsReply(
            const QString& stationId, QNetworkReply* reply);

private:
    void onNetworkReplyFinished();
    void onNetworkReplyError();
};

class TankerKoenigStationUpdatesReply : public StationUpdatesReply
{
    Q_OBJECT
public:
    explicit TankerKoenigStationUpdatesReply(
            const QStringList& stationIds, QNetworkReply* reply);

private:
    void onNetworkReplyFinished();
    void onNetworkReplyError();
};

#endif // TANKERKOENIGAPIREQUEST_H
