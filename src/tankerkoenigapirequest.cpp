/*
 * Copyright 2025 Richard Liebscher <r1tschy@posteo.de>.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
 
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
static const QUrl PRICES_URL = QUrl(QStringLiteral("https://creativecommons.tankerkoenig.de/json/prices.php"));

static const QString API_KEY = charSeqToQString(
            make_char_sequence<TankerKoenigApiKey>{});

static QString capitalizeEachWord(const QString& input) {
    QString result = input.toLower();
    bool newWord = true;
    for (int i = 0; i < result.length(); ++i) {
        QChar c = result[i];
        if (c.isSpace() || c.isPunct()) {
            newWord = true;
        } else if (newWord) {
            if (!c.isUpper()) {
                result[i] = c.toUpper();
            }
            newWord = false;
        }
    }
    return result;
}

static QGeoAddress createAddress(const QString& street, const QString& houseNumber, int postCode, const QString& place) {
    QGeoAddress address;
    QString place_ = capitalizeEachWord(place);

    address.setCity(capitalizeEachWord(place_));
    address.setCountryCode(QStringLiteral("de"));
    address.setPostalCode(QString::number(postCode).rightJustified(5, '0'));
    if (houseNumber.trimmed().isEmpty()) {
        address.setStreet(capitalizeEachWord(street));
    } else {
        address.setStreet(capitalizeEachWord(street) % QChar(' ') % houseNumber);
    }
    address.setText(address.street() % QStringLiteral(", ") % address.postalCode() % QChar(' ') % place_);
    return address;
}

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

QString TankerKoenigProvider::copyright() const
{
    return tr("Licence: CC BY 4.0 - <a href=\"https://creativecommons.tankerkoenig.de\">Tankerkönig</a><br>"
            "Data: MTS-K");
}

QStringList TankerKoenigProvider::fuels() const
{
    return { GASOLINE_95_E10, GASOLINE_95_E5, DIESEL };
}

void TankerKoenigProvider::setUserAgent(const QString &value)
{
    if (m_userAgent != value) {
        m_userAgent = value;
        emit userAgentChanged();
    }
}

FuelPriceReply *TankerKoenigProvider::list(const QGeoCoordinate &coordinate, double radius, const QString &fuelId,
        Sorting sorting)
{
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("apikey"), m_apiKey);
    query.addQueryItem(QStringLiteral("lat"), QString::number(coordinate.latitude()));
    query.addQueryItem(QStringLiteral("lng"), QString::number(coordinate.longitude()));
    query.addQueryItem(QStringLiteral("rad"), QString::number(radius));

    QString fuelStr;
    if (fuelId == FuelPriceProvider::GASOLINE_95_E5) {
        fuelStr = QStringLiteral("e5");
    } else if (fuelId == FuelPriceProvider::GASOLINE_95_E10) {
        fuelStr = QStringLiteral("e10");
    } else if (fuelId == FuelPriceProvider::DIESEL) {
        fuelStr = QStringLiteral("diesel");
    } else {
        qWarning() << "Unsupported fuel ID" << fuelId;
        fuelStr = QStringLiteral("e10");
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
                coordinate, radius, fuelId, sorting, reply);
}

TankerKoenigPriceReply::TankerKoenigPriceReply(const QGeoCoordinate& coordinate, double radius,
        const QString &fuelId, FuelPriceProvider::Sorting sorting,
        QNetworkReply *reply)
    : FuelPriceReply(coordinate, radius, fuelId, sorting)
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

        QGeoAddress address = createAddress(street, houseNumber, postCode, place);

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

StationUpdatesReply *TankerKoenigProvider::pricesForStations(const QStringList &ids)
{
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("apikey"), m_apiKey);
    query.addQueryItem(QStringLiteral("ids"), ids.join(','));

    QUrl url = PRICES_URL;
    url.setQuery(query);

    QNetworkRequest request { url };
    if (!m_userAgent.isEmpty()) {
        request.setHeader(
                    QNetworkRequest::KnownHeaders::UserAgentHeader,
                    m_userAgent);
    }
    request.setRawHeader("Accept", "application/json;charset=utf-8");
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);

    qCInfo(logger) << "Start prices request for" << ids;
    qCDebug(logger) << "Request URL" << url;
    auto* reply = m_network.get(request);

    return new TankerKoenigStationUpdatesReply(ids, reply);
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
    if (text == QStringLiteral("Mo-Fr")) {
        return { (1 << 5) - 1 };
    } else if (text == QStringLiteral("täglich")) {
        return { (1 << 8) - 1 };
    } else if (text == QStringLiteral("täglich ausser Sonn- und Feiertagen")) {
        return { (1 << 6) - 1 };
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
        return { days };
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
    const auto street = capitalizeEachWord(station.value(QStringLiteral("street")).toString());
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
        auto end = QTime::fromString(
                    openingTime.value(QStringLiteral("end")).toString());

        if (end == QTime(0, 0)) {
            end = QTime(23, 59, 00);
        }

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

    QHash<QString, float> prices;
    prices.reserve(3);
    if (e5.isDouble()) {
        prices.insert(FuelPriceProvider::GASOLINE_95_E5, e5.toDouble());
    }
    if (e10.isDouble()) {
        prices.insert(FuelPriceProvider::GASOLINE_95_E10, e10.toDouble());
    }
    if (diesel.isDouble()) {
        prices.insert(FuelPriceProvider::DIESEL, diesel.toDouble());
    }

    QGeoAddress address = createAddress(street, houseNumber, postCode, place);

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

TankerKoenigStationUpdatesReply::TankerKoenigStationUpdatesReply(
        const QStringList &stationIds, QNetworkReply *reply)
    : StationUpdatesReply(stationIds)
{
    reply->setReadBufferSize(0);

    connect(reply, &QNetworkReply::finished,
            this, &TankerKoenigStationUpdatesReply::onNetworkReplyFinished);
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
            this, &TankerKoenigStationUpdatesReply::onNetworkReplyError);
    connect(this, &QObject::destroyed, reply, &QObject::deleteLater);
    connect(this, &FuelPriceReply::aborted, reply, &QNetworkReply::abort);
}

void TankerKoenigStationUpdatesReply::onNetworkReplyFinished()
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

    const auto prices = root.value(QStringLiteral("prices")).toObject();
    for (auto iter = prices.begin(); iter != prices.end(); ++iter) {
        const auto id = iter.key();
        const auto value = iter.value().toObject();
        const auto status = value.value(QStringLiteral("status")).toString();
        const auto e5 = value.value(QStringLiteral("e5"));
        const auto e10 = value.value(QStringLiteral("e10"));
        const auto diesel = value.value(QStringLiteral("diesel"));

        QHash<QString, float> prices;
        prices.reserve(3);
        if (e5.isDouble()) {
            prices.insert(FuelPriceProvider::GASOLINE_95_E5, e5.toDouble());
        }
        if (e10.isDouble()) {
            prices.insert(FuelPriceProvider::GASOLINE_95_E10, e10.toDouble());
        }
        if (diesel.isDouble()) {
            prices.insert(FuelPriceProvider::DIESEL, diesel.toDouble());
        }

        addStationUpdate(StationUpdate {
                         .id = id,
                         .prices = prices,
                         .isOpen = status == QStringLiteral("open")
                    });
    }

    setFinished();
}

void TankerKoenigStationUpdatesReply::onNetworkReplyError()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    reply->deleteLater();

    setError(Error::CommunicationError, reply->errorString());
}
