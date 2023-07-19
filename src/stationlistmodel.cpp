#include "stationlistmodel.h"

StationListModel::StationListModel(QObject *parent)
    : QAbstractListModel(parent)
{
    connect(
                &m_request, &TankerKoenigApiRequest::listReceived,
                this, &StationListModel::onSearchResults);
}

int StationListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_stations.size();
}

QVariant StationListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int row = index.row();
    if (row >= m_stations.size() || row < 0) {
        return QVariant();
    }

    const auto& station = m_stations[row];
    switch (role) {
    case IdRole:
        return station.id;
    case NameRole:
        return station.name;
    case BrandRole:
        return station.brand;
    case AddressRole:
        return station.address;
    case LatituteRole:
        return station.latitude;
    case LongitudeRole:
        return station.longitude;
    case DistanceRole:
        return station.distance;
    case IsOpenRole:
        return station.isOpen;
    case PriceRole:
        return station.price;
    default:
        return QVariant();
    }
}


QHash<int, QByteArray> StationListModel::roleNames() const
{
    QHash<int, QByteArray> roleNames;
    roleNames.insert(IdRole, "stationId");
    roleNames.insert(NameRole, "name");
    roleNames.insert(BrandRole, "brand");
    roleNames.insert(AddressRole, "address");
    roleNames.insert(LatituteRole, "latitute");
    roleNames.insert(LongitudeRole, "longitude");
    roleNames.insert(DistanceRole, "distance");
    roleNames.insert(IsOpenRole, "isOpen");
    roleNames.insert(PriceRole, "price");
    return roleNames;
}

void StationListModel::search(
        float lat, float lng, float radius, int fuel)
{
    m_request.list(lat, lng, radius, (Request::Fuel) fuel,
                   Request::Sorting::Distance);
}

void StationListModel::onSearchResults(const QVector<Station>& stations)
{
    beginResetModel();
    m_stations = stations;
    endResetModel();
}
