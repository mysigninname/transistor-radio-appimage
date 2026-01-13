# SPDX-FileCopyrightText: 2024 Yuri Saurov <dr@i-glu4it.ru>
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 20)
#set(CMAKE_BUILD_TYPE Release)

# Set output directories to build/bin and build/lib
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Set Qt generated files output directories
#set(CMAKE_AUTOMOC_OUTPUTS_DIRECTORY ${CMAKE_BINARY_DIR}/moc)
#set(CMAKE_AUTORCC_OUTPUTS_DIRECTORY ${CMAKE_BINARY_DIR}/rcc)
#set(CMAKE_AUTOUIC_OUTPUTS_DIRECTORY ${CMAKE_BINARY_DIR}/ui)

project(transistor VERSION 1.3)

set(QT6_MIN_VERSION 6.5.0)
set(KF6_MIN_VERSION 6.0.0)

find_package(ECM ${KF6_MIN_VERSION} REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

include(FeatureSummary)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMSetupVersion)
include(ECMFindQmlModule)
if (NOT ANDROID)
    include(KDEGitCommitHooks)
    include(KDEClangFormat)
endif()

ecm_setup_version(${PROJECT_VERSION}
    VARIABLE_PREFIX TRANSISTOR
    VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/src/version-transistor.h"
)

set(QT_EXTRA_COMPONENTS)
if (NOT ANDROID)
    list(APPEND QT_EXTRA_COMPONENTS Widgets)
endif()

find_package(Qt6 ${QT6_MIN_VERSION} REQUIRED COMPONENTS Core Gui Qml QuickControls2 Multimedia Sql Svg ${QT_EXTRA_COMPONENTS})
find_package(KF6 ${KF6_MIN_VERSION} REQUIRED COMPONENTS Kirigami CoreAddons Config I18n ColorScheme IconThemes Notifications)

qt_policy(SET QTP0001 NEW)

ecm_find_qmlmodule(org.kde.kirigamiaddons.formcard 1.0)

if (ANDROID)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/android/version.gradle.in ${CMAKE_BINARY_DIR}/version.gradle)
endif()
if (NOT ANDROID AND NOT WIN32 AND NOT APPLE AND NOT HAIKU)
    find_package(KF6DBusAddons ${KF_MIN_VERSION} REQUIRED)
endif()

add_subdirectory(src)
install(FILES transistor.notifyrc DESTINATION ${KDE_INSTALL_KNOTIFYRCDIR})
install(FILES ru.transistor_radio.transistor.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES ru.transistor_radio.transistor.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
install(FILES ru.transistor_radio.transistor.svg DESTINATION ${KDE_INSTALL_FULL_ICONDIR}/hicolor/scalable/apps)
install(FILES icons/ru.transistor_radio.transistor-tray-dark.svg DESTINATION ${KDE_INSTALL_FULL_ICONDIR}/hicolor/scalable/apps)
install(FILES icons/ru.transistor_radio.transistor-tray-light.svg DESTINATION ${KDE_INSTALL_FULL_ICONDIR}/hicolor/scalable/apps)

ki18n_install(po)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

if (NOT ANDROID)
    file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES src/*.cpp src/*.h)
    kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})
    kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
endif()
