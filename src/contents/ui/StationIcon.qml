import QtQuick
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.components as Components

Item {
    id: root
    property string title
    property alias color: shadowRectangle.color
    property alias radius: shadowRectangle.radius
    property alias shadow: shadowRectangle.shadow
    property alias border: shadowRectangle.border
    property alias corners: shadowRectangle.corners
    property alias source: image.source
    property alias imageStatus: image.status
    property alias asynchronous: image.asynchronous
    property alias fillMode: image.fillMode
    property alias mipmap: image.mipmap
    property alias sourceSize: image.sourceSize
    property alias backgroundColor: colorRectangle.color
    property bool timeout: false

    Timer {
        id: timeoutTimer
        interval: 5000
        onTriggered: root.timeout = true
    }

    Image {
        id: image
        anchors.fill: parent
        visible: status == Image.Ready
        retainWhileLoading: true
        mipmap: true
        smooth: true
        cache: true
        
        onStatusChanged: {
            if (status === Image.Loading) {
                root.timeout = false
                timeoutTimer.restart()
            } else {
                timeoutTimer.stop()
                root.timeout = false
            }
        }
        onSourceChanged: {
            root.timeout = false
            timeoutTimer.stop()
        }
    }

    ShaderEffectSource {
        id: textureSource
        sourceItem: image
        hideSource: !shadowRectangle.softwareRendering
    }

    Kirigami.ShadowedTexture {
        id: shadowRectangle
        anchors.fill: parent
        source: (image.status === Image.Ready && !softwareRendering) ? textureSource : null
    }

    Kirigami.ShadowedRectangle {
        id: colorRectangle
        anchors.fill: parent
        color: Components.NameUtils.colorsFromString(root.title)
        visible: image.status == Image.Error || 
                image.status == Image.Null || 
                (image.status == Image.Loading && root.timeout)
        corners.topLeftRadius: shadowRectangle.corners.topLeftRadius
        corners.bottomLeftRadius: shadowRectangle.corners.bottomLeftRadius
        corners.topRightRadius: shadowRectangle.corners.topRightRadius
        corners.bottomRightRadius: shadowRectangle.corners.bottomRightRadius
        border.width: shadowRectangle.border.width
        border.color: shadowRectangle.border.color

        Text {
            anchors.fill: parent
            text: Components.NameUtils.initialsFromString(root.title)
            color: Kirigami.ColorUtils.brightnessForColor(Components.NameUtils.colorsFromString(root.title)) === Kirigami.ColorUtils.Light ? "black" : "white"
            font {
                pointSize: -1
                pixelSize: Math.round((root.height - Kirigami.Units.largeSpacing) / 2)
            }
            fontSizeMode: Text.Fit
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
        }
    }

    Controls.BusyIndicator {
        anchors.centerIn: parent
        visible: image.status === Image.Loading && !root.timeout
        running: visible
        width: Math.round((root.height /*- Kirigami.Units.largeSpacing*/) / 2)
        height: width
    }
}