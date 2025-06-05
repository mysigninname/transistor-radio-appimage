// SPDX-FileCopyrightText: 2020 Tobias Fella <tobias.fella@kde.org>
// SPDX-FileCopyrightText: 2021-2022 Bart De Vries <bart@mogwai.be>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.transistor

Kirigami.OverlayDrawer {
    id: root
    modal: false
    closePolicy: Controls.Popup.NoAutoClose
    edge: Qt.application.layoutDirection === Qt.RightToLeft ? Qt.RightEdge : Qt.LeftEdge

    readonly property real pinnedWidth: Kirigami.Units.gridUnit * 3
    readonly property real widescreenBigWidth: Kirigami.Units.gridUnit * 20
    readonly property int buttonDisplayMode: transistorMainWindow.isWidescreen ? Kirigami.NavigationTabButton.TextBesideIcon : Kirigami.NavigationTabButton.IconOnly
    readonly property bool settingsOpened: Kirigami.Settings.isMobile && pageStack.layers.depth >= 2 && pageStack.layers.currentItem.title === "Settings"
    width: showGlobalDrawer ? (transistorMainWindow.isWidescreen ? Kirigami.Units.gridUnit * 10 : pinnedWidth) : 0

    Kirigami.Theme.colorSet: Kirigami.Theme.Window
    Kirigami.Theme.inherit: false

    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0

    contentItem: Loader {
        id: sidebarColumn
        active: showGlobalDrawer

        sourceComponent: ColumnLayout {
            spacing: 0

            Controls.ToolBar {
                Layout.fillWidth: true
                Layout.preferredHeight: pageStack.globalToolBar.preferredHeight

                leftPadding: Kirigami.Units.smallSpacing
                rightPadding: Kirigami.Units.smallSpacing
                topPadding: Kirigami.Units.smallSpacing
                bottomPadding: Kirigami.Units.smallSpacing

                /*contentItem: Kirigami.SearchField {
                    id: search
                    autoAccept: false
                    onAccepted: {
                        var pageObject = Qt.createComponent("org.kde.transistor", "SearchPage");
                        pageStack.clear();
                        pageStack.layers.clear();
                        pageStack.push(pageObject, {initialSearch: search.text});
                        //currentPage = "SearchPage";
                        search.text = "";
                    }
                }*/
            }

            Controls.ScrollView {
                id: scrollView
                Layout.fillWidth: true
                Layout.fillHeight: true

                Controls.ScrollBar.vertical.policy: Controls.ScrollBar.AlwaysOff
                Controls.ScrollBar.horizontal.policy: Controls.ScrollBar.AlwaysOff
                contentWidth: -1 // disable horizontal scroll

                ColumnLayout {
                    id: column
                    width: scrollView.width
                    spacing: 0

                    Kirigami.NavigationTabButton {
                        Layout.fillWidth: true
                        display: root.buttonDisplayMode
                        text: i18nc("@title of page showing the list queued items; this is the noun 'the queue', not the verb", "My Stations")
                        icon.name: "bookmarks-bookmarked-symbolic"
                        checked: currentPage == "HomePage" && !settingsOpened
                        onClicked: {
                            pushPage("HomePage");
                        }
                    }
                    Kirigami.NavigationTabButton {
                        Layout.fillWidth: true
                        display: root.buttonDisplayMode
                        text: i18nc("@title of page allowing to search for new podcasts online", "Discover")
                        icon.name: "rss"
                        checked: currentPage == "DiscoverPage" && !settingsOpened
                        onClicked: {
                            pushPage("DiscoverPage");
                        }
                    }
                    Kirigami.NavigationTabButton {
                        Layout.fillWidth: true
                        display: root.buttonDisplayMode
                        text: i18nc("@title of page with list of podcast subscriptions", "Search")
                        icon.name: "search"
                        checked: currentPage == "SearchPage" && !settingsOpened
                        onClicked: {
                            pushPage("SearchPage");
                        }
                    }
                }
            }

            Kirigami.Separator {
                Layout.fillWidth: true
                Layout.rightMargin: Kirigami.Units.smallSpacing
                Layout.leftMargin: Kirigami.Units.smallSpacing
            }

            Kirigami.NavigationTabButton {
                Layout.fillWidth: true
                display: root.buttonDisplayMode

                text: i18nc("@title of dialog with app settings", "Settings")
                icon.name: "settings-configure"
                checked: settingsOpened
                onClicked: {
                    if (!Kirigami.Settings.isMobile)
                        checked = false;
                    pushPage("SettingsView");
                }
            }
        }
    }
}
