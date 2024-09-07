#include "station.h"

#include <QLoggingCategory>

Station::Station(QObject *parent) : QObject(parent)
{

}

void Station::setStationId(const QString &value)
{
    if (value != m_id) {
        m_id = value;
        emit stationIdChanged();
    }
}

void Station::setProvider(FuelPriceProvider *provider)
{
    if (provider != m_provider) {
        m_provider = provider;
        emit providerChanged();
    }
}

void Station::setError(const QString &errorString)
{
    if (m_errorString != errorString) {
        m_errorString = errorString;
        emit errorStringChanged();
        if (!m_errorString.isEmpty()) {
            setStatus(Status::Error);
        }
    }
}

void Station::setDetailsError(const QString &errorString)
{
    if (m_detailsErrorString != errorString) {
        m_detailsErrorString = errorString;
        emit detailsErrorStringChanged();
        if (!m_detailsErrorString.isEmpty()) {
            setDetailsStatus(Status::Error);
        }
    }
}

void Station::setStatus(Status status)
{
    if (m_status != status) {
        m_status = status;
        emit statusChanged();
    }
}

void Station::setDetailsStatus(Status status)
{
    if (m_detailsStatus != status) {
        m_detailsStatus = status;
        emit statusChanged();
    }
}

void Station::fetchDetails()
{
    if (m_provider && !m_id.isEmpty() && m_detailsStatus != Status::Loading) {
        m_detailsReply = m_provider->stationForId(m_id);
        connect(m_detailsReply, &StationDetailsReply::finished,
                this, &Station::onSearchResults);
        connect(m_detailsReply, &StationDetailsReply::errorOccured,
                this, &Station::onSearchError);
        connect(m_detailsReply, &QObject::destroyed, this, [this]() { m_detailsReply = nullptr; });
        setDetailsStatus(Status::Loading);
        setStatus(Status::Loading);
    }
}

float Station::priceFor(const QString& fuelId) const
{
    return m_prices.value(fuelId, std::numeric_limits<float>::quiet_NaN());
}

void Station::onSearchResults()
{
    auto* reply = static_cast<StationDetailsReply*>(sender());
    if (reply != m_detailsReply) {
        return;
    }

   const StationDetails stations = reply->stationDetails();
   m_name = stations.name;
   m_brand = stations.brand;
   m_address = stations.address;
   m_coordinate = stations.coordinate;
   m_openingTimes.clear();
   m_openingTimes.reserve(stations.openingTimes.size());
   for (auto openingTime : stations.openingTimes) {
       m_openingTimes.append(QVariant::fromValue(openingTime));
   }

   m_openingTimesOverrides = stations.openingTimesOverrides;
   m_prices = stations.prices;
   m_isOpen = stations.isOpen;
   m_wholeDay = stations.wholeDay;

   setStatus(Status::Ready);
   setDetailsStatus(Status::Ready);
   emit detailsFetched();
   emit updated();
   m_detailsReply = nullptr;
}

void Station::onSearchError()
{
    auto* reply = static_cast<StationDetailsReply*>(sender());
    if (reply != m_detailsReply) {
        return;
    }

    setDetailsError(reply->errorString());
    m_detailsReply = nullptr;
}

void Station::update()
{
    if (m_provider && !m_id.isEmpty() && m_status != Status::Loading) {
        m_updateReply = m_provider->pricesForStations({ m_id });
        connect(m_updateReply, &StationUpdatesReply::finished,
                this, &Station::onUpdateResults);
        connect(m_updateReply, &StationUpdatesReply::errorOccured,
                this, &Station::onUpdateError);
        connect(m_updateReply, &QObject::destroyed, this, [this]() { m_updateReply = nullptr; });
        setStatus(Status::Loading);
    }
}

void Station::onUpdateResults()
{
    auto* reply = static_cast<StationUpdatesReply*>(sender());
    if (reply != m_updateReply) {
        return;
    }

   const QVector<StationUpdate> updates = reply->stationUpdates();
   if (updates.size() != 1 || updates[0].id != m_id) {
       setError(tr("Wrong update data received"));
       m_updateReply = nullptr;
       return;
   }

   const auto update = updates[0];
   m_prices = update.prices;
   m_isOpen = update.isOpen;

   setStatus(Status::Ready);
   emit updated();
   m_updateReply = nullptr;
}

void Station::onUpdateError()
{
    auto* reply = static_cast<StationUpdatesReply*>(sender());
    if (reply != m_updateReply) {
        return;
    }

    setError(reply->errorString());
    m_updateReply = nullptr;
}
