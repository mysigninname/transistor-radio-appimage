# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2025 Yuri Saurov <dr@i-glu4it.ru>

qt_policy(SET QTP0004 NEW)

# Target: static library
qt_add_qml_module(transistor_static
    STATIC
    URI ru.transistor_radio.transistor
    VERSION 1.0
    OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/qml
    QML_FILES
        contents/ui/Main.qml
        contents/ui/FeedPage.qml
        contents/ui/HomePage.qml
        contents/ui/SearchPage.qml
        contents/ui/DiscoverPage.qml
        contents/ui/StationsGridLayout.qml
        contents/ui/ListDelegate.qml
        contents/ui/StationIcon.qml
        contents/ui/InfoSidebar.qml
        contents/ui/FilterAction.qml
        contents/ui/TransistorGlobalDrawer.qml
        contents/ui/StationInfoPage.qml
        contents/ui/StationInfoDelegate.qml
        contents/ui/StationInfoComponent.qml
        contents/ui/BottomToolbar.qml
        contents/ui/FooterBar.qml
        contents/ui/SettingsView.qml
        contents/ui/GeneralSettingsPage.qml
        contents/ui/AddStationDialog.qml
        contents/ui/ImportExportDialog.qml
)

set_source_files_properties(../ru.transistor_radio.transistor.svg PROPERTIES
    QT_RESOURCE_ALIAS icons/ru.transistor_radio.transistor.svg
)
set_source_files_properties(../icons/ru.transistor_radio.transistor-tray-light.svg PROPERTIES
    QT_RESOURCE_ALIAS icons/ru.transistor_radio.transistor-tray-light.svg
)
set_source_files_properties(../icons/ru.transistor_radio.transistor-tray-dark.svg PROPERTIES
    QT_RESOURCE_ALIAS icons/ru.transistor_radio.transistor-tray-dark.svg
)

qt_add_resources(transistor_static "icons"
    FILES
        ../ru.transistor_radio.transistor.svg
        ../icons/ru.transistor_radio.transistor-tray-light.svg
        ../icons/ru.transistor_radio.transistor-tray-dark.svg
)

target_sources(transistor_static PRIVATE
    DatabaseManager.cpp
    StationManager.cpp
    StationAbstractModel.cpp
    StationDBModel.cpp
    StationSearchModel.cpp
    ApiManager.cpp
    StationInfo.cpp
    AudioPlayer.cpp
    NotificationManager.cpp
    StreamReader.cpp
    SystrayIcon.cpp
    ColorSchemer.cpp
)

target_link_libraries(transistor_static PUBLIC
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Multimedia
    Qt6::Sql
    Qt6::Svg
    Qt6::Concurrent
    KF6::I18n
    KF6::CoreAddons
    KF6::ConfigCore
    KF6::ConfigGui
    KF6::ColorScheme
    KF6::IconThemes
    KF6::Notifications
)

target_include_directories(transistor_static PUBLIC ${CMAKE_BINARY_DIR})

target_link_libraries(transistor_static PUBLIC Qt::Widgets)

kconfig_target_kcfg_file(transistor_static FILE transistorconfig.kcfg CLASS_NAME TransistorConfig MUTATORS GENERATE_PROPERTIES DEFAULT_VALUE_GETTERS PARENT_IN_CONSTRUCTOR SINGLETON GENERATE_MOC QML_REGISTRATION)

add_executable(transistor main.cpp)
target_link_libraries(transistor PUBLIC transistor_static transistor_staticplugin)
install(TARGETS transistor ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
