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

#include "misepriceprovider.h"

#include <algorithm>

#include <QGeoCoordinate>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLoggingCategory>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QStringBuilder>

#include "qtployfill.h"

static Q_LOGGING_CATEGORY(logger, "refuel.mise");

static const QUrl ZONE_SEARCH_URL = QUrl(
    QStringLiteral("https://carburanti.mise.gov.it/ospzApi/search/zone"));

static QString detailsUrl(int id)
{
    return QStringLiteral("https://carburanti.mise.gov.it/ospzApi/registry/servicearea/")
           % QString::number(id);
}

static const int CARB_BENZINA = 1;
static const int CARB_GASOLIO = 2;
static const int CARB_METANO = 3;
static const int CARB_GPL = 4;

static const QString MISE_METANO = QStringLiteral("cng");
static const QString MISE_GPL = QStringLiteral("lpg");

static int fuelIdToCarb(const QString &fuelId)
{
    if (fuelId == FuelPriceProvider::DIESEL) {
        return CARB_GASOLIO;
    } else if (fuelId == MISE_METANO) {
        return CARB_METANO;
    } else if (fuelId == MISE_GPL) {
        return CARB_GPL;
    }
    return CARB_BENZINA;
}

static QString fuelIdToFuelType(const QString &fuelId)
{
    return QString::number(fuelIdToCarb(fuelId)) % QStringLiteral("-x");
}

static QString encodeStationId(int id, double lat, double lng)
{
    return QString::number(id) % QStringLiteral("|") % QString::number(lat, 'f', 6)
           % QStringLiteral("|") % QString::number(lng, 'f', 6);
}

static bool decodeStationId(const QString &encoded, int &id, double &lat, double &lng)
{
    auto parts = encoded.split(QLatin1Char('|'));
    if (parts.size() != 3)
        return false;
    bool ok;
    id = parts[0].toInt(&ok);
    if (!ok)
        return false;
    lat = parts[1].toDouble(&ok);
    if (!ok)
        return false;
    lng = parts[2].toDouble(&ok);
    return ok;
}

static QNetworkReply *postZoneSearch(QNetworkAccessManager &network,
                                     const QGeoCoordinate &coordinate,
                                     double radius,
                                     const QString &fuelType,
                                     const QString &priceOrder,
                                     const QString &userAgent)
{
    QJsonObject point;
    point[QStringLiteral("lat")] = coordinate.latitude();
    point[QStringLiteral("lng")] = coordinate.longitude();

    QJsonArray points;
    points.append(point);

    QJsonObject body;
    body[QStringLiteral("points")] = points;
    body[QStringLiteral("fuelType")] = fuelType;
    body[QStringLiteral("priceOrder")] = priceOrder;
    body[QStringLiteral("radius")] = radius;

    QByteArray data = QJsonDocument(body).toJson(QJsonDocument::Compact);

    QNetworkRequest request{ZONE_SEARCH_URL};
    if (!userAgent.isEmpty()) {
        request.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader, userAgent);
    }
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    request.setRawHeader("Accept", "application/json");
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);

    qCDebug(logger) << "Zone search body" << data;

    return network.post(request, data);
}

static QNetworkReply *getStationDetails(QNetworkAccessManager &network,
                                        int stationId,
                                        const QString &userAgent)
{
    QUrl url(detailsUrl(stationId));
    QNetworkRequest request{url};
    if (!userAgent.isEmpty()) {
        request.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader, userAgent);
    }
    request.setRawHeader("Accept", "application/json");
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);

    qCDebug(logger) << "Details request" << url;

    return network.get(request);
}

static QGeoAddress parseDetailsAddress(const QString &addr)
{
    QGeoAddress address;
    address.setCountryCode(QStringLiteral("it"));
    address.setText(addr);

    static QRegularExpression re(
        QStringLiteral(R"(^(.*)\s+(\d{5})\s*[-–]\s*(.*?)\s*\(([A-Z]{1,2})\)$)"));
    auto match = re.match(addr.trimmed());
    if (match.hasMatch()) {
        address.setStreet(match.captured(1).trimmed());
        address.setPostalCode(match.captured(2));
        address.setCity(match.captured(3).trimmed());
    }

    return address;
}

static OpeningTime::WeekDays giornoToWeekDay(int giorno)
{
    switch (giorno) {
    case 1:
        return OpeningTime::Monday;
    case 2:
        return OpeningTime::Tuesday;
    case 3:
        return OpeningTime::Wednesday;
    case 4:
        return OpeningTime::Thursday;
    case 5:
        return OpeningTime::Friday;
    case 6:
        return OpeningTime::Saturday;
    case 7:
        return OpeningTime::Sunday;
    case 8:
        return OpeningTime::PublicHoliday;
    default:
        return OpeningTime::NoDays;
    }
}

static QString giornoName(int giorno)
{
    switch (giorno) {
    case 1:
        return QStringLiteral("Lunedì");
    case 2:
        return QStringLiteral("Martedì");
    case 3:
        return QStringLiteral("Mercoledì");
    case 4:
        return QStringLiteral("Giovedì");
    case 5:
        return QStringLiteral("Venerdì");
    case 6:
        return QStringLiteral("Sabato");
    case 7:
        return QStringLiteral("Domenica");
    case 8:
        return QStringLiteral("Festivo");
    default:
        return QString();
    }
}

static QVector<OpeningTime> parseOpeningTimes(const QJsonArray &orari)
{
    QVector<OpeningTime> result;

    for (auto entry : orari) {
        auto obj = entry.toObject();
        int giorno = obj.value(QStringLiteral("giornoSettimanaId")).toInt();
        auto weekDay = giornoToWeekDay(giorno);
        if (weekDay == OpeningTime::NoDays)
            continue;

        bool h24 = obj.value(QStringLiteral("flagH24")).toBool();
        bool chiusura = obj.value(QStringLiteral("flagChiusura")).toBool();
        bool nonComunicato = obj.value(QStringLiteral("flagNonComunicato")).toBool();
        bool continuato = obj.value(QStringLiteral("flagOrarioContinuato")).toBool();

        if (chiusura || nonComunicato)
            continue;

        if (h24) {
            result.push_back(OpeningTime(weekDay, giornoName(giorno), QTime(0, 0), QTime(23, 59)));
            continue;
        }

        if (continuato) {
            auto startStr = obj.value(QStringLiteral("oraAperturaOrarioContinuato")).toString();
            auto endStr = obj.value(QStringLiteral("oraChiusuraOrarioContinuato")).toString();
            QTime start = QTime::fromString(startStr, QStringLiteral("hh:mm"));
            QTime end = QTime::fromString(endStr, QStringLiteral("hh:mm"));
            if (start.isValid() && end.isValid()) {
                if (end == QTime(0, 0))
                    end = QTime(23, 59);
                result.push_back(OpeningTime(weekDay, giornoName(giorno), start, end));
            }
        } else {
            auto amStartStr = obj.value(QStringLiteral("oraAperturaMattina")).toString();
            auto amEndStr = obj.value(QStringLiteral("oraChiusuraMattina")).toString();
            auto pmStartStr = obj.value(QStringLiteral("oraAperturaPomeriggio")).toString();
            auto pmEndStr = obj.value(QStringLiteral("oraChiusuraPomeriggio")).toString();

            QTime amStart = QTime::fromString(amStartStr, QStringLiteral("hh:mm"));
            QTime amEnd = QTime::fromString(amEndStr, QStringLiteral("hh:mm"));
            QTime pmStart = QTime::fromString(pmStartStr, QStringLiteral("hh:mm"));
            QTime pmEnd = QTime::fromString(pmEndStr, QStringLiteral("hh:mm"));

            if (amStart.isValid() && amEnd.isValid()) {
                if (amEnd == QTime(0, 0))
                    amEnd = QTime(23, 59);
                result.push_back(OpeningTime(weekDay,
                                             giornoName(giorno) % QStringLiteral(" mattina"),
                                             amStart,
                                             amEnd));
            }
            if (pmStart.isValid() && pmEnd.isValid()) {
                if (pmEnd == QTime(0, 0))
                    pmEnd = QTime(23, 59);
                result.push_back(OpeningTime(weekDay,
                                             giornoName(giorno) % QStringLiteral(" pomeriggio"),
                                             pmStart,
                                             pmEnd));
            }
        }
    }

    return result;
}

static QHash<QString, float> parseFuelPrices(const QJsonArray &fuels)
{
    QHash<QString, float> prices;
    for (auto f : fuels) {
        auto fuelObj = f.toObject();
        int fuelId = fuelObj.value(QStringLiteral("fuelId")).toInt(-1);
        float price = static_cast<float>(fuelObj.value(QStringLiteral("price")).toDouble());
        if (fuelId < 0 || price <= 0)
            continue;

        QString fuelKey;
        if (fuelId == CARB_BENZINA) {
            fuelKey = FuelPriceProvider::GASOLINE_95_E5;
        } else if (fuelId == CARB_GASOLIO) {
            fuelKey = FuelPriceProvider::DIESEL;
        } else if (fuelId == CARB_METANO) {
            fuelKey = MISE_METANO;
        } else if (fuelId == CARB_GPL) {
            fuelKey = MISE_GPL;
        } else {
            continue;
        }

        if (!prices.contains(fuelKey) || price < prices[fuelKey]) {
            prices[fuelKey] = price;
        }
    }
    return prices;
}

MisePriceProvider::MisePriceProvider(QObject *parent)
    : FuelPriceProvider(parent)
    , m_network()
{}

QGeoRectangle MisePriceProvider::boundingBox() const
{
    // Data © OpenStreetMap contributors, ODbL 1.0. https://osm.org/copyright
    // > curl
    // "https://nominatim.openstreetmap.org/search?q=italy&format=json&limit=1"
    //   | jq '.[0].boundingbox'
    return QGeoRectangle(QGeoCoordinate(35.2889616, 6.6272658),
                         QGeoCoordinate(47.0921462, 18.7844746));
}

QString MisePriceProvider::copyright() const
{
    return "Data: MISE - "
           "<a href=\"https://carburanti.mise.gov.it\">Osservaprezzi "
           "Carburanti</a>";
}

QStringList MisePriceProvider::fuels() const
{
    return {GASOLINE_95_E5, DIESEL, MISE_METANO, MISE_GPL};
}

QString MisePriceProvider::fuelName(const QString &fuelId)
{
    if (fuelId == MISE_METANO) {
        return tr("Metano (CNG)");
    } else if (fuelId == MISE_GPL) {
        return tr("GPL (LPG)");
    }
    return FuelPriceProvider::fuelName(fuelId);
}

void MisePriceProvider::setUserAgent(const QString &value)
{
    if (m_userAgent != value) {
        m_userAgent = value;
        emit userAgentChanged();
    }
}

FuelPriceReply *MisePriceProvider::list(const QGeoCoordinate &coordinate,
                                        double radius,
                                        const QString &fuelId,
                                        Sorting sorting)
{
    QString fuelType = fuelIdToFuelType(fuelId);
    QString priceOrder = (sorting == Sorting::Price) ? QStringLiteral("asc")
                                                     : QStringLiteral("asc");

    qCInfo(logger) << "Start list request radius=" << radius;
    auto *reply = postZoneSearch(m_network, coordinate, radius, fuelType, priceOrder, m_userAgent);

    return new MisePriceReply(coordinate, radius, fuelId, sorting, reply);
}

MisePriceReply::MisePriceReply(const QGeoCoordinate &coordinate,
                               double radius,
                               const QString &fuelId,
                               FuelPriceProvider::Sorting sorting,
                               QNetworkReply *reply)
    : FuelPriceReply(coordinate, radius, fuelId, sorting)
{
    reply->setReadBufferSize(0);

    connect(reply, &QNetworkReply::finished, this, &MisePriceReply::onNetworkReplyFinished);
    connect(reply,
            QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
            this,
            &MisePriceReply::onNetworkReplyError);
    connect(this, &QObject::destroyed, reply, &QObject::deleteLater);
    connect(this, &FuelPriceReply::aborted, reply, &QNetworkReply::abort);
}

void MisePriceReply::onNetworkReplyFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
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
    bool success = root.value(QStringLiteral("success")).toBool(false);
    if (!success) {
        setError(Error::UnknownError, QStringLiteral("API returned success=false"));
        return;
    }

    const auto results = root.value(QStringLiteral("results")).toArray();
    qCInfo(logger) << "Got list result with" << results.size() << "items";

    int requestedCarb = fuelIdToCarb(fuelId());

    for (auto station : results) {
        auto obj = station.toObject();
        int id = obj.value(QStringLiteral("id")).toInt(-1);
        if (id < 0)
            continue;

        auto name = obj.value(QStringLiteral("name")).toString();
        auto brand = obj.value(QStringLiteral("brand")).toString();
        auto location = obj.value(QStringLiteral("location")).toObject();
        double lat = location.value(QStringLiteral("lat")).toDouble();
        double lng = location.value(QStringLiteral("lng")).toDouble();
        double dist = obj.value(QStringLiteral("distance")).toString().toDouble();

        auto fuels = obj.value(QStringLiteral("fuels")).toArray();
        float bestPrice = -1.0f;
        for (auto f : fuels) {
            auto fuelObj = f.toObject();
            int fuelId = fuelObj.value(QStringLiteral("fuelId")).toInt(-1);
            if (fuelId == requestedCarb) {
                float price = static_cast<float>(fuelObj.value(QStringLiteral("price")).toDouble());
                if (price > 0 && (bestPrice < 0 || price < bestPrice)) {
                    bestPrice = price;
                }
            }
        }

        if (bestPrice < 0)
            continue;

        addStation(StationWithPrice{.id = encodeStationId(id, lat, lng),
                                    .name = !name.isEmpty() ? name : brand,
                                    .brand = brand,
                                    .address = QGeoAddress(),
                                    .coordinate = QGeoCoordinate(lat, lng),
                                    .distance = dist,
                                    .isOpen = true,
                                    .price = bestPrice});
    }

    if (sorting() == FuelPriceProvider::Sorting::Distance) {
        auto sorted = stations();
        std::sort(sorted.begin(),
                  sorted.end(),
                  [](const StationWithPrice &a, const StationWithPrice &b) {
                      return a.distance < b.distance;
                  });
        setStations(sorted);
    }

    setFinished();
}

void MisePriceReply::onNetworkReplyError()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    reply->deleteLater();

    setError(Error::CommunicationError, reply->errorString());
}

StationDetailsReply *MisePriceProvider::stationForId(const QString &id)
{
    int intId;
    double lat, lng;
    if (!decodeStationId(id, intId, lat, lng)) {
        qCWarning(logger) << "Invalid station ID format:" << id;
        auto *reply = new MiseStationDetailsReply(id, QGeoCoordinate());
        QMetaObject::invokeMethod(reply, "reportError", Qt::QueuedConnection);
        return reply;
    }

    qCInfo(logger) << "Start details request for" << id;
    auto *netReply = getStationDetails(m_network, intId, m_userAgent);

    return new MiseStationDetailsReply(id, QGeoCoordinate(lat, lng), netReply);
}

MiseStationDetailsReply::MiseStationDetailsReply(const QString &stationId,
                                                 const QGeoCoordinate &coordinate,
                                                 QNetworkReply *reply)
    : StationDetailsReply(stationId)
    , m_coordinate(coordinate)
{
    if (!reply) {
        QMetaObject::invokeMethod(this, "reportError", Qt::QueuedConnection);
        return;
    }

    reply->setReadBufferSize(0);

    connect(reply, &QNetworkReply::finished, this, &MiseStationDetailsReply::onNetworkReplyFinished);
    connect(reply,
            QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
            this,
            &MiseStationDetailsReply::onNetworkReplyError);
    connect(this, &QObject::destroyed, reply, &QObject::deleteLater);
    connect(this, &FuelPriceReply::aborted, reply, &QNetworkReply::abort);
}

void MiseStationDetailsReply::reportError()
{
    setError(Error::ParseError, QStringLiteral("Invalid station ID"));
}

void MiseStationDetailsReply::onNetworkReplyFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
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

    auto stationObj = jsonDoc.object();
    qCInfo(logger) << "Got station details for" << stationId();

    auto name = stationObj.value(QStringLiteral("name")).toString();
    auto brand = stationObj.value(QStringLiteral("brand")).toString();
    auto addr = stationObj.value(QStringLiteral("address")).toString();
    auto fuels = stationObj.value(QStringLiteral("fuels")).toArray();
    auto orari = stationObj.value(QStringLiteral("orariapertura")).toArray();

    auto prices = parseFuelPrices(fuels);
    auto openingTimes = parseOpeningTimes(orari);

    bool isH24 = false;
    for (auto o : openingTimes) {
        if (o.start() == QTime(0, 0) && o.end() == QTime(23, 59)) {
            isH24 = true;
            break;
        }
    }

    QGeoAddress address = parseDetailsAddress(addr);

    setStationDetails(StationDetails{
        .id = stationId(),
        .name = !name.isEmpty() ? name : brand,
        .brand = brand,
        .address = address,
        .coordinate = m_coordinate,
        .openingTimes = openingTimes,
        .openingTimesOverrides = {},
        .prices = prices,
        .isOpen = true,
        .wholeDay = isH24,
    });

    setFinished();
}

void MiseStationDetailsReply::onNetworkReplyError()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    reply->deleteLater();

    setError(Error::CommunicationError, reply->errorString());
}

StationUpdatesReply *MisePriceProvider::pricesForStations(const QStringList &ids)
{
    return new MiseStationUpdatesReply(ids, &m_network, m_userAgent);
}

MiseStationUpdatesReply::MiseStationUpdatesReply(const QStringList &stationIds,
                                                 QNetworkAccessManager *network,
                                                 const QString &userAgent)
    : StationUpdatesReply(stationIds)
{
    if (stationIds.isEmpty()) {
        QMetaObject::invokeMethod(this, "reportEmpty", Qt::QueuedConnection);
        return;
    }

    m_pendingReplies.reserve(stationIds.size());

    for (const auto &id : stationIds) {
        int intId;
        double lat, lng;
        if (!decodeStationId(id, intId, lat, lng)) {
            qCWarning(logger) << "Invalid station ID format for update:" << id;
            continue;
        }

        auto *reply = getStationDetails(*network, intId, userAgent);
        reply->setProperty("stationId", id);

        m_pendingReplies.append(reply);
        connect(reply,
                &QNetworkReply::finished,
                this,
                &MiseStationUpdatesReply::onNetworkReplyFinished);
        connect(reply,
                QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
                this,
                &MiseStationUpdatesReply::onNetworkReplyError);
        connect(this, &QObject::destroyed, reply, &QObject::deleteLater);
    }
}

MiseStationUpdatesReply::~MiseStationUpdatesReply()
{
    for (auto *reply : m_pendingReplies) {
        reply->abort();
        reply->deleteLater();
    }
}

void MiseStationUpdatesReply::reportEmpty()
{
    setFinished();
}

void MiseStationUpdatesReply::onNetworkReplyFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    m_pendingReplies.removeOne(reply);
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        checkAllFinished();
        return;
    }

    QJsonParseError error;
    auto jsonDoc = QJsonDocument::fromJson(reply->readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
        checkAllFinished();
        return;
    }

    auto stationObj = jsonDoc.object();

    QString encodedId = reply->property("stationId").toString();
    auto fuels = stationObj.value(QStringLiteral("fuels")).toArray();
    auto prices = parseFuelPrices(fuels);

    addStationUpdate(StationUpdate{.id = encodedId, .prices = prices, .isOpen = true});

    checkAllFinished();
}

void MiseStationUpdatesReply::onNetworkReplyError()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    m_pendingReplies.removeOne(reply);
    reply->deleteLater();

    checkAllFinished();
}

void MiseStationUpdatesReply::checkAllFinished()
{
    if (m_pendingReplies.isEmpty()) {
        setFinished();
    }
}
