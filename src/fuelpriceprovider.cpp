#include "fuelpriceprovider.h"

#include <QDebug>

const QString FuelPriceProvider::GASOLINE_95_E5 = QString::fromLatin1("g95-e5");
const QString FuelPriceProvider::GASOLINE_95_E10 = QString::fromLatin1("g95");
const QString FuelPriceProvider::GASOLINE_98 = QString::fromLatin1("g98");
const QString FuelPriceProvider::DIESEL = QString::fromLatin1("d");

FuelPriceProvider::FuelPriceProvider(QObject *parent)
    : QObject(parent)
{ }

QString FuelPriceProvider::fuelName(const QString &fuelId)
{
    if (fuelId == GASOLINE_95_E5) {
        //: Gasoline with 95 RON and 5 percent ethanol
        return tr("Unleaded E5");
    } else if (fuelId == GASOLINE_95_E10) {
        //: Gasoline with 95 RON and 10 percent ethanol
        return tr("Unleaded E10");
    } else if (fuelId == GASOLINE_98) {
        //: Gasoline with 98 RON
        return tr("Unleaded Premium");
    } else if (fuelId == DIESEL) {
        return tr("Diesel");
    } else {
        return QStringLiteral("Unknown: ") + fuelId;
    }
}

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

FuelPriceReply::FuelPriceReply(const QGeoCoordinate &coordinate, double radius, const QString& fuelId, FuelPriceProvider::Sorting sorting)
    : m_coordinate(coordinate)
    , m_radius(radius)
    , m_fuelId(fuelId)
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

StationUpdatesReply::~StationUpdatesReply() = default;

StationUpdatesReply::StationUpdatesReply(const QStringList &stationIds)
    : m_stationIds(stationIds)
{ }

void StationUpdatesReply::addStationUpdate(const StationUpdate &update)
{
    m_stationUpdates.append(update);
}

void StationUpdatesReply::clear()
{
    m_stationUpdates.clear();
}
