import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import Qt.labs.platform as Labs
import org.kde.transistor

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
                importExportDialog.exportMode = false;
                importExportDialog.open();
            }
        },
        Kirigami.Action {
            text: i18n("Export stations…")
            icon.name: "document-export-symbolic"
            displayHint: Kirigami.Settings.isMobile ? Kirigami.DisplayHint.IconOnly : Kirigami.DisplayHint.KeepVisible
            visible: gridView.count > 0
            onTriggered: {
                importExportDialog.exportMode = true;
                importExportDialog.open();
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

            icon.name: "transistor"
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
    Labs.FileDialog {
        id: importExportDialog
        property bool exportMode
        title: exportMode ? i18n("Export stations") : i18n("Import stations")
        nameFilters: ["Transistor Stations Backup (*.stations)"]
        folder: Labs.StandardPaths.writableLocation(Labs.StandardPaths.DocumentsLocation)
        fileMode: exportMode ? Labs.FileDialog.SaveFile : Labs.FileDialog.OpenFile
        acceptLabel: exportMode ? i18n("Export") : i18n("Import")
        onVisibleChanged: {
            if (visible) {
                const home = Labs.StandardPaths.writableLocation(Labs.StandardPaths.DocumentsLocation);
                currentFile = "file:///" + home + "/transistor.stations";
            }
        }
        onAccepted: {
            let localPath = currentFile.toString().replace("file://", "");
            if (exportMode) {
                if (StationDBModel.exportStations(localPath)) {
                    showPassiveNotification(i18n("Stations saved successfully"));
                } else {
                    showPassiveNotification(i18n("An error occurred while saving"));
                }
            } else {
                if (StationDBModel.importStations(localPath)) {
                    showPassiveNotification(i18n("Stations loaded successfully"));
                } else {
                    showPassiveNotification(i18n("An error occurred when uploading"));
                }
            }
        }
    }
}
