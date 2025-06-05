import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigamiaddons.settings as KirigamiSettings

KirigamiSettings.ConfigurationView {
    modules: [
        KirigamiSettings.ConfigurationModule {
            moduleId: "general"
            text: i18nc("@title:menu Category in settings", "General")
            icon.name: "transistor"
            page: () => Qt.createComponent("org.kde.transistor", "GeneralSettingsPage")
        },
        KirigamiSettings.ConfigurationModule {
            moduleId: "aboutKasts"
            text: i18nc("@title:menu Category in settings", "About Transistor")
            icon.name: "documentinfo"
            page: () => Qt.createComponent("org.kde.kirigamiaddons.formcard", "AboutPage")
            category: i18nc("@title:group", "About")
        },
        KirigamiSettings.ConfigurationModule {
            moduleId: "aboutKDE"
            text: i18nc("@title:menu Category in settings", "About KDE")
            icon.name: "kde-symbolic"
            page: () => Qt.createComponent("org.kde.kirigamiaddons.formcard", "AboutKDEPage")
            category: i18nc("@title:group", "About")
        }
    ]
}
