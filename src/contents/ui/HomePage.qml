// SPDX-FileCopyrightText: 2024 Yuri Saurov <dr@i-glu4it.ru>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import Qt.labs.platform as Labs
import QtQuick.Dialogs as Dialogs
import ru.transistor_radio.transistor

FeedPage {
    id: page
    title: i18n("My stations")
    property string request
    property bool isSearch: true
    Component.onCompleted: {
        if(TransistorConfig.autoupdateInfo) {
            StationDBModel.updateStationsFromApi()
        }
        StationDBModel.loadStations();
    }
    actions: [
        Kirigami.Action {
            text: i18n("Add local station…")
            icon.name: "list-add"
            displayHint: Kirigami.Settings.isMobile ? Kirigami.DisplayHint.IconOnly : Kirigami.DisplayHint.KeepVisible
            onTriggered: {
                var popupComponent = Qt.createComponent("AddStationDialog.qml");
                var addStationDialog = popupComponent.createObject(pageStack);
                addStationDialog.open();
            }
        },
        Kirigami.Action {
            text: i18n("Update stations info")
            icon.name: "system-software-update-symbolic"
            displayHint: Kirigami.Settings.isMobile ? Kirigami.DisplayHint.IconOnly : Kirigami.DisplayHint.KeepVisible
            visible: gridView.count > 0
            onTriggered: {
                StationDBModel.updateStationsFromApi()
            }
        },
        Kirigami.Action {
            text: i18n("Import stations…")
            icon.name: "document-import-symbolic"
            displayHint: Kirigami.Settings.isMobile ? Kirigami.DisplayHint.IconOnly : Kirigami.DisplayHint.KeepVisible
            onTriggered: {
                var importDialog = Qt.createComponent("ImportExportDialog.qml");
                var dialog = importDialog.createObject(page);
                dialog.exportMode = false;
                dialog.open();
            }
        },
        Kirigami.Action {
            text: i18n("Export stations…")
            icon.name: "document-export-symbolic"
            displayHint: Kirigami.Settings.isMobile ? Kirigami.DisplayHint.IconOnly : Kirigami.DisplayHint.KeepVisible
            visible: gridView.count > 0
            onTriggered: {
                var exportDialog = Qt.createComponent("ImportExportDialog.qml");
                var dialog = exportDialog.createObject(page);
                dialog.exportMode = true;
                dialog.open();
            }
        }
    ]

    StationsGridLayout {
        id: gridView
        model: StationDBModel
        Kirigami.PlaceholderMessage {
            anchors.centerIn: parent
            width: parent.width - (Kirigami.Units.largeSpacing * 4)
            visible: gridView.count == 0

            icon.name: "ru.transistor_radio.transistor"
            text: "Your library is empty"
            explanation: "Add new stations by clicking on the button below."
            helpfulAction: Kirigami.Action {
                icon.name: "list-add"
                text: i18n("Add New Stations")
                onTriggered: {
                    pushPage("DiscoverPage");
                }
            }
        }
    }
}
