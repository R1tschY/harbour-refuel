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
 
#include "osmtiledmappingmanagerengine.h"

#include <private/qgeocameracapabilities_p.h>
#include <private/qgeomaptype_p.h>
#include <private/qgeotiledmapdata_p.h>
#include <private/qgeotilespec_p.h>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QLoggingCategory>
#include <QStringBuilder>

#include "../qtployfill.h"

static Q_LOGGING_CATEGORY(logger, "refuel.tankerkoenig");


OsmTiledMappingManagerEngine::OsmTiledMappingManagerEngine(const QVariantMap& parameters,
        QGeoServiceProvider::Error* error, QString* errorString, QObject* parent)
    : QGeoTiledMappingManagerEngine(parent)
{
    QGeoCameraCapabilities cameraCaps;
    cameraCaps.setMinimumZoomLevel(0.0);
    cameraCaps.setMaximumZoomLevel(18.0);
    setCameraCapabilities(cameraCaps);

    setTileSize(QSize(256, 256));

    QList<QGeoMapType> mapTypes {
        QGeoMapType(QGeoMapType::StreetMap,
                    tr("Street Map"),
                    tr("OpenStreetMap Standard tile layer"),
                    false, false, 1),
                QGeoMapType(QGeoMapType::StreetMap,
                            tr("Street Map"),
                            tr("OpenStreetMap German tile layer"),
                            false, false, 2)
    };


    auto userAgent = parameters.value(QStringLiteral("osmimproved.useragent"))
            .toString().toLatin1();
    if (userAgent.isEmpty()) {
        *error = QGeoServiceProvider::MissingRequiredParameterError;
        *errorString = QStringLiteral("useragent parameter missing");
        return;
    }
    setTileFetcher(new OsmTileFetcher(userAgent, this));

    *error = QGeoServiceProvider::NoError;
    errorString->clear();
}

OsmTiledMappingManagerEngine::~OsmTiledMappingManagerEngine() = default;

QGeoMapData *OsmTiledMappingManagerEngine::createMapData()
{
    return new QGeoTiledMapData(this, nullptr);
}

OsmTileFetcher::OsmTileFetcher(const QByteArray& userAgent, QObject* parent)
    : QGeoTileFetcher(parent)
    , m_userAgent(userAgent)
{

}

OsmTileFetcher::~OsmTileFetcher() = default;

static QString buildStdUrl(const QString& base, const QGeoTileSpec& spec) {
    return base
            % '/' % QString::number(spec.zoom())
            % '/' % QString::number(spec.x())
            % '/' % QString::number(spec.y())
            % QStringLiteral(".png");
}

QGeoTiledMapReply *OsmTileFetcher::getTileImage(const QGeoTileSpec& spec)
{
    QNetworkRequest request;
    request.setRawHeader("User-Agent", m_userAgent);

    QString url;
    switch (spec.mapId()) {
    default:
    case 1:
        url = buildStdUrl(QStringLiteral("https://tile.openstreetmap.org"), spec);
        break;
    case 2:
        url = buildStdUrl(QStringLiteral("https://tile.openstreetmap.de"), spec);
        break;
    }

    request.setUrl(QUrl::fromUserInput(url));

    qCDebug(logger) << "Request GET" << request.url();

    auto* reply = m_networkManager.get(request);
    return new OsmTileReply(reply, spec, this);
}

OsmTileReply::OsmTileReply(QNetworkReply *reply, const QGeoTileSpec &spec, QObject *parent)
    : QGeoTiledMapReply(spec, parent)
    , m_reply(reply)
{
    Q_ASSERT(reply != nullptr);

    reply->setReadBufferSize(0);

    connect(reply, &QNetworkReply::finished,
            this, &OsmTileReply::onNetworkReplyFinished);
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
            this, &OsmTileReply::onNetworkReplyError);
}

void OsmTileReply::abort()
{
    if (m_reply) {
        m_reply->abort();
    }
}

OsmTileReply::~OsmTileReply() = default;

void OsmTileReply::onNetworkReplyFinished()
{
    QNetworkReply *reply = m_reply;
    if (!reply) {
        return;
    }

    reply->deleteLater();
    m_reply = nullptr;

    if (reply->error() != QNetworkReply::NoError) {
        return;
    }

    auto image = reply->readAll();
    setMapImageData(image);
    setMapImageFormat("png");
    setFinished(true);
}

void OsmTileReply::onNetworkReplyError(QNetworkReply::NetworkError error)
{
    QNetworkReply *reply = m_reply;
    if (!reply) {
        return;
    }

    reply->deleteLater();
    m_reply = nullptr;

    if (error != QNetworkReply::OperationCanceledError) {
        setError(QGeoTiledMapReply::CommunicationError, reply->errorString());
    }

    setFinished(true);
}
