import QtQuick
import QtQuick.Controls
import org.kde.kirigami as Kirigami
import org.kde.transistor

Kirigami.NavigationTabBar {
    id: navBar
    position: ToolBar.Footer
    z: 999

    property alias toolbarHeight: navBar.implicitHeight

    actions: [
        Kirigami.Action {
            icon.name: "bookmarks-bookmarked-symbolic"
            text: i18n("Home")
            checked: "HomePage" === transistorMainWindow.currentPage && !settingsOpened
            onTriggered: {
                pushPage("HomePage");
            }
        },
        Kirigami.Action {
            icon.name: "rss"
            text: i18n("Discover")
            checked: "DiscoverPage" === transistorMainWindow.currentPage && !settingsOpened
            onTriggered: {
                pushPage("DiscoverPage");
            }
        },
        Kirigami.Action {
            icon.name: "search"
            text: i18n("Search")
            checked: "SearchPage" === transistorMainWindow.currentPage && !settingsOpened
            onTriggered: {
                pushPage("SearchPage");
            }
        },
        Kirigami.Action {
            icon.name: "settings-configure"
            text: i18nc("@title of dialog with app settings", "Settings")
            checked: settingsOpened
            onTriggered: {
                if (!settingsOpened)
                    pushPage("SettingsView");
            }
        }
    ]
}
