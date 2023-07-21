#ifdef QT_QML_DEBUG
#include <QtQuick>
#endif
#include <QtQml>
#include <QScopedPointer>

#include <sailfishapp.h>
#include "stationlistmodel.h"
#include "fuelpriceprovider.h"
#include "osmimproved/osmimprovedplugin.h"

Q_IMPORT_PLUGIN(OsmImprovedPlugin)

int main(int argc, char *argv[])
{
    qmlRegisterType<StationListModel>(
                "de.richardliebscher.refuel", 0, 1, "StationListModel");
    qmlRegisterUncreatableType<FuelPriceProvider>(
                "de.richardliebscher.refuel", 0, 1, "FuelPriceProvider",
                "abstract class");

    QScopedPointer<QGuiApplication> app { SailfishApp::application(argc, argv) };
    QScopedPointer<QQuickView> view { SailfishApp::createView() };

    QQmlEngine *engine = view->engine();
    engine->rootContext()->setContextProperty(
                QStringLiteral("qVersion"), QString(qVersion()));

    view->setSource(SailfishApp::pathToMainQml());
    view->showFullScreen();

    return app->exec();
}
