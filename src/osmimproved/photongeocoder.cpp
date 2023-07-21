#include "photongeocoder.h"

#include <QGeoAddress>
#include <QUrlQuery>
#include <QGeoRectangle>
#include <QGeoShape>
#include <QStringBuilder>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "../qtployfill.h"

static const QString PHOTON_URL = QStringLiteral("https://photon.komoot.io/api/");

static QString bboxFromShape(const QGeoRectangle& bounds) {
    return QString::number(bounds.topLeft().longitude()) % QChar(',')
            % QString::number(bounds.topLeft().latitude()) % QChar(',')
            % QString::number(bounds.bottomRight().longitude()) % QChar(',')
            % QString::number(bounds.bottomRight().latitude());
}

PhotonGeoCodingManagerEngine::PhotonGeoCodingManagerEngine(const QVariantMap &parameters, QGeoServiceProvider::Error *error, QString *errorString)
    : QGeoCodingManagerEngine(parameters)
    , m_baseUrl(parameters.value(QStringLiteral("url"), PHOTON_URL).toString())
    , m_userAgent(parameters.value(QStringLiteral("useragent"), "Refuel").toString().toLatin1())
{
    if (m_userAgent.isEmpty()) {
        *error = QGeoServiceProvider::MissingRequiredParameterError;
        *errorString = QStringLiteral("useragent parameter is required");
        // TODO: return;
    }

    *error = QGeoServiceProvider::NoError;
    errorString->clear();
}

PhotonGeoCodeReply *PhotonGeoCodingManagerEngine::geocode(const QGeoAddress &address, const QGeoShape &bounds)
{
    return geocode(address.text(), -1, -1, bounds);
}


PhotonGeoCodeReply *PhotonGeoCodingManagerEngine::geocode(const QString &address, int limit, int offset, const QGeoShape &bounds)
{
    QNetworkRequest request;
    request.setRawHeader("user-agent", m_userAgent);

    QUrl url { QStringLiteral("https://photon.komoot.io/api/") };
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("q"), address);
    if (locale() != QLocale::c()) {
        query.addQueryItem(QStringLiteral("lang"), locale().name().left(2));
    }
    if (limit != -1) {
       query.addQueryItem(QStringLiteral("limit"), QString::number(limit));
    }
    if (bounds.type() == QGeoShape::RectangleType) {
        query.addQueryItem(QStringLiteral("bbox"), bboxFromShape(bounds));
    }
    url.setQuery(query);
    request.setUrl(url);

    auto* reply = m_networkManager.get(request);
    reply->setReadBufferSize(0);

    PhotonGeoCodeReply* geocodeReply =
            new PhotonGeoCodeReply(reply, limit, this);

    connect(geocodeReply, &PhotonGeoCodeReply::finished,
            this, [this]() { emit finished(static_cast<QGeoCodeReply*>(sender())); });
    connect(geocodeReply, QOverload<QGeoCodeReply::Error, const QString&>::of(&PhotonGeoCodeReply::error),
            this, [this](QGeoCodeReply::Error errorCode, const QString& errorString) {
        emit error(static_cast<QGeoCodeReply*>(sender()), errorCode, errorString);
    });

    return geocodeReply;
}

QGeoCodeReply *PhotonGeoCodingManagerEngine::reverseGeocode(const QGeoCoordinate &coordinate, const QGeoShape &bounds)
{
    return nullptr;
}

PhotonGeoCodeReply::PhotonGeoCodeReply(QNetworkReply *reply, int limit, QObject *parent)
:   QGeoCodeReply(parent)
{
    Q_ASSERT(reply != nullptr);
    setLimit(limit);
    setOffset(0);
    connect(
                reply, &QNetworkReply::finished,
                this, &PhotonGeoCodeReply::onNetworkReplyFinished);
    connect(
                reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
                this, &PhotonGeoCodeReply::onNetworkReplyError);
    connect(this, &QGeoCodeReply::abort, reply, &QNetworkReply::abort);
    connect(this, &QObject::destroyed, reply, &QObject::deleteLater);
}

void PhotonGeoCodeReply::onNetworkReplyFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        return;
    }

    QList<QGeoLocation> result;
    const QJsonDocument document = QJsonDocument::fromJson(reply->readAll());
    const QJsonObject featureCollection = document.object();
    const QJsonArray features = featureCollection.value(QStringLiteral("features")).toArray();
    for (auto feature : features) {
        auto featureObj = feature.toObject();

        auto geometry = featureObj.value(QStringLiteral("geometry")).toObject();
        auto type = geometry.value(QStringLiteral("type")).toString();
        if (type == QStringLiteral("Point")) {
            QGeoLocation location;
            QGeoAddress address;

            auto properties = featureObj.value(QStringLiteral("properties")).toObject();
            address.setCountryCode(properties.value(QStringLiteral("countrycode")).toString());
            address.setCountry(properties.value(QStringLiteral("country")).toString());
            address.setCounty(properties.value(QStringLiteral("county")).toString());
            address.setState(properties.value(QStringLiteral("state")).toString());
            address.setCity(properties.value(QStringLiteral("city")).toString());
            address.setDistrict(properties.value(QStringLiteral("district")).toString());
            address.setPostalCode(properties.value(QStringLiteral("postcode")).toString());
            address.setStreet(properties.value(QStringLiteral("street")).toString());
            address.setText(properties.value(QStringLiteral("name")).toString());
            // TODO: extend property
            location.setAddress(address);

            auto coordinates = geometry.value(QStringLiteral("coordinates")).toArray();
            location.setCoordinate(QGeoCoordinate {
                coordinates.at(1).toDouble(), coordinates.at(0).toDouble() });

            result.append(location);
        }
    }

    setLocations(result);
    setFinished(true);
}

void PhotonGeoCodeReply::onNetworkReplyError(QNetworkReply::NetworkError error)
{
    QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
    reply->deleteLater();

    setError(QGeoCodeReply::CommunicationError, reply->errorString());
}

