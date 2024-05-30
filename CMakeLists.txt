cmake_minimum_required(VERSION 3.5)
project(harbour-refuel VERSION ${PACKAGE_VERSION} LANGUAGES CXX)

find_package(Qt5 COMPONENTS Core Network Qml Gui Sql Quick Positioning Location LinguistTools REQUIRED)

# pkg-config
include(FindPkgConfig)
pkg_search_module(SAILFISH sailfishapp REQUIRED)

# cpm
include(cmake/CPM.cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(TANKERKOENIG_APIKEY "" CACHE STRING "Tankerkönig API Key (https://creativecommons.tankerkoenig.de/)")
if(NOT "${TANKERKOENIG_APIKEY}" MATCHES "^[0-9a-f\\-]+$")
    message(SEND_ERROR "TANKERKOENIG_APIKEY missing or invalid: '${TANKERKOENIG_APIKEY}'")
endif()

configure_file(src/config.h.in config.h @ONLY)

file(GLOB TS_FILES CONFIGURE_DEPENDS "translations/*.ts")
file(GLOB_RECURSE CXX_SOURCES CONFIGURE_DEPENDS src/*.cpp src/*.h)
file(GLOB_RECURSE QML_SOURCES CONFIGURE_DEPENDS qml/*.qml)
qt5_create_translation(
    QM_FILES
    ${CXX_SOURCES} ${QML_SOURCES}
    ${TS_FILES})

add_executable(harbour-refuel
    ${CXX_SOURCES} ${QML_SOURCES} ${QM_FILES}
)
target_compile_definitions(harbour-refuel PRIVATE
    $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>
    QT_STATICPLUGIN
)
target_include_directories(harbour-refuel PRIVATE
    $<BUILD_INTERFACE: ${SAILFISH_INCLUDE_DIRS} >
    "${CMAKE_CURRENT_BINARY_DIR}")
target_link_libraries(harbour-refuel
    Qt5::Quick Qt5::Sql Qt5::Positioning Qt5::Location
    ${SAILFISH_LDFLAGS}
)

install(TARGETS harbour-refuel
    RUNTIME DESTINATION bin
)
install(DIRECTORY qml
    DESTINATION share/harbour-refuel
)
install(FILES ${QM_FILES}
    DESTINATION share/harbour-refuel/translations
)
install(FILES harbour-refuel.desktop
    DESTINATION share/applications
)
install(FILES icons/86x86/harbour-refuel.png
    DESTINATION share/icons/hicolor/86x86/apps
)
install(FILES icons/108x108/harbour-refuel.png
    DESTINATION share/icons/hicolor/108x108/apps
)
install(FILES icons/128x128/harbour-refuel.png
    DESTINATION share/icons/hicolor/128x128/apps
)
install(FILES icons/172x172/harbour-refuel.png
    DESTINATION share/icons/hicolor/172x172/apps
)

install(FILES fonts/DSEG7Modern-Regular.ttf
    DESTINATION share/harbour-refuel/fonts
)

# Get the other files reachable from the project tree in Qt Creator
add_custom_target(distfiles
    SOURCES
        harbour-refuel.desktop
        qml/harbour-refuel.qml
        qml/cover/CoverPage.qml
        qml/cover/FavsCover.qml
        qml/cover/SearchResultsCover.qml
        qml/cover/StationCover.qml
        qml/pages/StationListPage.qml
        qml/pages/SearchPage.qml
        qml/pages/HomePage.qml
        qml/pages/AboutPage.qml
        qml/pages/ChooseLocationDialog.qml
        qml/pages/StationDetailsPage.qml
        qml/components/LastSearchesModel.qml
        qml/components/DbConnection.qml
        qml/components/PriceDisplay.qml
        qml/components/FooterText.qml
        rpm/harbour-refuel.spec
        README.md
        ${TS_FILES})

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
${CMAKE_BINARY_DIR}/harbour-refuel:bin
")

