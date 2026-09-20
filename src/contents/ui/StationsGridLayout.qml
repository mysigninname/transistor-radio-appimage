// SPDX-FileCopyrightText: 2024 Yuri Saurov <dr@i-glu4it.ru>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import ru.transistor_radio.transistor

GridView {

    id: gridView
    interactive: true
    boundsBehavior: Flickable.StopAtBounds
    reuseItems: true
    focus: true
    readonly property real effectiveWidth: width - leftMargin - rightMargin
    readonly property real targetDelegateWidth: Kirigami.Units.gridUnit * 18
    readonly property real minimumDelegateWidth: Kirigami.Units.gridUnit * 12
    readonly property real maximumColumns: 6
    readonly property int columns: Math.max(1, Math.min(maximumColumns, Math.floor(effectiveWidth / targetDelegateWidth)))
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
