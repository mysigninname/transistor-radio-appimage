/*
SPDX-FileCopyrightText: 2024 Yuri Saurov <dr@i-glu4it.ru>
SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "NotificationManager.h"
#include "transistorconfig.h"
#include <KNotification>
#include <QBuffer>
#include <QColor>
#include <QDebug>
#include <QDir>
#include <QEventLoop>
#include <QFont>
#include <QGuiApplication>
#include <QIODevice>
#include <QIcon>
#include <QMetaObject>
#include <QNetworkDiskCache>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QString>
#include <QTemporaryFile>
#include <QThreadPool>
#include <QTimer>

NotificationManager::NotificationManager(QObject *parent)
    : QObject(parent)
{
}

NotificationManager::~NotificationManager()
{
}

void NotificationManager::showNotification(const QString &stationName, const QString &streamTitle, const QString &streamUrl, const QUrl &stationImage)
{
    if (TransistorConfig::self()->showNotification() == false) {
        return;
    }
    if (streamTitle.isEmpty()) {
        return;
    }

    // Delay notification to allow streamUrl to be set
    QTimer::singleShot(1000, this, [this, stationName, streamTitle, streamUrl, stationImage]() {
        showNotificationDelayed(stationName, streamTitle, streamUrl, stationImage);
    });
}

void NotificationManager::showNotificationDelayed(const QString &stationName, const QString &streamTitle, const QString &streamUrl, const QUrl &stationImage)
{
    auto notification = new KNotification(QStringLiteral("songChanged"));
    notification->setComponentName(QStringLiteral("transistor"));

    notification->setTitle(stationName.trimmed());
    notification->setText(streamTitle);

    QUrl imageUrl;
    if (TransistorConfig::self()->showTrackImage()) {
        imageUrl = !streamUrl.isEmpty() ? QUrl(streamUrl) : stationImage;
    } else {
        imageUrl = stationImage;
    }

    if (!imageUrl.isEmpty()) {
        qDebug() << "NotificationManager: Image URL:" << imageUrl;
        // Download image synchronously
        QNetworkAccessManager manager;
        QNetworkRequest request(imageUrl);
        QNetworkReply *reply = manager.get(request);
        QEventLoop loop;
        connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        loop.exec();

        QByteArray imageData;
        if (reply->error() == QNetworkReply::NoError) {
            imageData = reply->readAll();
        } else {
            qDebug() << "NotificationManager: Network error:" << reply->errorString();
        }
        reply->deleteLater();

        if (!imageData.isEmpty()) {
            qDebug() << "NotificationManager: Image data size:" << imageData.size();
            QImage image;
            if (image.loadFromData(imageData)) {
                qDebug() << "NotificationManager: Image loaded successfully, size:" << image.size() << "format:" << image.format();
                // Scale image to 64x64 for notification
                QImage scaledImage = image.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                // Ensure image has alpha channel
                if (scaledImage.format() != QImage::Format_ARGB32 && scaledImage.format() != QImage::Format_ARGB32_Premultiplied) {
                    scaledImage = scaledImage.convertToFormat(QImage::Format_ARGB32);
                }
                // Create rounded corners pixmap only if it's not the app icon
                bool isAppIcon = (imageUrl == QUrl(QStringLiteral("qrc:/icons/ru.transistor_radio.transistor.svg")));
                if (!isAppIcon) {
                    QPixmap roundedPixmap(64, 64);
                    roundedPixmap.fill(Qt::transparent);
                    QPainter painter(&roundedPixmap);
                    painter.setRenderHint(QPainter::Antialiasing);
                    QPainterPath path;
                    path.addRoundedRect(roundedPixmap.rect(), 8, 8);
                    painter.setClipPath(path);
                    painter.drawImage(0, 0, scaledImage);
                    painter.end();
                    qDebug() << "NotificationManager: Rounded pixmap created, size:" << roundedPixmap.size();
                    notification->setPixmap(roundedPixmap);
                } else {
                    // Use original scaled image for app icon
                    notification->setPixmap(QPixmap::fromImage(scaledImage));
                }
            } else {
                qDebug() << "NotificationManager: Failed to load image from data";
                // Fallback to app icon if image loading failed
                notification->setPixmap(QIcon(QStringLiteral(":/icons/ru.transistor_radio.transistor.svg")).pixmap(64, 64));
            }
        } else {
            qDebug() << "NotificationManager: No image data received";
            // Fallback to app icon if no image data
            notification->setPixmap(QIcon(QStringLiteral(":/icons/ru.transistor_radio.transistor.svg")).pixmap(64, 64));
        }
    } else {
        qDebug() << "NotificationManager: No image URL provided";
        // No image URL - use app icon
        notification->setPixmap(QIcon(QStringLiteral(":/icons/ru.transistor_radio.transistor.svg")).pixmap(64, 64));
    }

    // Set header icon - use theme icon if available, otherwise use path for flatpak
    /*QIcon icon = QIcon::fromTheme(QStringLiteral("ru.transistor_radio.transistor"));
    if (icon.isNull()) {
        notification->setIconName(QStringLiteral("/app/share/icons/hicolor/scalable/apps/ru.transistor_radio.transistor.svg"));
    } else {
        notification->setIconName(QStringLiteral("ru.transistor_radio.transistor"));
    }*/

    // notification->setIconName(QStringLiteral("kteatime"));
    // notification->setPixmap(QIcon(QStringLiteral(":/icons/ru.transistor_radio.transistor.svg")).pixmap(64, 64));
    // Send event after all settings are done
    notification->sendEvent();
}
