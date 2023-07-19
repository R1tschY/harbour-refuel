#ifndef STATIONLISTMODEL_H
#define STATIONLISTMODEL_H

#include <QAbstractListModel>

#include "tankerkoenigapirequest.h"
#include "request.h"

class StationListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit StationListModel(QObject *parent = nullptr);

    enum Roles {
        IdRole = Qt::UserRole,
        NameRole,
        BrandRole,
        AddressRole,
        LatituteRole,
        LongitudeRole,
        DistanceRole,
        IsOpenRole,
        PriceRole,
    };

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void search(
            float lat, float lng, float radius, int fuel);

private:
    QVector<Station> m_stations;
    TankerKoenigApiRequest m_request;

    void onSearchResults(const QVector<Station>& stations);
};

#endif // STATIONLISTMODEL_H
