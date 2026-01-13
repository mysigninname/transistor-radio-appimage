import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigamiaddons.settings as KirigamiSettings

KirigamiSettings.ConfigurationView {
    modules: [
        KirigamiSettings.ConfigurationModule {
            moduleId: "general"
            text: i18nc("@title:menu Category in settings", "General")
            icon.name: "ru.transistor_radio.transistor"
            page: () => Qt.createComponent("ru.transistor_radio.transistor", "GeneralSettingsPage")
        },
        KirigamiSettings.ConfigurationModule {
            moduleId: "aboutTransistor"
            text: i18nc("@title:menu Category in settings", "About Transistor")
            icon.name: "documentinfo"
            page: () => Qt.createComponent("org.kde.kirigamiaddons.formcard", "AboutPage")
            category: i18nc("@title:group", "About")
            initialProperties: () => {
           return {
               donateUrl: "https://yoomoney.ru/fundraise/1CUG0Q1I6HP.250922",
               getInvolvedUrl: "https://invent.kde.org/saurov/transistor"
           };
       }
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
