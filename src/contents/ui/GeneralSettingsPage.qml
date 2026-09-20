import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import ru.transistor_radio.transistor

FormCard.FormCardPage {
    id: root

    FormCard.FormHeader {
        title: i18nc("@title Form header for settings related to app appearance", "Appearance")
        Layout.fillWidth: true
    }

    FormCard.FormCard {
        Layout.fillWidth: true

        FormCard.FormComboBoxDelegate {
            id: interfaceMode
            text: i18nc("@label:listbox Label for selecting the type of GUI for the app (desktop, mobile or auto)", "Optimize interface for mobile/desktop usage")
            description: i18nc("@label", "This will require a restart of the app to take effect")

            textRole: "text"
            valueRole: "value"

            model: [
                {
                    text: i18nc("Label for the GUI mode selection", "Auto"),
                    value: 2
                },
                {
                    text: i18nc("Label for the GUI mode selection", "Desktop"),
                    value: 0
                },
                {
                    text: i18nc("Label for the GUI mode selection", "Mobile"),
                    value: 1
                }
            ]
            Component.onCompleted: currentIndex = indexOfValue(TransistorConfig.interfaceMode)
            onActivated: {
                TransistorConfig.interfaceMode = currentValue;
                TransistorConfig.save();
            }
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormComboBoxDelegate {
            id: colorTheme
            Layout.fillWidth: true
            text: i18nc("@label:listbox", "Color theme")
            textRole: "display"
            valueRole: "display"
            model: ColorSchemer.model
            Component.onCompleted: currentIndex = ColorSchemer.indexForScheme(TransistorConfig.colorScheme)
            onCurrentValueChanged: {
                ColorSchemer.apply(currentIndex);
                TransistorConfig.colorScheme = ColorSchemer.nameForIndex(currentIndex);
                TransistorConfig.save();
            }
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormSwitchDelegate {
            id: showTrackImage
            text: i18nc("@option:check", "Show track image instead of station image if possible")
            checked: TransistorConfig.showTrackImage
            onToggled: {
                TransistorConfig.showTrackImage = checked;
                TransistorConfig.save();
            }
        }
        FormCard.FormDelegateSeparator {}

        FormCard.FormSwitchDelegate {
            id: autoupdateInfo
            text: i18nc("@option:check", "Automatically update info of stations at startup")
            checked: TransistorConfig.autoupdateInfo
            onToggled: {
                TransistorConfig.autoupdateInfo = checked;
                TransistorConfig.save();
            }
        }
    }

    FormCard.FormHeader {
        title: i18nc("@title Title header for settings related to the notifications", "Notifications")
        Layout.fillWidth: true
    }

    FormCard.FormCard {
        Layout.fillWidth: true

        FormCard.FormSwitchDelegate {
            id: showNotification
            text: i18nc("@option:check", "Show a notification when a track is changed")
            checked: TransistorConfig.showNotification
            onToggled: {
                TransistorConfig.showNotification = checked;
                TransistorConfig.save();
            }
        }
    }

    FormCard.FormHeader {
        title: i18nc("@title Title header for settings related to the tray icon", "Tray icon")
        Layout.fillWidth: true
    }

    FormCard.FormCard {
        Layout.fillWidth: true

        Component.onCompleted: console.log("Tray icon settings visible:", SystrayIcon.available)

        FormCard.FormSwitchDelegate {
            id: showTrayIcon
            visible: SystrayIcon.available
            enabled: SystrayIcon.available
            text: i18nc("@option:check", "Show icon in system tray")
            checked: TransistorConfig.showTrayIcon
            onToggled: {
                TransistorConfig.showTrayIcon = checked;
                TransistorConfig.save();
            }
        }

        FormCard.FormSwitchDelegate {
            id: minimizeToTray
            visible: SystrayIcon.available
            enabled: TransistorConfig.showTrayIcon && SystrayIcon.available
            text: i18nc("@option:check", "Minimize to tray instead of closing")
            checked: TransistorConfig.minimizeToTray
            onToggled: {
                TransistorConfig.minimizeToTray = checked;
                TransistorConfig.save();
            }
        }

        FormCard.FormComboBoxDelegate {
            id: trayIconType
            visible: SystrayIcon.available
            enabled: TransistorConfig.showTrayIcon && SystrayIcon.available
            text: i18nc("@label:listbox Label for selecting the color of the tray icon", "Tray icon type")

            textRole: "text"
            valueRole: "value"

            model: [
                {
                    text: i18nc("Label describing style of tray icon", "Colorful"),
                    value: 0
                },
                {
                    text: i18nc("Label describing style of tray icon", "Light"),
                    value: 1
                },
                {
                    text: i18nc("Label describing style of tray icon", "Dark"),
                    value: 2
                }
            ]
            Component.onCompleted: currentIndex = indexOfValue(TransistorConfig.trayIconType)
            onActivated: {
                TransistorConfig.trayIconType = currentValue;
                TransistorConfig.save();
            }
        }
    }
}
