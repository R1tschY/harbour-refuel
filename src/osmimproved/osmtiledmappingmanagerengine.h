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
 
#pragma once

#include <QGeoServiceProvider>
#include <private/qgeotiledmappingmanagerengine_p.h>
#include <private/qgeotilefetcher_p.h>
#include <private/qgeotiledmapreply_p.h>
#include <QNetworkAccessManager>
#include <QByteArray>
#include <QUrl>
#include <QObject>
#include <QNetworkReply>

class OsmTiledMappingManagerEngine : public QGeoTiledMappingManagerEngine {
    Q_OBJECT

public:
    OsmTiledMappingManagerEngine(
        const QVariantMap& parameters,
        QGeoServiceProvider::Error* error, QString* errorString, QObject* parent);
    ~OsmTiledMappingManagerEngine();

    QGeoMapData* createMapData();
};


class OsmTileFetcher : public QGeoTileFetcher {
    Q_OBJECT
public:
    explicit OsmTileFetcher(const QByteArray& userAgent, QObject* parent);
    ~OsmTileFetcher();

private:
    QGeoTiledMapReply *getTileImage(const QGeoTileSpec& spec);

    QUrl m_baseUrl;
    QByteArray m_userAgent;
    QNetworkAccessManager m_networkManager;
};


class OsmTileReply : public QGeoTiledMapReply {
    Q_OBJECT
public:
    explicit OsmTileReply(QNetworkReply* reply, const QGeoTileSpec& spec, QObject* parent);
    ~OsmTileReply();

    void abort() override;

private:
    void onNetworkReplyFinished();
    void onNetworkReplyError(QNetworkReply::NetworkError error);

    QNetworkReply* m_reply;
};
