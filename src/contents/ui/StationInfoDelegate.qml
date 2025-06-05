import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kirigami.private as KirigamiPrivate
import org.kde.transistor

FormCard.AbstractFormDelegate {
    required property string type
    required property string label
    required property string val
    property alias fontCapitalization: internalDescriptionItem.font.capitalization
    visible: val !== "" && val !== "0"
    background: null
    Layout.fillWidth: true
    horizontalPadding: Kirigami.Units.largeSpacing
    verticalPadding: isMobile ? Kirigami.Units.largeSpacing : Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing

    contentItem: GridLayout{
        Layout.fillWidth: true
        width: parent.width
       rowSpacing: 0
       columns: isMobile ? 1 : 2
       rows: isMobile ? 2 : 1

        Controls.Label {
            id: internalTextItem
            text: label + ":"
            elide: Text.ElideRight
            color: Kirigami.Theme.disabledTextColor
            font: Kirigami.Theme.smallFont
        }

        Controls.Label {
            id: internalDescriptionItem
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignLeft
            text: type == "count" ? val + " " + i18n("kBit/s") : val
            font.capitalization: type == "count" ? Font.Normal : Font.Capitalize
            wrapMode: Text.Wrap
        }
    }
}
