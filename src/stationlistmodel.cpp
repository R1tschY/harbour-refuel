#include "stationlistmodel.h"

#include <QLoggingCategory>

static Q_LOGGING_CATEGORY(logger, "refuel.models");

StationListModel::StationListModel(QObject *parent)
    : QAbstractListModel(parent)
{ }

void StationListModel::setProvider(FuelPriceProvider *provider)
{
    if (provider != m_provider) {
        m_provider = provider;
        emit providerChanged();
    }
}

void StationListModel::setError(const QString &errorString)
{
    if (m_errorString != errorString) {
        m_errorString = errorString;
        emit errorStringChanged();
        if (!m_errorString.isEmpty()) {
            setStatus(Status::Error);
        }
    }
}

void StationListModel::setStatus(Status status)
{
    if (m_status != status) {
        m_status = status;
        emit statusChanged();
    }
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
        return station.address.text();
    case CoordinateRole:
        return QVariant::fromValue(station.coordinate);
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
    roleNames.insert(CoordinateRole, "coordinate");
    roleNames.insert(DistanceRole, "distance");
    roleNames.insert(IsOpenRole, "isOpen");
    roleNames.insert(PriceRole, "price");
    return roleNames;
}

void StationListModel::search(
        const QGeoCoordinate& coordinate, float radius, int fuel)
{
    if (!m_provider) {
        return;
    }

    if (m_reply) {
        m_reply->abort();
        m_reply = nullptr;
    }

    beginResetModel();
    m_stations = { };
    endResetModel();

    setStatus(Status::Loading);
    setError({});

    m_reply = m_provider->list(
                coordinate, radius, (FuelPriceProvider::Fuel) fuel,
                FuelPriceProvider::Sorting::Price);
    connect(m_reply, &TankerKoenigPriceReply::finished,
            this, &StationListModel::onSearchResults);
    connect(m_reply, &TankerKoenigPriceReply::errorOccured,
            this, &StationListModel::onSearchError);
    connect(m_reply, &QObject::destroyed, this, [this]() { m_reply = nullptr; });
}

void StationListModel::reset()
{
    beginResetModel();
    m_stations = { };
    endResetModel();

    setError({});
    setStatus(Status::Null);
}

void StationListModel::onSearchResults()
{
    auto* reply = static_cast<TankerKoenigPriceReply*>(sender());
    if (reply != m_reply) {
        return;
    }

    beginResetModel();
    m_stations = reply->stations();
    endResetModel();

    setStatus(Status::Ready);
    m_reply = nullptr;
}

void StationListModel::onSearchError()
{
    auto* reply = static_cast<TankerKoenigPriceReply*>(sender());
    if (reply != m_reply) {
        return;
    }

    setError(reply->errorString());
    m_reply = nullptr;
}
