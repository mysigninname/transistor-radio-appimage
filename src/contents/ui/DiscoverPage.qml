// SPDX-FileCopyrightText: 2024 Yuri Saurov <dr@i-glu4it.ru>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami
import ru.transistor_radio.transistor

FeedPage {
    id: page

    property string request
    property bool isSearch: false

    title: i18nc("@title Trending", "Trending")
    Component.onCompleted: {
        actions[0].trigger();
    }

    actions: [
        FilterAction {
            title: i18n("Voted")
            description: i18n("Most Voted Stations")
            label: "favorite"
            request: "votes"
        },
        FilterAction {
            title: i18n("Trending")
            description: i18n("Trending Stations")
            label: "rss-symbolic"
            request: "clicktrend"
        },
        FilterAction {
            title: i18n("Now Listening")
            description: i18n("Other Users Are Listening")
            label: "view-process-users-symbolic"
            request: "clicktimestamp"
        },
        FilterAction {
            title: i18n("Newest")
            description: i18n("Last Changed Stations")
            label: "media-playlist-play-symbolic"
            request: "changetimestamp"
        },
        FilterAction {
            title: i18n("Random")
            description: i18n("Random Stations")
            label: "randomize-symbolic"
            request: "random"
        }
    ]

    StationsGridLayout {
        id: gridView
        model: StationSearchModel
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
            if (gridView.count == page.offset + page.limit) {
                page.offset += page.limit;
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
