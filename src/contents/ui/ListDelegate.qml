import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import ru.transistor_radio.transistor

Item {
    id: cardParent
    property var view: GridView.view || ListView.view
    width: view.cellWidth || view.width
    height: view.cellHeight || (Kirigami.Units.gridUnit * 4 + Kirigami.Units.smallSpacing)
    required property string stationName
    required property string stationImageSource
    required property string stationSource
    required property string stationLanguage
    required property string stationCountry
    required property string stationTags
    required property string stationHomepage
    required property int stationVotes
    required property string stationState
    required property int stationBitrate
    required property string stationCodec
    required property bool stationIsLocal
    required property var model

    Kirigami.AbstractCard {
        id: cardLayout
        showClickFeedback: true
        width: parent.width - Kirigami.Units.largeSpacing
        anchors.centerIn: parent
        topPadding: 0
        leftPadding: 0
        bottomPadding: 0
        rightPadding: Kirigami.Units.largeSpacing * 2
        hoverEnabled: true

        contentItem: Item {
            implicitHeight: delegateLayout.implicitHeight

            RowLayout {
                id: delegateLayout

                spacing: 0
                clip: true

                anchors {
                    left: parent.left
                    top: parent.top
                    right: parent.right
                }

                StationIcon {
                    id: stationIcon
                    title: stationName
                    width: Kirigami.Units.iconSizes.huge + Kirigami.Units.smallSpacing
                    height: width
                    color: "transparent"
                    asynchronous: true
                    fillMode: Image.PreserveAspectFit
                    source: cardParent.visible ? stationImageSource : ""
                    corners.bottomLeftRadius: Kirigami.Units.cornerRadius
                    corners.topLeftRadius: Kirigami.Units.cornerRadius
                    border.width: 1
                    border.color: Kirigami.ColorUtils.linearInterpolation(Kirigami.Theme.backgroundColor, Kirigami.Theme.textColor, Kirigami.Theme.frameContrast)
                }

                ColumnLayout {
                    clip: true
                    Layout.topMargin: Kirigami.Units.largeSpacing
                    Layout.bottomMargin: Kirigami.Units.largeSpacing
                    Layout.leftMargin: Kirigami.Units.largeSpacing
                    Layout.rightMargin: Kirigami.Units.largeSpacing

                    Kirigami.Heading {
                        id: name

                        level: 3
                        elide: Text.ElideRight
                        text: stationName.trim().replace(/\n/g, " ")
                        type: Kirigami.Heading.Type.Normal
                        Layout.fillWidth: true
                        antialiasing: true
                    }

                    Kirigami.Separator {
                        Layout.fillWidth: true
                        visible: stationIsLocal || (!stationIsLocal && (stationCountry || stationTags))
                    }

                    Controls.Label {
                        visible: stationIsLocal
                        text: i18n("Local Station")
                        antialiasing: true
                    }

                    RowLayout {
                        visible: !stationIsLocal
                        Controls.Label {
                            id: countryLabel
                            visible:  text.length > 0
                            text: stationCountry
                            antialiasing: true
                        }

                        Kirigami.Separator {
                            visible: countryLabel.visible && tagsLabel.visible
                            Layout.fillHeight: true
                        }

                        Controls.Label {
                            id: tagsLabel

                            elide: Text.ElideRight
                            Layout.fillWidth: true
                            visible: stationTags !== ""
                            text: stationTags.replace(/,/g, ", ")
                            antialiasing: true
                        }
                    }
                }

                /*Controls.Button {
                    text: i18n("Local Station")
                    display: Controls.Button.IconOnly
                    Layout.rightMargin: Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing
                    visible: !stationIsLocal
                    Controls.ToolTip.visible: hovered
                    Controls.ToolTip.text: text
                    background: Item {
                        implicitHeight: Kirigami.Units.iconSizes.smallMedium
                        implicitWidth: Kirigami.Units.iconSizes.smallMedium
                    }
                    contentItem: Item {
                        Kirigami.Icon {
                            source: "offline"
                            implicitHeight: Kirigami.Units.iconSizes.smallMedium
                            implicitWidth: Kirigami.Units.iconSizes.smallMedium
                            anchors.centerIn: parent
                        }
                    }
                }*/

                Controls.Button {
                    id: playButton
                    text: i18n("Play") + " " + name.text
                    display: Controls.Button.IconOnly
                    Controls.ToolTip.visible: hovered
                    Controls.ToolTip.text: text
                    height: Kirigami.Units.iconSizes.medium
                    width: Kirigami.Units.iconSizes.medium
                    z: 1000
                    onClicked: {
                        view.currentIndex = model.index;
                        transistorMainWindow.currentModel.switchToStationByIndex(view.currentIndex);
                        if(cardParent.stationSource !== "") {
                        AudioPlayer.play(cardParent.stationSource);
                        }
                        if (!isMobile) {
                            stationInfo.open();
                        }
                    }

                    background: Kirigami.ShadowedRectangle {
                        Kirigami.Theme.inherit: false
                        Kirigami.Theme.colorSet: Kirigami.Theme.View
                        color: {
                            if (parent.hovered)
                                Kirigami.ColorUtils.tintWithAlpha(Kirigami.Theme.hoverColor, Kirigami.Theme.backgroundColor, 0.6);
                            else
                                Kirigami.ColorUtils.tintWithAlpha(Kirigami.Theme.hoverColor, Kirigami.Theme.backgroundColor, 0.8);
                        }

                        corners {
                            topLeftRadius: Kirigami.Units.smallSpacing
                            bottomLeftRadius: Kirigami.Units.smallSpacing
                            topRightRadius: Kirigami.Units.smallSpacing
                            bottomRightRadius: Kirigami.Units.smallSpacing
                        }
                    }

                    contentItem: Item {
                        Kirigami.Icon {
                            source: "media-playback-start"
                            implicitHeight: Kirigami.Units.iconSizes.small
                            implicitWidth: Kirigami.Units.iconSizes.small
                            anchors.centerIn: parent
                        }
                    }
                }
            }
        }
    }

    MouseArea {
        id: dragArea
        anchors.fill: parent
        z: 0
        anchors.rightMargin: playButton.width + Kirigami.Units.gridUnit * 2
        cursorShape: dragArea.held ? Qt.DragMoveCursor : Qt.ArrowCursor
        property bool held: false
        property int startIndex: -1
        property int currentDropIndex: -1  // Новое свойство для отслеживания текущей позиции
        property real startX: cardParent ? cardParent.x : 0
        property real startY: cardParent ? cardParent.y : 0
        property real originalX: 0
        property real originalY: 0

        onClicked: {
            view.currentIndex = model.index;
            if (pageStack.depth > 1)
                pageStack.pop();
            pageStack.push(Qt.resolvedUrl("StationInfoPage.qml"), {
                feed: cardParent.model,
                currentIndex: model.index,
                currentModel: currentModel
            });
        }

        onPressAndHold: {
            if (currentModel == StationSearchModel) {
                return;
            }
            held = true;
            cardLayout.checked = true;
            originalX = cardParent.x;
            originalY = cardParent.y;
            cardParent.z = 999;
            startIndex = model.index;
            currentDropIndex = startIndex;
        }

        onPositionChanged: function (mouse) {
            if (held) {
                // Получаем позицию мыши относительно GridView
                var mousePos = mapToItem(view.contentItem, mouse.x, mouse.y);

                var dropIndex;
                if (view.cellWidth) { // GridView
                    // Вычисляем приблизительную позицию по колонкам
                    var cellWidth = view.cellWidth;
                    var approximateColumn = Math.floor(mousePos.x / cellWidth);
                    var approximateRow = Math.floor(mousePos.y / view.cellHeight);

                    // Вычисляем предполагаемый индекс на основе количества колонок
                    var columnsPerRow = Math.floor(view.width / cellWidth);
                    dropIndex = (approximateRow * columnsPerRow) + approximateColumn;
                } else { // ListView
                    var approximateRow = Math.floor(mousePos.y / height);
                    dropIndex = approximateRow;
                }

                // Ограничиваем индекс размером модели
                dropIndex = Math.min(dropIndex, view.count - 1);
                dropIndex = Math.max(0, dropIndex);

                // Если позиция изменилась, перемещаем визуально
                if (dropIndex !== currentDropIndex) {
                    // Временно перемещаем элемент
                    currentModel.move(currentDropIndex, dropIndex, 1);
                    currentDropIndex = dropIndex;
                }
            }
        }

        onReleased: function (mouse) {
            cardParent.z = 0;
            cardLayout.checked = false;
            if (held) {
                held = false;

                // Получаем финальную позицию мыши
                var mousePos = mapToItem(view.contentItem, mouse.x, mouse.y);

                // Вычисляем финальный индекс
                var finalDropIndex;
                if (view.cellWidth) { // GridView
                    var cellWidth = view.cellWidth;
                    var approximateColumn = Math.floor(mousePos.x / cellWidth);
                    var approximateRow = Math.floor(mousePos.y / view.cellHeight);
                    var columnsPerRow = Math.floor(view.width / cellWidth);
                    finalDropIndex = (approximateRow * columnsPerRow) + approximateColumn;
                } else { // ListView
                    var approximateRow = Math.floor(mousePos.y / height);
                    finalDropIndex = approximateRow;
                }
                finalDropIndex = Math.min(finalDropIndex, view.count - 1);
                finalDropIndex = Math.max(0, finalDropIndex);

                // Если начальная и конечная позиции различаются
                if (startIndex !== finalDropIndex && startIndex !== -1) {
                    // Сначала возвращаем элемент в начальную позицию
                    if (currentDropIndex !== startIndex) {
                        currentModel.move(currentDropIndex, startIndex, 1);
                    }
                    // Затем перемещаем в финальную позицию
                    currentModel.move(startIndex, finalDropIndex, 1);
                } else {
                    // Если позиция не изменилась, возвращаем элемент на место
                    if (currentDropIndex !== startIndex) {
                        currentModel.move(currentDropIndex, startIndex, 1);
                    } else {
                        cardParent.x = originalX;
                        cardParent.y = originalY;
                    }
                }

                if (currentDropIndex == 0 && (view.columns || 1) == 1 && currentModel == StationDBModel) {
                    //fix for missing coordinates
                    currentModel.loadStations();
                }
            }
        }

        drag {
            target: held ? cardParent : undefined
            axis: Drag.XAndYAxis
            minimumX: 0
            maximumX: view.width - cardParent.width
            minimumY: 0
            filterChildren: true
        }
    }
    states: [
        State {
            name: "dragging"
            when: dragArea.held
            PropertyChanges {
                target: cardLayout
                //scale: 1.05
                opacity: 0.8
            }
        }
    ]

    transitions: [
        Transition {
            from: "*"
            to: "dragging"
            NumberAnimation {
                properties: "scale,opacity"
                duration: 200
            }
        }
    ]
    /*Rectangle {
        id: debugIndicator
        width: 10
        height: 10
        radius: 5
        color: "red"
        visible: dragArea.held
        x: dragArea.mouseX
        y: dragArea.mouseY
        z: 1000
    }*/
}
