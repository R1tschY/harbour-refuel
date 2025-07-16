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
