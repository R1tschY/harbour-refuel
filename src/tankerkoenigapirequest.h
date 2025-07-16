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
    QString copyright() const override;
    QStringList fuels() const override;

    QString userAgent() const { return m_userAgent; }
    void setUserAgent(const QString& value);

    FuelPriceReply* list(
            const QGeoCoordinate& coordinate, double radius,
            const QString& fuelId, FuelPriceProvider::Sorting sorting
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
            const QString& fuelId, FuelPriceProvider::Sorting sorting,
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
