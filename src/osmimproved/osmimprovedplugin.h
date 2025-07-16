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

class OsmImprovedPlugin : public QObject, public QGeoServiceProviderFactory
{
    Q_OBJECT
    Q_INTERFACES(QGeoServiceProviderFactory)
    Q_PLUGIN_METADATA(IID "org.qt-project.qt.geoservice.serviceproviderfactory/5.0"
                      FILE "osmimproved_plugin.json")
public:
    OsmImprovedPlugin();

    QGeoCodingManagerEngine *createGeocodingManagerEngine(const QVariantMap &parameters, QGeoServiceProvider::Error *error, QString *errorString) const override;

    QGeoMappingManagerEngine *createMappingManagerEngine(const QVariantMap &parameters, QGeoServiceProvider::Error *error, QString *errorString) const override;
};
