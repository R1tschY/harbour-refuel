/*
 * Copyright 2025 Richard Liebscher <r1tschy@posteo.de>.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
 
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
    Q_PROPERTY(Status detailsStatus READ detailsStatus NOTIFY detailsStatusChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorStringChanged)
    Q_PROPERTY(QString detailsErrorString READ detailsErrorString NOTIFY detailsErrorStringChanged)
    Q_PROPERTY(QString name READ name NOTIFY detailsFetched)
    Q_PROPERTY(QString brand READ brand NOTIFY detailsFetched)
    Q_PROPERTY(QGeoAddress address READ address NOTIFY detailsFetched)
    Q_PROPERTY(QGeoCoordinate coordinate READ coordinate NOTIFY detailsFetched)
    Q_PROPERTY(QVariantList openingTimes READ openingTimes NOTIFY detailsFetched)
    Q_PROPERTY(QStringList openingTimesOverrides READ openingTimesOverrides NOTIFY detailsFetched)
    Q_PROPERTY(bool isOpen READ isOpen NOTIFY updated)
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
    Status detailsStatus() const { return m_detailsStatus; }
    QString errorString() const { return m_errorString; }
    QString detailsErrorString() const { return m_detailsErrorString; }

    QString stationId() const { return m_id; }
    void setStationId(const QString& value);

    FuelPriceProvider* provider() const { return m_provider; }
    void setProvider(FuelPriceProvider* provider);

    Q_INVOKABLE void fetchDetails();
    Q_INVOKABLE void update();
    Q_INVOKABLE float priceFor(const QString& fuelId) const;

signals:
    void stationIdChanged();
    void providerChanged();
    void statusChanged();
    void detailsStatusChanged();
    void errorStringChanged();
    void detailsErrorStringChanged();
    void detailsFetched();
    void updated();

private:
    QString m_id;
    QString m_name;
    QString m_brand;
    QGeoAddress m_address;
    QGeoCoordinate m_coordinate;
    QVariantList m_openingTimes;
    QStringList m_openingTimesOverrides;
    QHash<QString, float> m_prices;
    bool m_isOpen;
    bool m_wholeDay;

    FuelPriceProvider* m_provider = nullptr;
    Status m_detailsStatus = Status::Null;
    Status m_status = Status::Null;
    QString m_errorString;
    QString m_detailsErrorString;
    StationDetailsReply* m_detailsReply = nullptr;
    StationUpdatesReply* m_updateReply = nullptr;

    void setError(const QString &errorString);
    void setDetailsError(const QString &errorString);
    void setStatus(Status status);
    void setDetailsStatus(Status status);
    void onSearchResults();
    void onSearchError();
    void onUpdateResults();
    void onUpdateError();
};

#endif // STATION_H
