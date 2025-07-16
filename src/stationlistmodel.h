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
 
#ifndef STATIONLISTMODEL_H
#define STATIONLISTMODEL_H

#include <QAbstractListModel>

#include "tankerkoenigapirequest.h"
#include "fuelpriceprovider.h"

class StationListModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(FuelPriceProvider* provider READ provider WRITE setProvider NOTIFY providerChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorStringChanged)
    Q_PROPERTY(double lowestPrice READ lowestPrice NOTIFY lowestPriceChanged)

public:
    explicit StationListModel(QObject *parent = nullptr);

    enum Status {
        Null,
        Loading,
        Error,
        Ready
    };
    Q_ENUM(Status);

    enum Roles {
        IdRole = Qt::UserRole,
        NameRole,
        BrandRole,
        AddressRole,
        CoordinateRole,
        DistanceRole,
        IsOpenRole,
        PriceRole,
    };

    Status status() const { return m_status; }
    QString errorString() const { return m_errorString; }

    double lowestPrice() const { return m_lowestPrice; }

    FuelPriceProvider* provider() const { return m_provider; }
    void setProvider(FuelPriceProvider* provider);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void search(const QGeoCoordinate& coordinate, float radius, const QString& fuelId);
    Q_INVOKABLE void reset();

signals:
    void errorStringChanged();
    void statusChanged();
    void providerChanged();
    void lowestPriceChanged();

private:
    QVector<StationWithPrice> m_stations;
    FuelPriceProvider* m_provider = nullptr;
    QString m_errorString;
    FuelPriceReply* m_reply = nullptr;
    Status m_status = Status::Null;
    double m_lowestPrice = -1;

    void onSearchResults();
    void onSearchError();
    void setError(const QString& errorString);
    void setStatus(Status status);
};

#endif // STATIONLISTMODEL_H
