#include "fuelpriceprovider.h"

#include <QDebug>

FuelPriceProvider::FuelPriceProvider(QObject *parent)
    : QObject(parent)
{ }

void FuelPriceReply::abort()
{
    if (!m_finished) {
        setFinished();
        emit aborted();
    }
}

FuelPriceReply::FuelPriceReply(const QGeoCoordinate &coordinate, double radius, FuelPriceProvider::Fuel fuel, FuelPriceProvider::Sorting sorting)
    : m_coordinate(coordinate)
    , m_radius(radius)
    , m_fuel(fuel)
    , m_sorting(sorting)
{ }

void FuelPriceReply::setError(Error error, const QString &errorString)
{
    if (m_finished) return;

    m_finished = true;
    m_error = error;
    m_errorString = errorString;
    m_stations.clear();
    emit errorOccured();
}

void FuelPriceReply::setFinished()
{
    if (m_finished) return;

    m_finished = true;
    emit finished();
}

void FuelPriceReply::addStation(const StationWithPrice &station)
{
    m_stations.append(station);
}

void FuelPriceReply::setStations(const QVector<StationWithPrice> &stations)
{
    m_stations = stations;
}

FuelPriceReply::~FuelPriceReply() = default;

