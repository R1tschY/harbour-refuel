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
 
#ifdef QT_QML_DEBUG
#include <QtQuick>
#endif
#include <QtQml>
#include <QGuiApplication>
#include <QQuickView>
#include <QScopedPointer>

#include <sailfishapp.h>
#include "stationlistmodel.h"
#include "fuelpriceprovider.h"
#include "osmimproved/osmimprovedplugin.h"
#include "models/sqlquerymodel.h"
#include "config.h"
#include "station.h"

Q_IMPORT_PLUGIN(OsmImprovedPlugin)

int main(int argc, char *argv[])
{
    qmlRegisterType<StationListModel>(
                "de.richardliebscher.refuel", 0, 1, "StationListModel");
    qmlRegisterType<TankerKoenigProvider>(
                "de.richardliebscher.refuel", 0, 1, "TankerKoenigProvider");
    qmlRegisterType<Station>(
                "de.richardliebscher.refuel", 0, 1, "Station");
    qmlRegisterType<SqlQueryModel>(
                "de.richardliebscher.refuel", 0, 1, "SqlQueryModel");
    qmlRegisterUncreatableType<FuelPriceProvider>(
                "de.richardliebscher.refuel", 0, 1, "FuelPriceProvider",
                "abstract class");

    QScopedPointer<QGuiApplication> app { SailfishApp::application(argc, argv) };
    QScopedPointer<QQuickView> view { SailfishApp::createView() };

    app->setApplicationVersion(QStringLiteral(PACKAGE_COMPLETE_VERSION));

    QQmlEngine *engine = view->engine();
    engine->rootContext()->setContextProperty(
                QStringLiteral("qVersion"), QString(qVersion()));

    view->setSource(SailfishApp::pathToMainQml());
    view->showFullScreen();

    return app->exec();
}
