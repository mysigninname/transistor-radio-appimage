/*
SPDX-FileCopyrightText: 2024 Yuri Saurov <dr@i-glu4it.ru>
SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "SystrayIcon.h"
#include <qlogging.h>

#ifndef Q_OS_ANDROID
#include <QCoreApplication>
#include <QFile>
#include <QMenu>

#include "AudioPlayer.h"
#include "transistorconfig.h"
#include <KLocalizedString>

SystrayIcon *SystrayIcon::m_instance = nullptr;

SystrayIcon *SystrayIcon::instance()
{
    if (!m_instance) {
        m_instance = new SystrayIcon(QCoreApplication::instance());
    }
    return m_instance;
}

SystrayIcon::SystrayIcon(QObject *parent)
    : QObject(parent)
#ifndef Q_OS_ANDROID
    , m_trayIcon(this)
#endif
{
    qDebug() << "SystrayIcon constructor called";
    setIconColor(intToIconColorEnum(TransistorConfig::self()->trayIconType()));
    m_trayIcon.setToolTip(i18nc("@info:tooltip",
                                "Transistor"));

    QMenu *menu = new QMenu();

    connect(&m_trayIcon, &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::Trigger) {
            Q_EMIT raiseWindow();
        } else if (reason == QSystemTrayIcon::MiddleClick) {
            Q_EMIT playbackChanged();
        }
    });

    connect(TransistorConfig::self(), &TransistorConfig::showTrayIconChanged, this, [this]() {
        if (TransistorConfig::self()->showTrayIcon()) {
            m_trayIcon.show();
        } else {
            m_trayIcon.hide();
        }
    });

    connect(TransistorConfig::self(), &TransistorConfig::trayIconTypeChanged, this, [this]() {
        setIconColor(intToIconColorEnum(TransistorConfig::self()->trayIconType()));
    });

    QAction *raiseAction = new QAction(i18nc("@action:inmenu", "Show/Hide"), this);
    raiseAction->setIcon(QIcon::fromTheme(QStringLiteral("ru.transistor_radio.transistor")));

    connect(raiseAction, &QAction::triggered, QCoreApplication::instance(), [this]() {
        Q_EMIT raiseWindow();
    });
    menu->addAction(raiseAction);

    QAction *playAction = new QAction(i18nc("@action:inmenu", "Play/Stop"), this);
    playAction->setIcon(QIcon::fromTheme(QStringLiteral("media-playback-playing")));

    connect(playAction, &QAction::triggered, QCoreApplication::instance(), [this]() {
        Q_EMIT playbackChanged();
    });
    menu->addAction(playAction);

    QAction *mutedAction = new QAction(i18nc("@action:inmenu", "Mute/Unmute"), this);
    mutedAction->setIcon(QIcon::fromTheme(QStringLiteral("audio-volume-muted")));

    connect(mutedAction, &QAction::triggered, QCoreApplication::instance(), [this]() {
        Q_EMIT mutedChanged();
    });
    menu->addAction(mutedAction);

    // Quit
    QAction *quitAction = new QAction(i18nc("@action:inmenu", "Quit"), this);
    quitAction->setIcon(QIcon::fromTheme(QStringLiteral("application-exit")));

    connect(quitAction, &QAction::triggered, QCoreApplication::instance(), QCoreApplication::quit);
    menu->addAction(quitAction);

    m_trayIcon.setContextMenu(menu);

    if (TransistorConfig::self()->showTrayIcon()) {
        qDebug() << "Showing tray icon";
        m_trayIcon.show();
    } else {
        qDebug() << "Tray icon not shown because showTrayIcon is false";
    }
}

SystrayIcon::~SystrayIcon()
{
    if (m_instance == this) {
        m_instance = nullptr;
    }
}

#else
SystrayIcon::SystrayIcon(QObject *parent)
    : QObject(parent)
{
}

SystrayIcon::~SystrayIcon()
{
    if (m_instance == this) {
        m_instance = nullptr;
    }
}
#endif

bool SystrayIcon::available() const
{
#ifndef Q_OS_ANDROID
    bool available = QSystemTrayIcon::isSystemTrayAvailable();
    qDebug() << "SystrayIcon available:" << available;
    return available;
#else
    qDebug() << "SystrayIcon available: false (Android)";
    return false;
#endif
}

void SystrayIcon::setIconColor(SystrayIcon::IconColor iconColor)
{
#ifndef Q_OS_ANDROID
    QIcon icon;
    QString iconPath;
    switch (iconColor) {
    case SystrayIcon::IconColor::Colorful:
        iconPath = QStringLiteral(":/icons/ru.transistor_radio.transistor.svg");
        icon = QIcon(iconPath);
        break;
    case SystrayIcon::IconColor::Light:
        iconPath = QStringLiteral(":/icons/ru.transistor_radio.transistor-tray-light");
        icon = QIcon(iconPath);
        break;
    case SystrayIcon::IconColor::Dark:
        iconPath = QStringLiteral(":/icons/ru.transistor_radio.transistor-tray-dark");
        icon = QIcon(iconPath);
        break;
    }
    qDebug() << "Setting tray icon to:" << iconPath << "isNull:" << icon.isNull();
    m_trayIcon.setIcon(icon);
#endif
}

int SystrayIcon::iconColorEnumToInt(SystrayIcon::IconColor iconColor)
{
    switch (iconColor) {
    case SystrayIcon::IconColor::Light:
        return 1;
    case SystrayIcon::IconColor::Dark:
        return 2;
    case SystrayIcon::IconColor::Colorful:
    default:
        return 0;
    }
}

SystrayIcon::IconColor SystrayIcon::intToIconColorEnum(int iconColorCode)
{
    switch (iconColorCode) {
    case 1:
        return SystrayIcon::IconColor::Light;
    case 2:
        return SystrayIcon::IconColor::Dark;
    case 0:
    default:
        return SystrayIcon::IconColor::Colorful;
    }
}