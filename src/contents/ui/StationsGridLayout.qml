import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.transistor

GridView {

    id: gridView
    interactive: true
    boundsBehavior: Flickable.StopAtBounds
    readonly property real effectiveWidth: width - leftMargin - rightMargin
    readonly property real targetDelegateWidth: Kirigami.Units.gridUnit * 22
    readonly property real minimumDelegateWidth: Kirigami.Units.gridUnit * 16
    readonly property real maximumColumns: 6
    readonly property int columns: effectiveWidth <= Kirigami.Units.gridUnit * 25 ? 1 : Math.max(2, Math.min(maximumColumns, Math.floor(effectiveWidth / minimumDelegateWidth), Math.ceil(effectiveWidth / targetDelegateWidth)))
    moveDisplaced: Transition {
        NumberAnimation {
            properties: "x,y"
            duration: Kirigami.Units.veryLongDuration * 3
            easing.type: Easing.InOutQuad
        }
    }
    
    topMargin: Kirigami.Units.largeSpacing
    bottomMargin: Kirigami.Units.largeSpacing
    leftMargin: Kirigami.Units.largeSpacing
    rightMargin: Kirigami.Units.largeSpacing
    cellWidth: effectiveWidth / columns
    cellHeight: Kirigami.Units.gridUnit * 4 + Kirigami.Units.smallSpacing
    currentIndex: -1
    delegate: ListDelegate {}
    
}
