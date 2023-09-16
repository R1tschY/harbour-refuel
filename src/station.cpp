#include "station.h"

#include <QLoggingCategory>

static Q_LOGGING_CATEGORY(logger, "refuel.station");

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

void Station::setStatus(Status status)
{
    if (m_status != status) {
        m_status = status;
        emit statusChanged();
    }
}

void Station::fetchDetails()
{
    if (m_provider && !m_id.isEmpty()) {
        m_reply = m_provider->stationForId(m_id);
        connect(m_reply, &StationDetailsReply::finished,
                this, &Station::onSearchResults);
        connect(m_reply, &StationDetailsReply::errorOccured,
                this, &Station::onSearchError);
        connect(m_reply, &QObject::destroyed, this, [this]() { m_reply = nullptr; });
        m_status = Status::Loading;
    }
}

void Station::update()
{

}

float Station::priceFor(int fuel) const
{
    qCDebug(logger) << m_prices
                    << fuel
                    << static_cast<FuelPriceProvider::Fuel>(fuel);
    return m_prices.value(
                static_cast<FuelPriceProvider::Fuel>(fuel),
                std::numeric_limits<float>::quiet_NaN());
}

void Station::onSearchResults()
{
    auto* reply = static_cast<StationDetailsReply*>(sender());
    if (reply != m_reply) {
        return;
    }

   StationDetails stations = reply->stationDetails();
   m_name = stations.name;
   m_brand = stations.brand;
   m_address = stations.address;
   m_coordinate = stations.coordinate;
   m_openingTimes = stations.openingTimes;
   m_openingTimesOverrides = stations.openingTimesOverrides;
   m_prices = stations.prices;
   m_isOpen = stations.isOpen;
   m_wholeDay = stations.wholeDay;
   qCDebug(logger) << m_prices;

   setStatus(Status::Ready);
   emit detailsFetched();
   m_reply = nullptr;
}

void Station::onSearchError()
{
    auto* reply = static_cast<StationDetailsReply*>(sender());
    if (reply != m_reply) {
        return;
    }

    setError(reply->errorString());
    m_reply = nullptr;
}
