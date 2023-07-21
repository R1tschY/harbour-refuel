#include "osmimprovedplugin.h"

#include "photongeocoder.h"

OsmImprovedPlugin::OsmImprovedPlugin() = default;

QGeoCodingManagerEngine *OsmImprovedPlugin::createGeocodingManagerEngine(const QVariantMap &parameters, QGeoServiceProvider::Error *error, QString *errorString) const
{
    return new PhotonGeoCodingManagerEngine(parameters, error, errorString);
}
