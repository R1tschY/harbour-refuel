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
