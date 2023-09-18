#ifndef STATION_H
#define STATION_H

#include <QObject>
#include <QString>
#include <QGeoAddress>
#include <QVector>
#include <QStringList>
#include <QHash>

#include "fuelpriceprovider.h"

class Station : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString stationId READ stationId WRITE setStationId NOTIFY stationIdChanged)
    Q_PROPERTY(FuelPriceProvider* provider READ provider WRITE setProvider NOTIFY providerChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorStringChanged)
    Q_PROPERTY(QString name READ name NOTIFY detailsFetched)
    Q_PROPERTY(QString brand READ brand NOTIFY detailsFetched)
    Q_PROPERTY(QGeoAddress address READ address NOTIFY detailsFetched)
    Q_PROPERTY(QGeoCoordinate coordinate READ coordinate NOTIFY detailsFetched)
    Q_PROPERTY(QVariantList openingTimes READ openingTimes NOTIFY detailsFetched)
    Q_PROPERTY(QStringList openingTimesOverrides READ openingTimesOverrides NOTIFY detailsFetched)
    Q_PROPERTY(bool isOpen READ isOpen NOTIFY detailsFetched)
    Q_PROPERTY(bool wholeDay READ wholeDay NOTIFY detailsFetched)

public:
    enum Status {
        Null,
        Loading,
        Error,
        Ready
    };
    Q_ENUM(Status);

    explicit Station(QObject *parent = nullptr);

    QString name() const { return m_name; }
    QString brand() const { return m_brand; }
    QGeoAddress address() const { return m_address; }
    QGeoCoordinate coordinate() const { return m_coordinate; }
    QVariantList openingTimes() const { return m_openingTimes; }
    QStringList openingTimesOverrides() const { return m_openingTimesOverrides; }
    bool isOpen() const { return m_isOpen; }
    bool wholeDay() const { return m_wholeDay; }

    Status status() const { return m_status; }
    QString errorString() const { return m_errorString; }

    QString stationId() const { return m_id; }
    void setStationId(const QString& value);

    FuelPriceProvider* provider() const { return m_provider; }
    void setProvider(FuelPriceProvider* provider);

    Q_INVOKABLE void fetchDetails();
    Q_INVOKABLE void update();
    Q_INVOKABLE float priceFor(int) const;

signals:
    void stationIdChanged();
    void providerChanged();
    void statusChanged();
    void errorStringChanged();
    void detailsFetched();

private:
    QString m_id;
    QString m_name;
    QString m_brand;
    QGeoAddress m_address;
    QGeoCoordinate m_coordinate;
    QVariantList m_openingTimes;
    QStringList m_openingTimesOverrides;
    QHash<FuelPriceProvider::Fuel, float> m_prices;
    bool m_isOpen;
    bool m_wholeDay;

    FuelPriceProvider* m_provider = nullptr;
    Status m_status = Status::Null;
    QString m_errorString;
    StationDetailsReply* m_reply = nullptr;

    void setError(const QString &errorString);
    void setStatus(Status status);
    void onSearchResults();
    void onSearchError();
};

#endif // STATION_H
