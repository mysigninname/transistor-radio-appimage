/*
SPDX-FileCopyrightText: 2024 Yuri Saurov <dr@i-glu4it.ru>
SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef NOTIFICATIONMANAGER_H
#define NOTIFICATIONMANAGER_H

#include <KNotification>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QPointer>
#include <QRunnable>
#include <QThreadPool>
#include <QUrl>

class NotificationManager : public QObject
{
    Q_OBJECT

public:
    explicit NotificationManager(QObject *parent = nullptr);
    ~NotificationManager();

    void showNotification(const QString &stationName, const QString &streamTitle, const QString &streamUrl, const QUrl &stationImage);

private Q_SLOTS:
    void showNotificationDelayed(const QString &stationName, const QString &streamTitle, const QString &streamUrl, const QUrl &stationImage);
};

#endif // NOTIFICATIONMANAGER_H