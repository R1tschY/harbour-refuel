project(harbour-tanken CXX)
cmake_minimum_required(VERSION 3.5)

find_package (Qt5 COMPONENTS Core Network Qml Gui Quick Location REQUIRED)

include(FindPkgConfig)
pkg_search_module(SAILFISH sailfishapp REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(harbour-tanken
    src/qtployfill.h
    src/httpreply.h src/httpreply.cpp

    src/osmimproved/photongeocoder.h src/osmimproved/photongeocoder.cpp
    src/osmimproved/osmimprovedplugin.h src/osmimproved/osmimprovedplugin.cpp

    src/harbour-tanken.cpp
    src/tankerkoenigapirequest.h src/tankerkoenigapirequest.cpp
    src/request.h src/request.cpp
    src/stationlistmodel.h src/stationlistmodel.cpp
)
target_compile_definitions(harbour-tanken PRIVATE
    $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>
    QT_STATICPLUGIN
)
target_include_directories(harbour-tanken PRIVATE
    $<BUILD_INTERFACE:
    ${SAILFISH_INCLUDE_DIRS}
>)
target_link_libraries(harbour-tanken
    Qt5::Quick Qt5::Location
    ${SAILFISH_LDFLAGS}
)

install(TARGETS harbour-tanken
    RUNTIME DESTINATION bin
)
install(DIRECTORY qml
    DESTINATION share/harbour-tanken
)
install(DIRECTORY translations
    DESTINATION share/harbour-tanken
    FILES_MATCHING PATTERN "*.qm"
)
install(FILES harbour-tanken.desktop
    DESTINATION share/applications
)
install(FILES icons/86x86/harbour-tanken.png
    DESTINATION share/icons/hicolor/86x86/apps
)
install(FILES icons/108x108/harbour-tanken.png
    DESTINATION share/icons/hicolor/108x108/apps
)
install(FILES icons/128x128/harbour-tanken.png
    DESTINATION share/icons/hicolor/128x128/apps
)
install(FILES icons/172x172/harbour-tanken.png
    DESTINATION share/icons/hicolor/172x172/apps
)

# Get the other files reachable from the project tree in Qt Creator
FILE(GLOB TsFiles "translations/*.ts")
add_custom_target(distfiles
    SOURCES
        harbour-tanken.desktop
        qml/harbour-tanken.qml
        qml/cover/CoverPage.qml
        qml/pages/StationListPage.qml
        qml/pages/SearchPage.qml
        qml/pages/HomePage.qml
        qml/pages/ChooseLocationDialog.qml
        qml/pages/StationDetailsPage.qml
        rpm/harbour-tanken.spec
        ${TsFiles})

# Tell Qt Creator where the application executable(s) would be located on the
# device.
#
# It is not necessary to list other deployables than executables (runtime
# targets) here. The deployment process of Sailfish OS projects is opaque to
# Qt Creator and the information contained in QtCreatorDeployment.txt is only
# used to locate the executable associated with the active run configuration
# on the device in order to run it.
#
# Search the Qt Creator Manual to learn about the QtCreatorDeployment.txt file
# format.
file(WRITE "${CMAKE_BINARY_DIR}/QtCreatorDeployment.txt"
    "${CMAKE_INSTALL_PREFIX}
${CMAKE_BINARY_DIR}/harbour-tanken:bin
")

