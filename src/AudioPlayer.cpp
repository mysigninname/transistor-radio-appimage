/*
SPDX-FileCopyrightText: 2024 Yuri Saurov <dr@i-glu4it.ru>
SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "AudioPlayer.h"
#include "NotificationManager.h"
#include "StationManager.h"
#include "transistorconfig.h"
#include <QBuffer>
#include <QColor>
#include <QDebug>
#include <QDir>
#include <QEventLoop>
#include <QFont>
#include <QIODevice>
#include <QIcon>
#include <QMediaMetaData>
#include <QMetaObject>
#include <QNetworkDiskCache>
#include <QPainter>
#include <QPixmap>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QTemporaryFile>
#include <QThreadPool>
#include <QTimer>
#include <qmediaplayer.h>

AudioPlayer::AudioPlayer(QObject *parent)
    : QObject(parent)
    , m_audioOutput(new QAudioOutput(this))
    , m_streamReader(new StreamReader(this))
    , m_notificationManager(new NotificationManager(this))
{
    m_audioOutput.setDevice(QMediaDevices::defaultAudioOutput());
    m_mediaPlayer.setAudioOutput(&m_audioOutput);
    connect(&m_mediaPlayer, &QMediaPlayer::mediaStatusChanged, this, &AudioPlayer::onMediaStatusChanged);
    connect(&m_audioOutput, &QAudioOutput::mutedChanged, this, &AudioPlayer::mutedChanged);
    connect(this, &AudioPlayer::streamTitleChanged, this, &AudioPlayer::showNotification);
    connect(m_streamReader, &StreamReader::metadataParsed, this, &AudioPlayer::onMetadataParsed);
    connect(nullptr, &QMediaDevices::audioOutputsChanged, this, &AudioPlayer::onAudioOutputsChanged);
}

QString AudioPlayer::streamTitle() const
{
    return m_streamTitle;
}

void AudioPlayer::setStreamTitle(const QString &newStreamTitle)
{
    if (m_streamTitle != newStreamTitle) {
        m_streamTitle = newStreamTitle;
    }
    Q_EMIT streamTitleChanged();
}

void AudioPlayer::startStream(const QUrl &url)
{
    stopStream();
    m_streamReader->startStream(url);
}

void AudioPlayer::stopStream()
{
    setStreamTitle(QString());
    setStreamUrl(QString());
    Q_EMIT streamTitleChanged();
    Q_EMIT streamUrlChanged();
    m_streamReader->stopStream();
}

void AudioPlayer::setStreamUrl(const QString &newStreamUrl)
{
    if (m_streamUrl != newStreamUrl) {
        m_streamUrl = newStreamUrl;
    }
    Q_EMIT streamUrlChanged();
}

QString AudioPlayer::streamUrl() const
{
    return m_streamUrl;
}

void AudioPlayer::showNotification()
{
    if (TransistorConfig::self()->showNotification() == false) {
        return;
    }
    if (m_streamTitle.isEmpty()) {
        return;
    }

    // Delay notification to allow streamUrl to be set
    QTimer::singleShot(1000, this, [this]() {
        showNotificationDelayed();
    });
}

void AudioPlayer::showNotificationDelayed()
{
    QString stationName = StationManager::instance()->currentStation()->stationName().trimmed();
    QString streamTitle = this->streamTitle();
    QString streamUrlStr = streamUrl();
    QUrl stationImage = StationManager::instance()->currentStation()->stationImageSource();

    m_notificationManager->showNotification(stationName, streamTitle, streamUrlStr, stationImage);
}

void AudioPlayer::onMetadataParsed(const QString &title, const QString &url, bool hasTitle, bool hasUrl)
{
    if (hasTitle) {
        setStreamTitle(title);
    }
    if (hasUrl) {
        setStreamUrl(url);
    }
}

QString AudioPlayer::errorString() const
{
    return m_errorString;
}

void AudioPlayer::setErrorString(const QString &newErrorString)
{
    m_errorString = newErrorString;
    Q_EMIT streamUrlChanged();
}

bool AudioPlayer::isMuted() const
{
    return m_audioOutput.isMuted();
}

void AudioPlayer::setMuted(bool muted)
{
    m_audioOutput.setMuted(muted);
}

void AudioPlayer::play(const QUrl &source)
{
    m_mediaPlayer.stop();
    m_mediaPlayer.setSource(source);
    m_mediaPlayer.play();
    startStream(source);
    m_playing = true;
    Q_EMIT playingChanged();
}

void AudioPlayer::stop()
{
    m_mediaPlayer.stop();
    stopStream();
    m_playing = false;
    Q_EMIT playingChanged();
}

float AudioPlayer::volume() const
{
    return m_audioOutput.volume();
}

void AudioPlayer::setVolume(float volume)
{
    m_audioOutput.setVolume(volume);
}

bool AudioPlayer::playing() const
{
    return m_playing;
}

bool AudioPlayer::mediaLoading() const
{
    return m_loadingMedia;
}

void AudioPlayer::onMediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    if (status == QMediaPlayer::LoadingMedia || status == QMediaPlayer::BufferingMedia || status == QMediaPlayer::StalledMedia) {
        m_loadingMedia = true;
    } else if (status == QMediaPlayer::InvalidMedia) {
        setErrorString(m_mediaPlayer.errorString());
        m_loadingMedia = false;
        m_mediaPlayer.setSource(QUrl());
        m_playing = false;
        Q_EMIT errorStringChanged();
        Q_EMIT playingChanged();

    } else {
        m_loadingMedia = false;
        Q_EMIT playingChanged();
    }
    Q_EMIT mediaLoadingChanged();
}

void AudioPlayer::onAudioOutputsChanged()
{
    updateAudioDevice();
}

void AudioPlayer::updateAudioDevice()
{
    const QAudioDevice defaultDevice = QMediaDevices::defaultAudioOutput();
    const QAudioDevice currentDevice = m_audioOutput.device();

    if (currentDevice == defaultDevice) {
        // Already using default device, no change needed
        return;
    }

    m_audioOutput.setDevice(defaultDevice);
    Q_EMIT deviceChanged();
}

QList<QAudioDevice> AudioPlayer::availableAudioDevices() const
{
    return QMediaDevices::audioOutputs();
}

QAudioDevice AudioPlayer::currentAudioDevice() const
{
    return m_audioOutput.device();
}

void AudioPlayer::setAudioDeviceById(const QString &id)
{
    const QList<QAudioDevice> devices = QMediaDevices::audioOutputs();
    for (const QAudioDevice &device : devices) {
        if (device.id() == id.toUtf8()) {
            m_audioOutput.setDevice(device);
            Q_EMIT deviceChanged();
            break;
        }
    }
}
