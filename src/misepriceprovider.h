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

#ifndef MISEPRICEPROVIDER_H
#define MISEPRICEPROVIDER_H

#include <QGeoRectangle>
#include <QNetworkAccessManager>
#include <QObject>
#include <QVector>

#include "fuelpriceprovider.h"

class MisePriceProvider : public FuelPriceProvider
{
    Q_OBJECT
    Q_PROPERTY(QString userAgent READ userAgent WRITE setUserAgent NOTIFY userAgentChanged)
public:
    explicit MisePriceProvider(QObject *parent = nullptr);

    QGeoRectangle boundingBox() const override;
    QString copyright() const override;
    QStringList fuels() const override;

    QString userAgent() const { return m_userAgent; }
    void setUserAgent(const QString &value);

    FuelPriceReply *list(const QGeoCoordinate &coordinate,
                         double radius,
                         const QString &fuelId,
                         FuelPriceProvider::Sorting sorting) override;

    StationDetailsReply *stationForId(const QString &id) override;

    StationUpdatesReply *pricesForStations(const QStringList &ids) override;

    Q_INVOKABLE QString fuelName(const QString &fuelId) override;

signals:
    void userAgentChanged();

private:
    QNetworkAccessManager m_network;
    QString m_userAgent;
};

class MisePriceReply : public FuelPriceReply
{
    Q_OBJECT
public:
    explicit MisePriceReply(const QGeoCoordinate &coordinate,
                            double radius,
                            const QString &fuelId,
                            FuelPriceProvider::Sorting sorting,
                            QNetworkReply *reply);

private:
    void onNetworkReplyFinished();
    void onNetworkReplyError();
};

class MiseStationDetailsReply : public StationDetailsReply
{
    Q_OBJECT
public:
    explicit MiseStationDetailsReply(const QString &stationId,
                                     const QGeoCoordinate &coordinate,
                                     QNetworkReply *reply = nullptr);

    Q_INVOKABLE void reportError();

private:
    void onNetworkReplyFinished();
    void onNetworkReplyError();

    QGeoCoordinate m_coordinate;
};

class MiseStationUpdatesReply : public StationUpdatesReply
{
    Q_OBJECT
public:
    explicit MiseStationUpdatesReply(const QStringList &stationIds,
                                     QNetworkAccessManager *network,
                                     const QString &userAgent);
    ~MiseStationUpdatesReply() override;

    Q_INVOKABLE void reportEmpty();

private:
    void onNetworkReplyFinished();
    void onNetworkReplyError();
    void checkAllFinished();

    QVector<QNetworkReply *> m_pendingReplies;
};

#endif // MISEPRICEPROVIDER_H
