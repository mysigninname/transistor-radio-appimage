import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kirigamiaddons.components as Components
import org.kde.transistor

Kirigami.PromptDialog {
    id: addStationDialog
    title: i18n("Add Station")
    implicitWidth: Kirigami.Units.gridUnit * 25
    showCloseButton: true
    background: Kirigami.ShadowedRectangle {
        id: rect
        Kirigami.Theme.colorSet: Kirigami.Theme.Window
        Kirigami.Theme.inherit: false
        color: Kirigami.Theme.backgroundColor
        radius: Kirigami.Units.cornerRadius
        shadow {
            size: radius * 2
            color: Qt.rgba(0, 0, 0, 0.3)
            yOffset: 1
        }

        border {
            width: 1
            color: Kirigami.ColorUtils.linearInterpolation(Kirigami.Theme.backgroundColor, Kirigami.Theme.textColor, Kirigami.Theme.frameContrast)
        }
    }
    standardButtons: Kirigami.Dialog.NoButton
    property var regex: /^https?:\/\/[^\s/$.?#].[^\s]*$/i
    customFooterActions: [
        Kirigami.Action {
            text: i18n("Add Station")
            icon.name: "dialog-ok"
            onTriggered: {
                var nameIsValid = stationNameField.text.length > 0;
                var urlIsValid = addStationDialog.regex.test(stationUrlField.text);
                if (!urlIsValid || !nameIsValid) {
                    if (!nameIsValid) {
                        stationNameField.statusMessage = i18n("Station Name field cannot be empty.");
                    }
                    if (!urlIsValid) {
                        stationUrlField.statusMessage = i18n("URL is not valid");
                    }
                    return;
                } else {
                    var feed = {
                        "stationName": stationNameField.text,
                        "stationUuid": StationDBModel.generateUuidFromSeed(stationNameField.text),
                        "stationSource": stationUrlField.text,
                        "stationIsLocal": true
                    };
                    StationDBModel.addStation(feed);
                    addStationDialog.close();
                    showPassiveNotification(i18n("Station was added"));
                }
            }
        },
        Kirigami.Action {
            text: i18n("Cancel")
            icon.name: "dialog-cancel"
            onTriggered: {
                addStationDialog.close();
            }
        }
    ]
    ColumnLayout {
        spacing: Kirigami.Units.largeSpacing
        Kirigami.InlineMessage {
            Layout.fillWidth: true

            visible: true

            text: i18n("This station will only be added to your library. If you want it to be available to everyone, add it to a public database.")
        }

        StationIcon {
            title: stationNameField.text
            width: Kirigami.Units.iconSizes.enormous * 1.5
            height: width
            backgroundColor: stationNameField.text.length == 0 ? Kirigami.Theme.backgroundColor : Components.NameUtils.colorsFromString(title)
            Layout.alignment: Qt.AlignHCenter
            radius: Kirigami.Units.largeSpacing
            Kirigami.Theme.colorSet: Kirigami.Theme.View
            Kirigami.Theme.inherit: false
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
            Kirigami.PlaceholderMessage {
                width: parent.width - (Kirigami.Units.largeSpacing * 4)
                visible: stationNameField.text.length == 0
                explanation: "Image will be generated from name of the station"
                anchors.centerIn: parent
            }
        }

        FormCard.FormTextFieldDelegate {
            id: stationNameField
            label: i18n("Station Name:")
            leftPadding: 0
            rightPadding: 0
            statusMessage: ""
            status: Kirigami.MessageType.Error
            onTextChanged: statusMessage = ""
        }
        FormCard.FormTextFieldDelegate {
            id: stationUrlField
            label: i18n("Stream URL:")
            statusMessage: ""
            status: Kirigami.MessageType.Error
            leftPadding: 0
            rightPadding: 0
            onTextChanged: statusMessage = ""
        }
    }
}
