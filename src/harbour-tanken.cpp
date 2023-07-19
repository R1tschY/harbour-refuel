#ifdef QT_QML_DEBUG
#include <QtQuick>
#endif
#include <QtQml>

#include <sailfishapp.h>
#include "stationlistmodel.h"
#include "request.h"

int main(int argc, char *argv[])
{
    // SailfishApp::main() will display "qml/harbour-tanken.qml", if you need more
    // control over initialization, you can use:
    //
    //   - SailfishApp::application(int, char *[]) to get the QGuiApplication *
    //   - SailfishApp::createView() to get a new QQuickView * instance
    //   - SailfishApp::pathTo(QString) to get a QUrl to a resource file
    //   - SailfishApp::pathToMainQml() to get a QUrl to the main QML file
    //
    // To display the view, call "show()" (will show fullscreen on device).

    qmlRegisterType<StationListModel>(
                "de.richardliebscher.tanken", 0, 1, "StationListModel");
    qmlRegisterType<Request>(
                "de.richardliebscher.tanken", 0, 1, "Request");

    return SailfishApp::main(argc, argv);
}
