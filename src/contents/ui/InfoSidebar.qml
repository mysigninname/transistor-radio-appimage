import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.transistor

Kirigami.OverlayDrawer {
    id: contextDrawer
    property int currentIndex
    edge: isMobile ? Qt.BottomEdge : Qt.RightEdge
    drawerOpen: !isMobile && StationManager.currentStation
    Kirigami.Theme.colorSet: Kirigami.Theme.Window
    Kirigami.Theme.inherit: false
    width: isMobile ? parent.width : Kirigami.Units.gridUnit * 12
    height: isMobile ? contentItem.implicitHeight : applicationWindow().height
    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0
    modal: isMobile || applicationWindow().width < Kirigami.Units.gridUnit * 50
    handleVisible: StationManager.currentStation && !isMobile
    contentItem: StationInfoComponent{}
}
