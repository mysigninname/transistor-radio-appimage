/*
SPDX-FileCopyrightText: 2024 Yuri Saurov <dr@i-glu4it.ru>
SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#include "NotificationManager.h"
#include "StreamReader.h"
#include <QAudioDevice>
#include <QAudioOutput>
#include <QJSEngine>
#include <QLoggingCategory>
#include <QMediaDevices>
#include <QMediaPlayer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QPointer>
#include <QQmlEngine>
#include <QRunnable>
#include <QThreadPool>
#include <QUrl>
#include <QVector>
#include <qtmetamacros.h>

class AudioPlayer : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool playing READ playing NOTIFY playingChanged)
    Q_PROPERTY(bool mediaLoading READ mediaLoading NOTIFY mediaLoadingChanged)
    Q_PROPERTY(bool muted READ isMuted WRITE setMuted NOTIFY mutedChanged)
    Q_PROPERTY(QString streamTitle READ streamTitle WRITE setStreamTitle NOTIFY streamTitleChanged)
    Q_PROPERTY(QString streamUrl READ streamUrl WRITE setStreamUrl NOTIFY streamUrlChanged)
    Q_PROPERTY(QString errorString READ errorString WRITE setErrorString NOTIFY errorStringChanged)

public:
    explicit AudioPlayer(QObject *parent = nullptr);

    Q_INVOKABLE void play(const QUrl &source);
    Q_INVOKABLE void stop();
    Q_INVOKABLE float volume() const;
    Q_INVOKABLE void setVolume(float volume);
    Q_INVOKABLE bool playing() const;
    Q_INVOKABLE bool mediaLoading() const;
    Q_INVOKABLE bool isMuted() const;
    Q_INVOKABLE void setMuted(bool muted);
    Q_INVOKABLE void startStream(const QUrl &url);
    Q_INVOKABLE void stopStream();
    QString streamTitle() const;
    QString streamUrl() const;
    QString errorString() const;
    void setStreamTitle(const QString &newStreamTitle);
    void setStreamUrl(const QString &newStreamUrl);
    void setErrorString(const QString &newErrorString);
    Q_INVOKABLE QList<QAudioDevice> availableAudioDevices() const;
    Q_INVOKABLE QAudioDevice currentAudioDevice() const;
    Q_INVOKABLE void setAudioDeviceById(const QString &id);

Q_SIGNALS:
    void playingChanged();
    void mediaLoadingChanged();
    void mutedChanged();
    void streamTitleChanged();
    void streamUrlChanged();
    void errorStringChanged();
    void deviceChanged();

private Q_SLOTS:
    void onMediaStatusChanged(QMediaPlayer::MediaStatus status);
    void showNotification();
    void showNotificationDelayed();
    void onMetadataParsed(const QString &title, const QString &url, bool hasTitle, bool hasUrl);
    void onAudioOutputsChanged();

private:
    class RingBuffer
    {
    public:
        RingBuffer(int maxSize = 1024 * 1024, int minSize = 64 * 1024, int maxMaxSize = 10 * 1024 * 1024); // Default 1MB, min 64KB, max 10MB
        void append(const QByteArray &data);
        QByteArray take(int size);
        int size() const;
        void clear();
        void setMaxSize(int maxSize);
        bool isEmpty() const;
        char peek(int index = 0) const;

    private:
        QVector<char> m_data;
        int m_head;
        int m_tail;
        int m_size;
        int m_maxSize;
        int m_minSize;
        int m_maxMaxSize;
    };

private:
    class MetadataParserRunnable : public QRunnable
    {
    public:
        MetadataParserRunnable(AudioPlayer *player, const QByteArray &metadata);
        void run() override;

    private:
        AudioPlayer *m_player;
        QByteArray m_metadata;
    };

    QMediaPlayer m_mediaPlayer;
    QAudioOutput m_audioOutput;
    bool m_playing = false;
    bool m_loadingMedia = false;
    StreamReader *m_streamReader;
    NotificationManager *m_notificationManager;
    QString m_streamTitle;
    QString m_streamUrl;
    QString m_errorString;

    void updateAudioDevice();
};

#endif // AUDIOPLAYER_H