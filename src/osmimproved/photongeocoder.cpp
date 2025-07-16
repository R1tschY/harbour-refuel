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
    , m_baseUrl(parameters.value(QStringLiteral("osmimproved.photon.url"), PHOTON_URL).toString())
    , m_userAgent(parameters.value(QStringLiteral("osmimproved.useragent")).toString().toLatin1())
{
    if (m_userAgent.isEmpty()) {
        *error = QGeoServiceProvider::MissingRequiredParameterError;
        *errorString = QStringLiteral("'osmimproved.useragent' parameter is required");
        return;
    }

    *error = QGeoServiceProvider::NoError;
    errorString->clear();
}

PhotonGeoCodingManagerEngine::~PhotonGeoCodingManagerEngine() = default;

PhotonGeoCodeReply *PhotonGeoCodingManagerEngine::geocode(const QGeoAddress &address, const QGeoShape &bounds)
{
    return geocode(address.text(), -1, -1, bounds);
}

PhotonGeoCodeReply *PhotonGeoCodingManagerEngine::geocode(const QString &address, int limit, int offset, const QGeoShape &bounds)
{
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

    QUrl url { m_baseUrl };
    url.setQuery(query);

    QNetworkRequest request;
    request.setUrl(url);
    request.setRawHeader("user-agent", m_userAgent);
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);

    auto* reply = m_networkManager.get(request);
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
    : QGeoCodeReply(parent)
    , m_reply(reply)
{
    Q_ASSERT(reply != nullptr);

    reply->setReadBufferSize(0);

    setLimit(limit);
    setOffset(0);
    connect(reply, &QNetworkReply::finished,
            this, &PhotonGeoCodeReply::onNetworkReplyFinished);
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
            this, &PhotonGeoCodeReply::onNetworkReplyError);
}

PhotonGeoCodeReply::~PhotonGeoCodeReply()
{
    if (m_reply) {
        delete m_reply;
    }
}

void PhotonGeoCodeReply::onNetworkReplyFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
    reply->deleteLater();
    m_reply = nullptr;

    if (reply->error() != QNetworkReply::NoError) {
        return;
    }

    QList<QGeoLocation> result;
    QJsonParseError jsonError;
    const QJsonDocument document = QJsonDocument::fromJson(reply->readAll(), &jsonError);
    if (jsonError.error != QJsonParseError::NoError) {
        setError(QGeoCodeReply::ParseError,
                 QStringLiteral("Invalid JSON reply: ") % jsonError.errorString());
        return;
    }

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
    m_reply = nullptr;

    setError(QGeoCodeReply::CommunicationError, reply->errorString());
}

void PhotonGeoCodeReply::abort()
{
    if (m_reply) {
        m_reply->abort();
    }
}
