#include "fuelpriceprovider.h"

#include <QDebug>

FuelPriceProvider::FuelPriceProvider(QObject *parent)
    : QObject(parent)
{ }

void Reply::abort()
{
    if (!m_finished) {
        setFinished();
        emit aborted();
    }
}

Reply::Reply() = default;

void Reply::setError(Error error, const QString &errorString)
{
    if (m_finished) return;

    m_finished = true;
    m_error = error;
    m_errorString = errorString;
    clear();
    emit errorOccured();
}

void Reply::setFinished()
{
    if (m_finished) return;

    m_finished = true;
    emit finished();
}

FuelPriceReply::FuelPriceReply(const QGeoCoordinate &coordinate, double radius, FuelPriceProvider::Fuel fuel, FuelPriceProvider::Sorting sorting)
    : m_coordinate(coordinate)
    , m_radius(radius)
    , m_fuel(fuel)
    , m_sorting(sorting)
{ }


void FuelPriceReply::addStation(const StationWithPrice &station)
{
    m_stations.append(station);
}

void FuelPriceReply::setStations(const QVector<StationWithPrice> &stations)
{
    m_stations = stations;
}

FuelPriceReply::~FuelPriceReply() = default;

void FuelPriceReply::clear()
{
    m_stations.clear();
}

StationDetailsReply::StationDetailsReply(const QString &stationId)
    : m_stationId(stationId)
{ }


StationDetailsReply::~StationDetailsReply() = default;

void StationDetailsReply::setStationDetails(const StationDetails &station)
{
    m_stationDetails = station;
}

void StationDetailsReply::clear()
{
    m_stationDetails = StationDetails();
}
