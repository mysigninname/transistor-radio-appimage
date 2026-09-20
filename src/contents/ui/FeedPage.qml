import QtQuick
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami

Kirigami.ScrollablePage {
    id: root

    property int limit: 60
    property int offset: 0
    property string request

    Kirigami.Theme.colorSet: Kirigami.Theme.Window
}
