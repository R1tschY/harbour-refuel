#ifndef STATIONLISTMODEL_H
#define STATIONLISTMODEL_H

#include <QAbstractListModel>

#include "tankerkoenigapirequest.h"
#include "fuelpriceprovider.h"

class StationListModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorStringChanged)
    Q_PROPERTY(FuelPriceProvider* provider READ provider WRITE setProvider NOTIFY providerChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)

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

    FuelPriceProvider* provider() const { return m_provider; }
    void setProvider(FuelPriceProvider* provider);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void search(
            const QGeoCoordinate& coordinate, float radius, int fuel);
    Q_INVOKABLE void reset();

signals:
    void errorStringChanged();
    void statusChanged();
    void providerChanged();

private:
    QVector<StationWithPrice> m_stations;
    FuelPriceProvider* m_provider = nullptr;
    QString m_errorString;
    FuelPriceReply* m_reply = nullptr;
    Status m_status = Status::Null;

    void onSearchResults();
    void onSearchError();
    void setError(const QString& errorString);
    void setStatus(Status status);
};

#endif // STATIONLISTMODEL_H
