// SPDX-FileCopyrightText: 2024 Yuri Saurov <dr@i-glu4it.ru>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import QtCore
import org.kde.kirigami as Kirigami
import org.kde.config as KConfig
//import org.kde.kirigamiaddons.components as Components
import ru.transistor_radio.transistor

Kirigami.ApplicationWindow {
    id: transistorMainWindow
    title: i18n("Transistor")

    property bool isMobile: Kirigami.Settings.isMobile
    property var currentModel
    property double audioVolume
    width: isMobile ? Kirigami.Units.gridUnit * 20 : Kirigami.Units.gridUnit * 45
    height: isMobile ? Kirigami.Units.gridUnit * 37 : Kirigami.Units.gridUnit * 34
    readonly property bool settingsOpened: isMobile && pageStack.layers.depth >= 2
    pageStack.clip: true
    pageStack.popHiddenPages: true
    pageStack.globalToolBar.style: Kirigami.ApplicationHeaderStyle.ToolBar
    pageStack.globalToolBar.showNavigationButtons: Kirigami.ApplicationHeaderStyle.ShowBackButton

    pageStack.columnView.columnResizeMode: Kirigami.ColumnView.SingleColumn

    minimumWidth: Kirigami.Units.gridUnit * 20
    minimumHeight: Kirigami.Units.gridUnit * 30

    property string currentPage: ""
    onCurrentPageChanged: {
        if (currentPage == "HomePage")
            currentModel = StationDBModel;
        else if (currentPage == "DiscoverPage" || currentPage == "SearchPage")
            currentModel = StationSearchModel;
    }
    property bool isWidescreen: transistorMainWindow.width > transistorMainWindow.height
    KConfig.WindowStateSaver {
        configGroupName: "MainWindow"
    }

    function pushPage(page: string): void {
        if (page === "SettingsView") {
            settingsView.open();
        } else {
            var pageObject = Qt.createComponent("ru.transistor_radio.transistor", page);
            if (!pageObject) {
                page = "HomePage";
                pageObject = Qt.createComponent("ru.transistor_radio.transistor", page);
            }
            pageStack.clear();
            pageStack.layers.clear();
            pageStack.push(pageObject);
            currentPage = page;
        }
    }

    Component.onCompleted: {
        pageStack.initialPage = pushPage(currentPage);
    }

    SettingsView {
        id: settingsView
        window: transistorMainWindow
    }

    property bool showGlobalDrawer: !isMobile
    globalDrawer: globalDrawerLoader.item
    Loader {
        id: globalDrawerLoader
        active: transistorMainWindow.showGlobalDrawer
        sourceComponent: TransistorGlobalDrawer {}
    }

    contextDrawer: InfoSidebar {
        id: stationInfo
    }

    Connections {
        target: AudioPlayer
        function onErrorStringChanged(): void {
            if (AudioPlayer.errorString.length > 0) {
                if (isMobile) {
                    stationInfo.close();
                }
                showPassiveNotification(AudioPlayer.errorString, 2000);
            }
        }
    }
    pageStack.anchors.bottomMargin: (StationManager.currentStation && isMobile && !settingsOpened) ? Kirigami.Units.iconSizes.huge + Kirigami.Units.smallSpacing : 0
    Loader {
        id: footerLoader

        anchors.bottom: parent.bottom //bottomToolbarLoader.top
        active: StationManager.currentStation && isMobile && !settingsOpened
        visible: active
        height: Kirigami.Units.iconSizes.huge + Kirigami.Units.smallSpacing
        width: transistorMainWindow.width
        z: (!item || item.contentY === 0) ? -1 : 0
        sourceComponent: FooterBar {
            focus: true
            //width: transistorMainWindow.width
        }
    }

    footer: Loader {
        id: bottomToolbarLoader
        visible: active
        height: visible ? implicitHeight : 0
        active: isMobile
        sourceComponent: BottomToolbar {}
    }

    Kirigami.PromptDialog {
        id: deleteDialog
        dialogType: Kirigami.PromptDialog.Warning
        property QtObject feed
        title: i18n("Remove Station?")
        subtitle: i18n("Do you really want to remove this station? This action cannot be canceled.")
        standardButtons: Kirigami.Dialog.Yes | Kirigami.Dialog.No

        onAccepted: {
            StationDBModel.removeStation(feed.stationUuid);
            if (pageStack.currentItem && pageStack.currentItem.objectName === "stationInfoPage") {
                pageStack.pop();
            }

            showPassiveNotification(i18n("Station is removed"));
        }
    }

    Connections {
        target: transistorMainWindow

        function onClosing(close) {
            if (SystrayIcon.available && TransistorConfig.showTrayIcon && TransistorConfig.minimizeToTray) {
                close.accepted = false;
                transistorMainWindow.hide();
            } else {
                close.accepted = true;
                Qt.quit();
            }
        }
    }

    Connections {
        target: SystrayIcon

        function onRaiseWindow() {
            if (transistorMainWindow.visible) {
                transistorMainWindow.visible = false;
                transistorMainWindow.hide();
            } else {
                transistorMainWindow.visible = true;
                transistorMainWindow.show();
                transistorMainWindow.raise();
                transistorMainWindow.requestActivate();
            }
        }
        function onPlaybackChanged() {
            if (AudioPlayer.playing) {
                AudioPlayer.stop();
            } else if (StationManager.currentStation) {
                AudioPlayer.play(StationManager.currentStation.stationSource);
            }
        }
        function onMutedChanged() {
            if (AudioPlayer.muted) {
                AudioPlayer.setMuted(false);
            } else {
                AudioPlayer.setMuted(true);
            }
            TransistorConfig.isMuted = AudioPlayer.muted;
            TransistorConfig.save();
        }
    }
}
