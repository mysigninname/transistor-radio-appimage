import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.transistor

ColumnLayout {
    width: parent.width
    Layout.alignment: Qt.AlignHCenter
    Layout.fillHeight: true
    spacing: Kirigami.Units.largeSpacing
    property QtObject feed: StationManager.currentStation

    onFeedChanged: feed ? StationDBModel.checkStationExists(feed.stationUuid) : ""
    z: 90
    Controls.ToolBar {
        Layout.fillWidth: true
        Layout.preferredHeight: pageStack.globalToolBar.preferredHeight
        leftPadding: isMobile ? 0 : Kirigami.Units.gridUnit

        contentItem: Kirigami.Heading {
            text: i18n("Current Station")
            verticalAlignment: Qt.AlignVCenter
            horizontalAlignment: isMobile ? Qt.AlignHCenter : Qt.AlignLeft
        }
    }

    StationIcon {
        title: !!StationManager.currentStation ? StationManager.currentStation.stationName : ""
        width: Kirigami.Units.iconSizes.enormous * 1.5
        height: width
        Layout.alignment: Qt.AlignHCenter
        radius: Kirigami.Units.largeSpacing
        Kirigami.Theme.colorSet: Kirigami.Theme.View
        Kirigami.Theme.inherit: false
        color: Kirigami.Theme.backgroundColor
        source: TransistorConfig.showTrackImage && AudioPlayer.streamUrl ? AudioPlayer.streamUrl : (StationManager.currentStation ? StationManager.currentStation.stationImageSource : "")
        sourceSize: Qt.size(Kirigami.Units.iconSizes.enormous * 1.5, Kirigami.Units.iconSizes.enormous * 1.5)
        fillMode: Image.PreserveAspectCrop
        border.width: 1
        border.color: Kirigami.ColorUtils.tintWithAlpha(Kirigami.Theme.backgroundColor, Kirigami.Theme.textColor, 0.2)
        corners.topLeftRadius: Kirigami.Units.largeSpacing
        corners.topRightRadius: Kirigami.Units.largeSpacing
        corners.bottomLeftRadius: Kirigami.Units.largeSpacing
        corners.bottomRightRadius: Kirigami.Units.largeSpacing

        shadow {
            size: Kirigami.Units.gridUnit
            color: Qt.rgba(0, 0, 0, 0.05)
            yOffset: 2
        }
    }

    Kirigami.Heading {
        Layout.alignment: Qt.AlignHCenter
        horizontalAlignment: Text.AlignHCenter
        Layout.maximumWidth: parent.width - Kirigami.Units.largeSpacing
        wrapMode: Text.WordWrap
        level: 2
        text: StationManager.currentStation ? StationManager.currentStation.stationName : ""
    }

    

    Kirigami.ShadowedRectangle {
        id: root

        Layout.alignment: Qt.AlignHCenter
        color: "transparent"
        height: Kirigami.Units.iconSizes.large
        width: Kirigami.Units.iconSizes.enormous * 1.5
        z: 10000
        shadow {
            size: Kirigami.Units.gridUnit
            color: Qt.rgba(0, 0, 0, 0.05)
            yOffset: 2
        }

        Controls.Button {
            id: leftButton

            anchors.left: root.left
            height: root.height
            width: root.height
            display: Controls.AbstractButton.IconOnly
            text: StationDBModel.stationExists ? i18n("Remove from Library") : i18n("Add to library")
            onClicked: {
                AudioPlayer.muted ? AudioPlayer.setMuted(false) : AudioPlayer.setMuted(true);
                TransistorConfig.isMuted = AudioPlayer.muted;
                TransistorConfig.save();
            }
            Component.onCompleted: {
                AudioPlayer.muted = TransistorConfig.isMuted;
            }
            z: 10000

            background: Kirigami.ShadowedRectangle {
                Kirigami.Theme.inherit: false
                Kirigami.Theme.colorSet: Kirigami.Theme.View
                color: {
                    if (parent.down || parent.visualFocus)
                        Kirigami.ColorUtils.tintWithAlpha(Kirigami.Theme.hoverColor, Kirigami.Theme.backgroundColor, 0.6);
                    else if (parent.hovered)
                        Kirigami.ColorUtils.tintWithAlpha(Kirigami.Theme.hoverColor, Kirigami.Theme.backgroundColor, 0.8);
                    else
                        Kirigami.Theme.backgroundColor;
                }

                corners {
                    topLeftRadius: Kirigami.Units.smallSpacing
                    bottomLeftRadius: Kirigami.Units.smallSpacing
                }

                border {
                    width: 1
                    color: Kirigami.ColorUtils.tintWithAlpha(color, Kirigami.Theme.textColor, 0.2)
                }

                Behavior on color {
                    enabled: true

                    ColorAnimation {
                        duration: Kirigami.Units.longDuration
                        easing.type: Easing.OutCubic
                    }
                }

                Behavior on border.color {
                    enabled: true

                    ColorAnimation {
                        duration: Kirigami.Units.longDuration
                        easing.type: Easing.OutCubic
                    }
                }
            }

            contentItem: Item {
                Kirigami.Icon {
                    implicitHeight: Kirigami.Units.iconSizes.medium
                    implicitWidth: Kirigami.Units.iconSizes.medium
                    source: {
                if (volumeSlider.value < 0.01 || TransistorConfig.isMuted)
                    return "audio-volume-muted";
                else if (volumeSlider.value < 0.3)
                    return "audio-volume-low";
                else if (volumeSlider.value > 0.7)
                    return "audio-volume-high";
                else
                    return "audio-volume-medium";
            }
                    anchors.centerIn: parent
                }
            }
        }

        Controls.Button {
            id: centerButton

            z: 10000
            anchors.left: leftButton.right
            anchors.leftMargin: -1
            anchors.rightMargin: -1
            anchors.right: rightButton.left
            onClicked: {
                AudioPlayer.playing ? AudioPlayer.stop() : AudioPlayer.play(StationManager.currentStation.stationSource);
            }
            height: root.height
            width: root.height * 2
            display: Controls.AbstractButton.IconOnly

            background: Kirigami.ShadowedRectangle {
                Kirigami.Theme.inherit: false
                Kirigami.Theme.colorSet: Kirigami.Theme.View
                color: {
                    if (parent.down || parent.visualFocus)
                        Kirigami.ColorUtils.tintWithAlpha(Kirigami.Theme.hoverColor, Kirigami.Theme.backgroundColor, 0.6);
                    else if (parent.hovered)
                        Kirigami.ColorUtils.tintWithAlpha(Kirigami.Theme.hoverColor, Kirigami.Theme.backgroundColor, 0.8);
                    else
                        Kirigami.Theme.backgroundColor;
                }

                border {
                    width: 1
                    color: Kirigami.ColorUtils.tintWithAlpha(color, Kirigami.Theme.textColor, 0.2)
                }

                Behavior on color {
                    enabled: true

                    ColorAnimation {
                        duration: Kirigami.Units.longDuration
                        easing.type: Easing.OutCubic
                    }
                }

                Behavior on border.color {
                    enabled: true

                    ColorAnimation {
                        duration: Kirigami.Units.longDuration
                        easing.type: Easing.OutCubic
                    }
                }
            }

            contentItem: Item {
                Kirigami.Icon {
                    implicitHeight: Kirigami.Units.iconSizes.medium
                    implicitWidth: Kirigami.Units.iconSizes.medium
                    source: AudioPlayer.playing ? "media-playback-stop" : "media-playback-start"
                    visible: !AudioPlayer.mediaLoading
                    anchors.centerIn: parent
                }

                Controls.BusyIndicator {
                    implicitHeight: Kirigami.Units.iconSizes.medium
                    implicitWidth: Kirigami.Units.iconSizes.medium
                    running: AudioPlayer.mediaLoading && AudioPlayer.playing
                    visible: running
                    anchors.centerIn: parent
                }
            }
        }

        Controls.Button {
            id: rightButton

            z: 10000
            anchors.right: root.right
            height: root.height
            width: root.height
            display: Controls.AbstractButton.IconOnly
            onClicked: {
                if (pageStack.currentItem && pageStack.currentItem.objectName === "stationInfoPage") {
                    pageStack.pop();
                }
                if (isMobile) {
                    stationInfo.close();
                }
                pageStack.push(Qt.resolvedUrl("StationInfoPage.qml"), {
                    feed: StationManager.currentStation,
                    objectName: "stationInfoPage",
                    currentModel: currentModel,
                    currentIndex: contextDrawer.currentIndex
                });
            }
            background: Kirigami.ShadowedRectangle {
                Kirigami.Theme.inherit: false
                Kirigami.Theme.colorSet: Kirigami.Theme.View
                color: {
                    if (parent.down || parent.visualFocus)
                        Kirigami.ColorUtils.tintWithAlpha(Kirigami.Theme.hoverColor, Kirigami.Theme.backgroundColor, 0.6);
                    else if (parent.hovered)
                        Kirigami.ColorUtils.tintWithAlpha(Kirigami.Theme.hoverColor, Kirigami.Theme.backgroundColor, 0.8);
                    else
                        Kirigami.Theme.backgroundColor;
                }

                corners {
                    topRightRadius: Kirigami.Units.smallSpacing
                    bottomRightRadius: Kirigami.Units.smallSpacing
                }

                border {
                    width: 1
                    color: Kirigami.ColorUtils.tintWithAlpha(color, Kirigami.Theme.textColor, 0.2)
                }

                Behavior on color {
                    enabled: true

                    ColorAnimation {
                        duration: Kirigami.Units.longDuration
                        easing.type: Easing.OutCubic
                    }
                }

                Behavior on border.color {
                    enabled: true

                    ColorAnimation {
                        duration: Kirigami.Units.longDuration
                        easing.type: Easing.OutCubic
                    }
                }
            }

            contentItem: Item {
                Kirigami.Icon {
                    implicitHeight: Kirigami.Units.iconSizes.medium
                    implicitWidth: Kirigami.Units.iconSizes.medium
                    source: "kdocumentinfo-symbolic"
                    anchors.centerIn: parent
                }
            }
        }
    }
    RowLayout {
        Layout.maximumWidth: Kirigami.Units.iconSizes.enormous * 1.5
        Layout.alignment: Qt.AlignHCenter
        Layout.fillWidth: true
        /*Controls.ToolButton {
            icon.name: {
                if (volumeSlider.value < 0.01 || TransistorConfig.isMuted)
                    return "audio-volume-muted";
                else if (volumeSlider.value < 0.3)
                    return "audio-volume-low";
                else if (volumeSlider.value > 0.7)
                    return "audio-volume-high";
                else
                    return "audio-volume-medium";
            }
            onClicked: {
                AudioPlayer.muted ? AudioPlayer.setMuted(false) : AudioPlayer.setMuted(true);
                TransistorConfig.isMuted = AudioPlayer.muted;
                TransistorConfig.save();
            }
            Component.onCompleted: {
                AudioPlayer.muted = TransistorConfig.isMuted;
            }
        }*/
        Controls.Slider {
            id: volumeSlider
            Layout.fillWidth: true
            snapMode: Controls.Slider.SnapAlways
            Layout.alignment: Qt.AlignHCenter
            orientation: Qt.Horizontal
            Layout.minimumWidth: 2
            wheelEnabled: true
            from: 0
            to: 1
            value: TransistorConfig.audioVolume
            onValueChanged: {
                AudioPlayer.setVolume(value);
                TransistorConfig.audioVolume = value;
                TransistorConfig.save();
            }
            Component.onCompleted: {
                AudioPlayer.setVolume(TransistorConfig.audioVolume);
            }
        }
        Controls.Label {
            //width: perecentWidth.width
            text: Math.round(volumeSlider.value * 100) + "%"
            Layout.minimumWidth: perecentWidth.width
            Layout.minimumHeight: perecentWidth.height
            horizontalAlignment: Text.AlignRight
        }
        TextMetrics {
            id: perecentWidth
            text: "199%"
        }
    }
    Controls.Label {
        Layout.alignment: Qt.AlignHCenter
        horizontalAlignment: Text.AlignHCenter
        Layout.maximumWidth: parent.width - Kirigami.Units.largeSpacing
        wrapMode: Text.WordWrap
        text: AudioPlayer.streamTitle
        visible: text.length > 0
    }
    Item {
        Layout.fillHeight: true
    }
}
