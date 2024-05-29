#ifndef REQUEST_H
#define REQUEST_H

#include <QObject>
#include <QList>
#include <QHash>
#include <QGeoCoordinate>
#include <QGeoAddress>
#include <QVector>
#include <QGeoRectangle>
#include <QTime>

class FuelPriceReply;
class StationDetailsReply;
class StationUpdatesReply;

class FuelPriceProvider : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QGeoRectangle boundingBox READ boundingBox CONSTANT)
    Q_PROPERTY(QString copyright READ copyright CONSTANT)

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
    virtual QString copyright() const = 0;

    Q_INVOKABLE virtual FuelPriceReply* list(
            const QGeoCoordinate& coordinate, double radius,
            Fuel fuel, Sorting sorting) = 0;

    Q_INVOKABLE virtual StationDetailsReply* stationForId(
            const QString& id) = 0;

    Q_INVOKABLE virtual StationUpdatesReply* pricesForStations(
            const QStringList& ids) = 0;
};

inline int qHash(const FuelPriceProvider::Fuel& value, uint seed = 0) {
    return qHash(static_cast<int>(value), seed);
}

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

class OpeningTime {
    Q_GADGET
    Q_PROPERTY(WeekDays weekDays READ weekDays CONSTANT FINAL)
    Q_PROPERTY(QString weekDaysText READ weekDaysText CONSTANT FINAL)
    Q_PROPERTY(QTime start READ start CONSTANT FINAL)
    Q_PROPERTY(QTime end READ end CONSTANT FINAL)
public:
    enum WeekDay {
        NoDays = 0,
        Monday = 1 << 0,
        Tuesday = 1 << 1,
        Wednesday = 1 << 2,
        Thursday = 1 << 3,
        Friday = 1 << 4,
        Saturday = 1 << 5,
        Sunday = 1 << 6,
        PublicHoliday = 1 << 7
    };
    Q_DECLARE_FLAGS(WeekDays, WeekDay)
    Q_FLAG(WeekDays)

    OpeningTime() = default;

    OpeningTime(WeekDays days, const QString& weekDaysText, QTime start, QTime end)
        : m_weekDays(days)
        , m_start(start)
        , m_end(end)
        , m_weekDaysText(weekDaysText)
    { }

    WeekDays weekDays() const { return m_weekDays; }
    QString weekDaysText() const { return m_weekDaysText; }
    QTime start() const { return m_start; }
    QTime end() const { return m_end; }
    bool isNull() const { return m_weekDays == 0; }

private:
    WeekDays m_weekDays;
    QTime m_start;
    QTime m_end;
    QString m_weekDaysText;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(OpeningTime::WeekDays)

struct StationDetails {
    QString id;
    QString name;
    QString brand;
    QGeoAddress address;
    QGeoCoordinate coordinate;
    QVector<OpeningTime> openingTimes;
    QStringList openingTimesOverrides;
    QHash<FuelPriceProvider::Fuel, float> prices;
    bool isOpen;
    bool wholeDay;
};

struct StationUpdate {
    QString id;
    QHash<FuelPriceProvider::Fuel, float> prices;
    bool isOpen;
};

class Reply : public QObject {
    Q_OBJECT
public:
    enum Error {
        NoError,
        CommunicationError,
        ParseError,
        UnknownError
    };

    bool isFinished() const { return m_finished; }
    Error error() const { return m_error; }
    QString errorString() const { return m_errorString; }

    virtual void abort();

Q_SIGNALS:
    void aborted();
    void finished();
    void errorOccured();

protected:
    explicit Reply();

    void setError(Error error, const QString &errorString);
    void setFinished();

    virtual void clear() = 0;

private:
    bool m_finished = false;
    Error m_error = Error::NoError;
    QString m_errorString;
};

class FuelPriceReply : public Reply {
    Q_OBJECT
public:
    virtual ~FuelPriceReply();

    QGeoCoordinate coordinate() const { return m_coordinate; }
    double radius() const { return m_radius; }
    FuelPriceProvider::Fuel fuel() const { return m_fuel; }
    FuelPriceProvider::Sorting sorting() const { return m_sorting; }
    QVector<StationWithPrice> stations() const { return m_stations; }

protected:
    explicit FuelPriceReply(
            const QGeoCoordinate& coordinate, double radius,
            FuelPriceProvider::Fuel fuel, FuelPriceProvider::Sorting sorting);

    void addStation(const StationWithPrice &location);
    void setStations(const QVector<StationWithPrice> &stations);  

private:
    QGeoCoordinate m_coordinate;
    double m_radius;
    FuelPriceProvider::Fuel m_fuel;
    FuelPriceProvider::Sorting m_sorting;

    QVector<StationWithPrice> m_stations;

    void clear() override;
};

class StationDetailsReply : public Reply {
    Q_OBJECT
public:
    virtual ~StationDetailsReply();

    QString stationId() const { return m_stationId; }
    StationDetails stationDetails() const { return m_stationDetails; }

protected:
    explicit StationDetailsReply(const QString& stationId);

    void setStationDetails(const StationDetails& station);

private:
    QString m_stationId;

    StationDetails m_stationDetails;

    void clear() override;
};

class StationUpdatesReply : public Reply {
    Q_OBJECT
public:
    virtual ~StationUpdatesReply();

    QStringList stationIds() const { return m_stationIds; }
    QVector<StationUpdate> stationUpdates() const { return m_stationUpdates; }

protected:
    explicit StationUpdatesReply(const QStringList& stationIds);

    void addStationUpdate(const StationUpdate& update);

private:
    QStringList m_stationIds;

    QVector<StationUpdate> m_stationUpdates;

    void clear() override;
};


#endif // REQUEST_H
