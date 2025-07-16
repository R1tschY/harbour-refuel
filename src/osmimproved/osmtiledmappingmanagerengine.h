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
