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

Dialogs.FileDialog {
    id: importExportDialog
    property bool exportMode
    title: exportMode ? i18n("Export stations") : i18n("Import stations")
    nameFilters: ["Transistor Stations Backup (*.stations)"]
    currentFolder: Labs.StandardPaths.writableLocation(Labs.StandardPaths.DocumentsLocation)
    fileMode: exportMode ? Dialogs.FileDialog.SaveFile : Dialogs.FileDialog.OpenFile
    acceptLabel: exportMode ? i18n("Export") : i18n("Import")
    /*onVisibleChanged: {
        if (visible) {
            const home = Labs.StandardPaths.writableLocation(Labs.StandardPaths.DocumentsLocation);
            currentFile = "file:///" + home + "/transistor.stations";
        }
    }*/
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
        destroy();
    }
    onRejected: {
        destroy();
    }
}