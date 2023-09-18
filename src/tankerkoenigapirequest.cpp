#include "tankerkoenigapirequest.h"

#include <optional>

#include <QUrlQuery>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLoggingCategory>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QStringBuilder>
#include <QGeoCoordinate>

#include "qtployfill.h"
#include "char_sequence.h"

static Q_LOGGING_CATEGORY(logger, "refuel.tankerkoenig");

static const QUrl LIST_URL = QUrl(QStringLiteral("https://creativecommons.tankerkoenig.de/json/list.php"));
static const QUrl DETAILS_URL = QUrl(QStringLiteral("https://creativecommons.tankerkoenig.de/json/detail.php"));

static const QString API_KEY = charSeqToQString(
            make_char_sequence<TankerKoenigApiKey>{});

TankerKoenigProvider::TankerKoenigProvider(QObject *parent)
    : FuelPriceProvider(parent)
    , m_network()
    , m_apiKey(API_KEY)
{ }

QGeoRectangle TankerKoenigProvider::boundingBox() const
{
    // Data © OpenStreetMap contributors, ODbL 1.0. https://osm.org/copyright
    // > curl "https://nominatim.openstreetmap.org/search?q=germany&format=json&limit=1"
    //   | jq '.[0].boundingbox'
    return QGeoRectangle(
                QGeoCoordinate(47.2701114, 5.8663153),
                QGeoCoordinate(55.099161, 15.0419309));
}

void TankerKoenigProvider::setUserAgent(const QString &value)
{
    if (m_userAgent != value) {
        m_userAgent = value;
        emit userAgentChanged();
    }
}

FuelPriceReply *TankerKoenigProvider::list(const QGeoCoordinate &coordinate, double radius, Fuel fuel,
        Sorting sorting)
{
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("apikey"), m_apiKey);
    query.addQueryItem(QStringLiteral("lat"), QString::number(coordinate.latitude()));
    query.addQueryItem(QStringLiteral("lng"), QString::number(coordinate.longitude()));
    query.addQueryItem(QStringLiteral("rad"), QString::number(radius));

    QString fuelStr;
    switch (fuel) {
    case FuelPriceProvider::Fuel::SuperE5:
        fuelStr = QStringLiteral("e5");
        break;
    case FuelPriceProvider::Fuel::SuperE10:
        fuelStr = QStringLiteral("e10");
        break;
    case FuelPriceProvider::Fuel::Diesel:
        fuelStr = QStringLiteral("diesel");
        break;
    }
    query.addQueryItem(QStringLiteral("type"), fuelStr);

    QString sortingStr;
    switch (sorting) {
    case FuelPriceProvider::Sorting::Price:
        sortingStr = QStringLiteral("price");
        break;
    case FuelPriceProvider::Sorting::Distance:
        sortingStr = QStringLiteral("dist");
        break;
    }
    query.addQueryItem(QStringLiteral("sort"), sortingStr);

    QUrl url = LIST_URL;
    url.setQuery(query);

    QNetworkRequest request { url };
    if (!m_userAgent.isEmpty()) {
        request.setHeader(
                    QNetworkRequest::KnownHeaders::UserAgentHeader,
                    m_userAgent);
    }
    request.setRawHeader("Accept", "application/json;charset=utf-8");
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);

    qCInfo(logger) << "Start list request";
    qCDebug(logger) << "Request URL" << url;
    auto* reply = m_network.get(request);


    return new TankerKoenigPriceReply(
                coordinate, radius, fuel, sorting, reply);
}

TankerKoenigPriceReply::TankerKoenigPriceReply(
        const QGeoCoordinate& coordinate, double radius,
        FuelPriceProvider::Fuel fuel, FuelPriceProvider::Sorting sorting,
        QNetworkReply *reply)
    : FuelPriceReply(coordinate, radius, fuel, sorting)
{
    reply->setReadBufferSize(0);

    connect(reply, &QNetworkReply::finished,
            this, &TankerKoenigPriceReply::onNetworkReplyFinished);
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
            this, &TankerKoenigPriceReply::onNetworkReplyError);
    connect(this, &QObject::destroyed, reply, &QObject::deleteLater);
    connect(this, &FuelPriceReply::aborted, reply, &QNetworkReply::abort);
}

void TankerKoenigPriceReply::onNetworkReplyFinished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        return;
    }

    QJsonParseError error;
    auto jsonDoc = QJsonDocument::fromJson(reply->readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
        setError(Error::ParseError, QStringLiteral("Server error: invalid JSON"));
        return;
    }

    auto root = jsonDoc.object();
    bool ok = root.value(QStringLiteral("ok")).toBool(false);
    if (!ok) {
        auto message = root.value("message").toString("<missing>");
        setError(Error::UnknownError, QStringLiteral("Server error: ") % message);
        return;
    }

    QString status = root.value(QStringLiteral("status")).toString();
    if (status != QStringLiteral("ok")) {
        setError(Error::UnknownError, QStringLiteral("Status error: ") % status);
        return;
    }

    const auto stations = root.value(QStringLiteral("stations")).toArray();
    qCInfo(logger) << "Got list result with" << stations.size() << "items";

    for (auto station : stations) {
        auto stationObject = station.toObject();
        auto id = stationObject.value(QStringLiteral("id")).toString();
        auto name = stationObject.value(QStringLiteral("name")).toString();
        auto brand = stationObject.value(QStringLiteral("brand")).toString();
        auto street = stationObject.value(QStringLiteral("street")).toString();
        auto houseNumber = stationObject.value(QStringLiteral("houseNumber")).toString();
        auto postCode = stationObject.value(QStringLiteral("postCode")).toInt();
        auto place = stationObject.value(QStringLiteral("place")).toString();
        float lat = stationObject.value(QStringLiteral("lat")).toDouble();
        float lng = stationObject.value(QStringLiteral("lng")).toDouble();
        float dist = stationObject.value(QStringLiteral("dist")).toDouble();
        auto isOpen = stationObject.value(QStringLiteral("isOpen")).toBool();
        float price = stationObject.value(QStringLiteral("price")).toDouble();

        QGeoAddress address;
        address.setCity(place);
        address.setCountryCode(QStringLiteral("de"));
        address.setPostalCode(QString::number(postCode).rightJustified(5, '0'));
        address.setStreet(street % QChar(' ') % houseNumber);
        address.setText(street % QChar(' ') % houseNumber % QStringLiteral(", ")
                        % address.postalCode()
                        % QChar(' ') % place);


        addStation(StationWithPrice {
                          .id = id,
                          .name = name,
                          .brand = brand,
                          .address = address,
                          .coordinate = QGeoCoordinate(lat, lng),
                          .distance = dist,
                          .isOpen = isOpen,
                          .price = price
                      });
    }

    setFinished();
}

void TankerKoenigPriceReply::onNetworkReplyError()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    reply->deleteLater();

    setError(Error::CommunicationError, reply->errorString());
}


StationDetailsReply *TankerKoenigProvider::stationForId(const QString &id)
{
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("apikey"), m_apiKey);
    query.addQueryItem(QStringLiteral("id"), id);

    QUrl url = DETAILS_URL;
    url.setQuery(query);

    QNetworkRequest request { url };
    if (!m_userAgent.isEmpty()) {
        request.setHeader(
                    QNetworkRequest::KnownHeaders::UserAgentHeader,
                    m_userAgent);
    }
    request.setRawHeader("Accept", "application/json;charset=utf-8");
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);

    qCInfo(logger) << "Start details request for" << id;
    qCDebug(logger) << "Request URL" << url;
    auto* reply = m_network.get(request);

    return new TankerKoenigStationDetailsReply(id, reply);
}

TankerKoenigStationDetailsReply::TankerKoenigStationDetailsReply(
        const QString &stationId, QNetworkReply* reply)
    : StationDetailsReply(stationId)
{
    reply->setReadBufferSize(0);

    connect(reply, &QNetworkReply::finished,
            this, &TankerKoenigStationDetailsReply::onNetworkReplyFinished);
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
            this, &TankerKoenigStationDetailsReply::onNetworkReplyError);
    connect(this, &QObject::destroyed, reply, &QObject::deleteLater);
    connect(this, &FuelPriceReply::aborted, reply, &QNetworkReply::abort);
}

enum WeekDayBit {
    Monday,
    Tuesday,
    Wednesday,
    Thursday,
    Friday,
    Saturday,
    Sunday,
    PublicHoliday
};

static std::optional<WeekDayBit> toDay(const QStringRef& text) {
    if (text.size() == 2) {
        if (text == QStringLiteral("Mo")) {
            return WeekDayBit::Monday;
        } else if (text == QStringLiteral("Di")) {
            return WeekDayBit::Tuesday;
        } else if (text == QStringLiteral("Mi")) {
            return WeekDayBit::Wednesday;
        } else if (text == QStringLiteral("Do")) {
            return WeekDayBit::Thursday;
        } else if (text == QStringLiteral("Fr")) {
            return WeekDayBit::Friday;
        } else if (text == QStringLiteral("Sa")) {
            return WeekDayBit::Saturday;
        } else if (text == QStringLiteral("So")) {
            return WeekDayBit::Sunday;
        }
    }

    if (text == QStringLiteral("Montag")) {
        return WeekDayBit::Monday;
    } else if (text == QStringLiteral("Dienstag")) {
        return WeekDayBit::Tuesday;
    } else if (text == QStringLiteral("Mittwoch")) {
        return WeekDayBit::Wednesday;
    } else if (text == QStringLiteral("Donnerstag")) {
        return WeekDayBit::Thursday;
    } else if (text == QStringLiteral("Freitag")) {
        return WeekDayBit::Friday;
    } else if (text == QStringLiteral("Samstag")) {
        return WeekDayBit::Saturday;
    } else if (text == QStringLiteral("Sonntag")) {
        return WeekDayBit::Sunday;
    } else if (text == QStringLiteral("Feiertag")) {
        return WeekDayBit::PublicHoliday;
    }

    return {};
}

static OpeningTime::WeekDays parseDays(const QString& text) {
    if (text.contains(QChar('-'))) {
        const auto fromTo = text.split(QChar('-'));
        if (fromTo.size() != 2) {
            return {};
        }

        const auto maybeFrom = toDay(&fromTo.at(0));
        const auto maybeTo = toDay(&fromTo.at(1));
        if (maybeFrom.has_value() && maybeTo.has_value()) {
            int days = 0;
            for (int i = maybeFrom.value(); i <= maybeTo.value(); i++) {
                days |= 1 << i;
            }
            return static_cast<OpeningTime::WeekDays>(days);
        } else {
            return {};
        }
    } else if (text == QStringLiteral("täglich ausser Sonn- und Feiertagen")) {
        return OpeningTime::WeekDay::Monday
                | OpeningTime::WeekDay::Tuesday
                | OpeningTime::WeekDay::Wednesday
                | OpeningTime::WeekDay::Thursday
                | OpeningTime::WeekDay::Friday
                | OpeningTime::WeekDay::Saturday;
    } else {
        int days = 0;
        for (auto weekDayStr : text.splitRef(QStringLiteral(", "))) {
            auto const weekDayBit = toDay(weekDayStr);
            if (weekDayBit.has_value()) {
                days |= 1 << weekDayBit.value();
            } else {
                return {};
            }
        }
        return static_cast<OpeningTime::WeekDays>(days);
    }
}

void TankerKoenigStationDetailsReply::onNetworkReplyFinished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        return;
    }

    QJsonParseError error;
    auto jsonDoc = QJsonDocument::fromJson(reply->readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
        setError(Error::ParseError, QStringLiteral("Server error: invalid JSON"));
        return;
    }

    auto root = jsonDoc.object();
    bool ok = root.value(QStringLiteral("ok")).toBool(false);
    if (!ok) {
        auto message = root.value("message").toString("<missing>");
        setError(Error::UnknownError, QStringLiteral("Server error: ") % message);
        return;
    }

    QString status = root.value(QStringLiteral("status")).toString();
    if (status != QStringLiteral("ok")) {
        setError(Error::UnknownError, QStringLiteral("Status error: ") % status);
        return;
    }

    const auto station = root.value(QStringLiteral("station")).toObject();
    qCInfo(logger) << "Got station details for" << stationId();

    const auto id = station.value(QStringLiteral("id")).toString();
    const auto name = station.value(QStringLiteral("name")).toString();
    const auto brand = station.value(QStringLiteral("brand")).toString();
    const auto street = station.value(QStringLiteral("street")).toString();
    const auto houseNumber = station.value(QStringLiteral("houseNumber")).toString();
    const auto postCode = station.value(QStringLiteral("postCode")).toInt();
    const auto place = station.value(QStringLiteral("place")).toString();
    const float lat = station.value(QStringLiteral("lat")).toDouble();
    const float lng = station.value(QStringLiteral("lng")).toDouble();
    const auto isOpen = station.value(QStringLiteral("isOpen")).toBool();
    const auto wholeDay = station.value(QStringLiteral("wholeDay")).toBool();
    const auto openingTimes = station.value(QStringLiteral("openingTimes")).toArray();
    const auto overrides = station.value(QStringLiteral("overrides")).toArray();
    const auto e5 = station.value(QStringLiteral("e5"));
    const auto e10 = station.value(QStringLiteral("e10"));
    const auto diesel = station.value(QStringLiteral("diesel"));

    QVector<OpeningTime> openingTimesList;
    openingTimesList.reserve(openingTimes.size());
    for (auto entry : openingTimes) {
        const auto openingTime = entry.toObject();
        const auto text = openingTime.value(QStringLiteral("text")).toString();
        const auto start = QTime::fromString(
                    openingTime.value(QStringLiteral("start")).toString());
        const auto end = QTime::fromString(
                    openingTime.value(QStringLiteral("end")).toString());

        auto days = parseDays(text);
        if (days == OpeningTime::WeekDay::NoDays) {
            qCWarning(logger)
                    << "Failed to parse opening times date range:"
                    << text;
            continue;
        }

        openingTimesList.push_back(OpeningTime { days, text, start, end });
    }

    QStringList overridesList;
    overridesList.reserve(overrides.size());
    for (auto entry : overrides) {
        overridesList.push_back(entry.toString());
    }

    QHash<FuelPriceProvider::Fuel, float> prices;
    prices.reserve(3);
    if (e5.isDouble()) {
        prices.insert(FuelPriceProvider::Fuel::SuperE5, e5.toDouble());
    }
    if (e10.isDouble()) {
        prices.insert(FuelPriceProvider::Fuel::SuperE10, e10.toDouble());
    }
    if (diesel.isDouble()) {
        prices.insert(FuelPriceProvider::Fuel::Diesel, diesel.toDouble());
    }

    QGeoAddress address;
    address.setCity(place);
    address.setCountryCode(QStringLiteral("de"));
    address.setPostalCode(QString::number(postCode).rightJustified(5, '0'));
    address.setStreet(street % QChar(' ') % houseNumber);
    address.setText(street % QChar(' ') % houseNumber % QStringLiteral(", ")
                    % address.postalCode()
                    % QChar(' ') % place);


    setStationDetails(StationDetails {
                      .id = id,
                      .name = name,
                      .brand = brand,
                      .address = address,
                      .coordinate = QGeoCoordinate(lat, lng),
                      .openingTimes = openingTimesList,
                      .openingTimesOverrides = overridesList,
                      .prices = prices,
                      .isOpen = isOpen,
                      .wholeDay = wholeDay,
                  });

    setFinished();
}

void TankerKoenigStationDetailsReply::onNetworkReplyError()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    reply->deleteLater();

    setError(Error::CommunicationError, reply->errorString());
}
