#ifndef REQUEST_H
#define REQUEST_H

#include <QObject>
#include <QList>
#include <QHash>
#include <QGeoCoordinate>
#include <QGeoAddress>
#include <QVector>
#include <QGeoRectangle>

struct StationWithPrice {
    QString id;
    QString name;
    QString brand;
    QGeoAddress address;
    QGeoCoordinate coordinate;
    double distance;
    bool isOpen;
    double price;
};

struct StationDetails {
    QString id;
    QString name;
    QString brand;
    QGeoAddress address;
    QGeoCoordinate coordinate;
    QList<QString> openingTimes;
    QList<QString> openingTimesOverrides;
    QHash<int, float> prices;
    bool isOpen;
    bool wholeDay;
};

class FuelPriceReply;


class FuelPriceProvider : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QGeoRectangle boundingBox READ boundingBox CONSTANT)

public:
    explicit FuelPriceProvider(QObject *parent = nullptr);

    enum class Fuel {
        SuperE5,
        SuperE10,
        Diesel
    };
    Q_ENUM(Fuel)

    enum class Sorting {
        Price,
        Distance
    };
    Q_ENUM(Sorting)

    virtual QGeoRectangle boundingBox() const = 0;

    Q_INVOKABLE virtual FuelPriceReply* list(
            const QGeoCoordinate& coordinate, double radius,
            Fuel fuel, Sorting sorting) = 0;
};

class FuelPriceReply : public QObject {
    Q_OBJECT
public:
    enum Error {
        NoError,
        CommunicationError,
        ParseError,
        UnknownError
    };

    virtual ~FuelPriceReply();

    bool isFinished() const { return m_finished; }
    Error error() const { return m_error; }
    QString errorString() const { return m_errorString; }

    QGeoCoordinate coordinate() const { return m_coordinate; }
    double radius() const { return m_radius; }
    FuelPriceProvider::Fuel fuel() const { return m_fuel; }
    FuelPriceProvider::Sorting sorting() const { return m_sorting; }
    QVector<StationWithPrice> stations() const { return m_stations; }

    virtual void abort();

Q_SIGNALS:
    void aborted();
    void finished();
    void errorOccured();

protected:
    explicit FuelPriceReply(
            const QGeoCoordinate& coordinate, double radius,
            FuelPriceProvider::Fuel fuel, FuelPriceProvider::Sorting sorting);

    void setError(Error error, const QString &errorString);
    void setFinished();

    void addStation(const StationWithPrice &location);
    void setStations(const QVector<StationWithPrice> &stations);

private:
    QGeoCoordinate m_coordinate;
    double m_radius;
    FuelPriceProvider::Fuel m_fuel;
    FuelPriceProvider::Sorting m_sorting;

    bool m_finished = false;
    Error m_error = Error::NoError;
    QString m_errorString;
    QVector<StationWithPrice> m_stations;
};

#endif // REQUEST_H
