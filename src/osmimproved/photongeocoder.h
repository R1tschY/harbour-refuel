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

#include <QObject>
#include <QGeoServiceProviderFactory>
#include <QGeoCodeReply>
#include <QGeoCodingManagerEngine>
#include <QNetworkReply>

class PhotonGeoCodeReply : public QGeoCodeReply {
    Q_OBJECT
public:
    explicit PhotonGeoCodeReply(QNetworkReply* reply, int limit, QObject* parent);
    ~PhotonGeoCodeReply();

    void abort() override;

private:
    void onNetworkReplyFinished();
    void onNetworkReplyError(QNetworkReply::NetworkError error);

    QNetworkReply* m_reply;
};


class PhotonGeoCodingManagerEngine : public QGeoCodingManagerEngine
{
    Q_OBJECT
public:
    explicit PhotonGeoCodingManagerEngine(const QVariantMap &parameters, QGeoServiceProvider::Error *error, QString *errorString);
    ~PhotonGeoCodingManagerEngine();

    PhotonGeoCodeReply* geocode(const QGeoAddress &address, const QGeoShape &bounds) override;
    PhotonGeoCodeReply* geocode(const QString &address, int limit, int offset, const QGeoShape &bounds) override;
    QGeoCodeReply* reverseGeocode(const QGeoCoordinate &coordinate, const QGeoShape &bounds) override;

private:
    QUrl m_baseUrl;
    QByteArray m_userAgent;
    QNetworkAccessManager m_networkManager;
};
