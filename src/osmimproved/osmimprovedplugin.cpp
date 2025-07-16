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
 
#include "osmimprovedplugin.h"

#include "photongeocoder.h"
#include "osmtiledmappingmanagerengine.h"

OsmImprovedPlugin::OsmImprovedPlugin() = default;

QGeoCodingManagerEngine *OsmImprovedPlugin::createGeocodingManagerEngine(const QVariantMap &parameters, QGeoServiceProvider::Error *error, QString *errorString) const
{
    return new PhotonGeoCodingManagerEngine(parameters, error, errorString);
}

QGeoMappingManagerEngine *OsmImprovedPlugin::createMappingManagerEngine(const QVariantMap &parameters, QGeoServiceProvider::Error *error, QString *errorString) const
{
    return new OsmTiledMappingManagerEngine(parameters, error, errorString, nullptr);
}
