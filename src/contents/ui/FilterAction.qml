import QtQuick
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami
import org.kde.transistor

Kirigami.Action {
    id: act
    property string title
    property string description
    property string label
    property string request
    displayHint: Kirigami.Settings.isMobile ? Kirigami.DisplayHint.IconOnly : Kirigami.DisplayHint.KeepVisible  
    icon.name: label
    text: title
    checkable: true
    checked: page.request == act.request;
    onTriggered: {
        page.offset = 0;
        page.limit = 60;
        page.request = act.request;
        page.title = description;
        StationSearchModel.getData(page.request, limit, 0, true, page.isSearch);
    }
}
