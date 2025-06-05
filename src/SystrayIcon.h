#pragma once

#include <QObject>
#include <QQmlEngine>
#ifndef Q_OS_ANDROID
#include <QSystemTrayIcon>
#endif

class SystrayIcon : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    enum IconColor {
        Colorful,
        Light,
        Dark,
    };
    Q_ENUM(IconColor)

    Q_PROPERTY(bool available READ available CONSTANT)

    static SystrayIcon *instance();
    static SystrayIcon *create(QQmlEngine *engine, QJSEngine *)
    {
        auto instance = SystrayIcon::instance();
        engine->setObjectOwnership(instance, QQmlEngine::CppOwnership);
        return instance;
    }

    [[nodiscard]] bool available() const;
    void setIconColor(IconColor iconColor);

Q_SIGNALS:
    void raiseWindow();
    void playbackChanged();
    void mutedChanged();

protected:
    explicit SystrayIcon(QObject *parent = nullptr);
    ~SystrayIcon() override;

private:
    static SystrayIcon* m_instance;
    
#ifndef Q_OS_ANDROID
    QSystemTrayIcon m_trayIcon;
#endif

    int iconColorEnumToInt(IconColor iconColor);
    IconColor intToIconColorEnum(int iconColorCode);
};