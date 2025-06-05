import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.transistor

Kirigami.ShadowedRectangle {
    height: Kirigami.Units.iconSizes.huge + Kirigami.Units.smallSpacing
    Kirigami.Theme.colorSet: Kirigami.Theme.View
    Kirigami.Theme.inherit: false
    color: openArea.containsMouse || openArea.pressed ? Kirigami.Theme.alternateBackgroundColor : Kirigami.Theme.backgroundColor
    RowLayout {
        anchors.fill: parent
        spacing: Kirigami.Units.largeSpacing
        clip: true

        StationIcon {
            id: stationIcon
            title: StationManager.currentStation.stationName
            width: Kirigami.Units.iconSizes.huge + Kirigami.Units.smallSpacing
            height: width
            color: "transparent"
            asynchronous: true
            fillMode: Image.PreserveAspectCrop
            source: TransistorConfig.showTrackImage && AudioPlayer.streamUrl ? AudioPlayer.streamUrl : StationManager.currentStation.stationImageSource
        }

        Column {
            Layout.fillWidth: true
            spacing: 0

            Layout.alignment: Qt.AlignVCenter
            clip: true

            Kirigami.Heading {
                Layout.fillWidth: true
                level: 2
                clip: true
                text: StationManager.currentStation.stationName.trim().replace(/\n/g, " ")
                elide: Text.ElideRight
                wrapMode: Text.Wrap
            }
            Controls.Label {
                id: label
                Layout.fillWidth: true
                visible: text.length > 0
                width: parent.width
                clip: true
                text: AudioPlayer.streamTitle
                elide: Text.ElideRight
                wrapMode: Text.Wrap
                maximumLineCount: 1
            }
        }

        Item {
            Layout.fillWidth: true
        }
        Controls.Button {
            id: playButton
            Layout.alignment: Qt.AlignRight
            Layout.rightMargin: Kirigami.Units.largeSpacing
            display: Controls.Button.IconOnly
            height: Kirigami.Units.iconSizes.large
            width: Kirigami.Units.iconSizes.large
            z: 1000
            onClicked: {
                AudioPlayer.playing ? AudioPlayer.stop() : AudioPlayer.play(StationManager.currentStation.stationSource);
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
    }
    MouseArea {
        id: openArea
        anchors.fill: parent
        hoverEnabled: true
        z: 999
        anchors.rightMargin: playButton.width + Kirigami.Units.gridUnit
        onClicked: {
            stationInfo.isOpened ? stationInfo.close() : stationInfo.open();
        }
    }
}
