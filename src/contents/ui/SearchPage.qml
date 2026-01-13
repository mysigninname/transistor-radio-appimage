// SPDX-FileCopyrightText: 2024 Yuri Saurov <dr@i-glu4it.ru>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.components as Components

import ru.transistor_radio.transistor

FeedPage {
    id: page
    title: i18n("Search")
    property string request
    property bool isSearch: true
    Component.onCompleted: {
        StationSearchModel.clearAll();
    }
    actions: [
        Kirigami.Action {
            displayHint: Kirigami.DisplayHint.KeepVisible
            displayComponent: RowLayout {
                Layout.fillWidth: true
                width: parent.width - Kirigami.Units.gridUnit

                Kirigami.SearchField {
                    id: searchField
                    Layout.fillWidth: true
                    selectByMouse: true
                    autoAccept: false

                    onTextChanged: {
                    }

                    onAccepted: {
                        if (text.length > 0) {
                            request = searchField.text;
                            offset = 0;
                            StationSearchModel.getData(request, limit, offset, true, true);
                        } else {
                            request = "";
                            StationSearchModel.clearAll();
                        }
                    }
                    Component.onCompleted: {
                        searchField.forceActiveFocus();
                    }
                }

                Controls.Button {
                    icon.name: "search"
                    enabled: searchField.text.length > 0
                    onClicked: {
                        searchField.accepted();
                    }
                }
            }
        }
    ]
    StationsGridLayout {
        id: gridView
        model: StationSearchModel
        Kirigami.PlaceholderMessage {
            anchors.centerIn: parent
            width: parent.width - (Kirigami.Units.largeSpacing * 4)
            visible: gridView.count == 0 && !StationSearchModel.isSearching

            icon.name: "search"
            text: "No search results found"
            explanation: "Please specify the search parameters."
        }
        Kirigami.LoadingPlaceholder {
            anchors.centerIn: parent
            visible: gridView.model.isSearching && gridView.count == 0
        }
        footer: ColumnLayout {
            width: parent.width
            spacing: 0
            visible: gridView.count > 0

            Controls.ProgressBar {
                id: loadingBar

                visible: StationSearchModel.isSearching
                indeterminate: true
                Layout.alignment: Qt.AlignHCenter
            }
        }
        function getMore() {
            if (gridView.count == offset + limit) {
                offset = offset + limit;
                StationSearchModel.getData(page.request, page.limit, page.offset, false, page.isSearch);
            }
        }
        onContentHeightChanged: {
            if (contentHeight < height && gridView.count > 0)
                getMore();
        }
        onContentWidthChanged: {
            if (contentHeight < height) {
                getMore();
            }
        }
        onContentYChanged: {
            if (atYEnd)
                getMore();
        }
    }
}
