#include "tankerkoenigapirequest.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLoggingCategory>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QStringBuilder>
#include <QGeoCoordinate>

#include "qtployfill.h"

static Q_LOGGING_CATEGORY(logger, "refuel.tankerkoenig");

static const QUrl LIST_URL = QUrl(QStringLiteral("https://creativecommons.tankerkoenig.de/json/list.php"));


TankerKoenigProvider::TankerKoenigProvider(QObject *parent)
    : FuelPriceProvider(parent)
    , m_network()
    , m_apiKey(QStringLiteral("00000000-0000-0000-0000-000000000002"))
{
}

QGeoRectangle TankerKoenigProvider::boundingBox() const
{
    // Data © OpenStreetMap contributors, ODbL 1.0. https://osm.org/copyright
    // > curl "https://nominatim.openstreetmap.org/search?q=germany&format=json&limit=1"
    //   | jq '.[0].boundingbox'
    return QGeoRectangle(
                QGeoCoordinate(5.8663153, 47.2701114),
                QGeoCoordinate(15.0419309, 55.099161));
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
