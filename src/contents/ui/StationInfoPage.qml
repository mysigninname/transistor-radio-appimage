import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kirigami.private as KirigamiPrivate
import org.kde.transistor

Kirigami.ScrollablePage {
    id: infoDialog
    objectName: "stationInfoPage"

    property QtObject feed
    property int currentIndex
    property var currentModel
    property bool isCurrentStation: StationManager.currentStation && StationManager.currentStation.stationUuid == feed.stationUuid
    Component.onCompleted: {
        StationDBModel.checkStationExists(feed.stationUuid);
    }

    actions: [
        Kirigami.Action {
            text: isCurrentStation && AudioPlayer.playing ? i18n("Stop playing") : i18n("Play this station")
            icon.name: isCurrentStation && AudioPlayer.playing ? "media-playback-stop" : "media-playback-start"
            displayHint: Kirigami.Settings.isMobile ? Kirigami.DisplayHint.IconOnly : Kirigami.DisplayHint.KeepVisible
            onTriggered: {
                
                if (AudioPlayer.playing && isCurrentStation) {
                    AudioPlayer.stop()
                } else {
                currentModel.switchToStationByIndex(currentIndex);
                AudioPlayer.play(StationManager.currentStation.stationSource);
                }
            }
        },
        Kirigami.Action {
            text: StationDBModel.stationExists ? i18n("Remove from Library") : i18n("Add to library")
            icon.name: StationDBModel.stationExists ? "bookmark-remove-symbolic" : "bookmark-new-symbolic"
            displayHint: Kirigami.Settings.isMobile ? Kirigami.DisplayHint.IconOnly : Kirigami.DisplayHint.KeepVisible
            onTriggered: {
                if (StationDBModel.stationExists) {
                    deleteDialog.feed = feed;
                    deleteDialog.open();
                } else {
                    StationDBModel.addStation(feed);
                    showPassiveNotification(i18n("Station has been added"), 2000)
                }
                StationDBModel.checkStationExists(feed.stationUuid);
            }
        }
    ]

    title: feed ? feed.stationName : undefined
    ColumnLayout {
        id: view

        spacing: Kirigami.Units.largeSpacing

        Item {
            width: parent.width
            height: 0
        }

        StationIcon {
            title: feed ? feed.stationName : null
            width: Kirigami.Units.iconSizes.enormous * 1.5
            height: width
            Layout.alignment: Qt.AlignHCenter
            radius: Kirigami.Units.largeSpacing
            Kirigami.Theme.colorSet: Kirigami.Theme.View
            Kirigami.Theme.inherit: false
            color: Kirigami.Theme.backgroundColor
            source: feed ? feed.stationImageSource : null
            sourceSize: Qt.size(Kirigami.Units.iconSizes.enormous * 1.5, Kirigami.Units.iconSizes.enormous * 1.5)
            fillMode: Image.PreserveAspectFit
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
            clip: true
            Layout.maximumWidth: parent.width
            elide: Text.ElideRight
            text: feed ? feed.stationName : ""
        }

        Kirigami.UrlButton {
            Layout.alignment: Qt.AlignHCenter
            clip: true
            Layout.maximumWidth: parent.width
            elide: Text.ElideRight
            url: feed ? feed.stationHomepage : ""
        }

        FormCard.FormHeader {
            title: i18n("Info")
            visible: infoSection.visible
        }

        FormCard.FormCard {
            id: infoSection
            visible: lang.visible || tags.visible || votes.visible
            StationInfoDelegate {
                id: lang
                type: "text"
                label: i18n("Language")
                val: feed.stationLanguage
            }

            FormCard.FormDelegateSeparator {
                visible: lang.visible
            }

            StationInfoDelegate {
                id: tags
                type: "text"
                label: i18n("Tags")
                val: feed.stationTags.replace(/,/g, ", ")
                fontCapitalization: Font.Normal
            }

            FormCard.FormDelegateSeparator {
                visible: tags.visible
            }

            StationInfoDelegate {
                id: votes
                type: "text"
                label: i18n("Votes")
                val: feed.stationVotes
            }
        }

        FormCard.FormHeader {
            title: i18n("Location")
            visible: locationSection.visible
        }

        FormCard.FormCard {
            id: locationSection
            visible: country.visible || state.visible

            StationInfoDelegate {
                id: country
                type: "text"
                label: i18n("Country")
                val: feed.stationCountry
            }

            FormCard.FormDelegateSeparator {
                visible: country.visible && state.visible
            }

            StationInfoDelegate {
                id: state
                type: "text"
                label: i18n("State")
                val: feed.stationState
            }
        }

        FormCard.FormHeader {
            title: i18n("Station")
        }

        FormCard.FormCard {
            StationInfoDelegate {
                id: localStation
                visible: feed.stationIsLocal
                type: "text"
                label: i18n("Local station")
                val: i18n("This station exists only in your library and is not part of the public database.")
                fontCapitalization: Font.Normal
            }
            FormCard.FormDelegateSeparator {
                visible: localStation.visible
            }
            StationInfoDelegate {
                id: bitrate
                type: "count"
                label: i18n("Bitrate")
                val: feed.stationBitrate
            }

            FormCard.FormDelegateSeparator {
                visible: bitrate.visible
            }

            StationInfoDelegate {
                id: codec
                type: "text"
                label: i18n("Codec")
                val: feed.stationCodec
            }

            FormCard.FormDelegateSeparator {
                visible: codec.visible
            }

            FormCard.AbstractFormDelegate {
                background: null
                Layout.fillWidth: true
                horizontalPadding: Kirigami.Units.largeSpacing
                verticalPadding: Kirigami.Units.largeSpacing

                contentItem: RowLayout {
                    GridLayout {
                        Layout.fillWidth: true
                        width: parent.width
                        rowSpacing: 0
                        columns: isMobile ? 1 : 2
                        rows: isMobile ? 2 : 1

                        Controls.Label {
                            id: internalTextItem
                            Layout.fillWidth: false
                            text: i18n("Stream") + ":"
                            elide: Text.ElideRight
                            color: Kirigami.Theme.disabledTextColor
                            font: Kirigami.Theme.smallFont
                        }
                        Kirigami.UrlButton {
                            id: streamLink
                            url: feed.stationSource
                            Layout.fillWidth: true
                            acceptedButtons: Qt.LeftButton
                            horizontalAlignment: Text.AlignLeft
                            elide: Text.ElideRight
                        }
                    }
                    Controls.ToolButton {
                        icon.name: "edit-copy"
                        onClicked: {
                            KirigamiPrivate.CopyHelperPrivate.copyTextToClipboard(streamLink.url);
                            showPassiveNotification(i18n("The link is copied"));
                        }
                    }
                }
            }
        }

        Item {
            width: parent.width
            height: 0
        }
    }
    footer: Item {}
    background: Kirigami.ShadowedRectangle {
        id: rect

        Kirigami.Theme.colorSet: Kirigami.Theme.Window
        Kirigami.Theme.inherit: false
        color: Kirigami.Theme.backgroundColor
        radius: Kirigami.Units.smallSpacing

        shadow {
            size: radius * 2
            color: Qt.rgba(0, 0, 0, 0.3)
            yOffset: 1
        }
    }
}
