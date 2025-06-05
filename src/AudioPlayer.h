#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#include <QMediaPlayer>
#include <QMediaDevices>
#include <QAudioDevice>
#include <QAudioOutput>
#include <KNotification>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>
#include <QObject>
#include <QQmlEngine>
#include <QJSEngine>
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
    void parseMetadata(const QByteArray &metadata);
    

Q_SIGNALS:
    void playingChanged();
    void mediaLoadingChanged();
    void mutedChanged();
    void streamTitleChanged();
    void streamUrlChanged();
    void errorStringChanged();
    

private Q_SLOTS:
    void onMediaStatusChanged(QMediaPlayer::MediaStatus status);
    void onReadyRead();
    void showNotification();

private:
    QMediaPlayer m_mediaPlayer;
    QAudioOutput m_audioOutput;
    bool m_playing = false;
    bool m_loadingMedia = false;
    QNetworkAccessManager *m_manager;
    QNetworkReply *m_reply;
    QByteArray m_buffer;
    int m_icyMetaint;
    QString m_streamTitle;
    QString m_streamUrl;
    QString m_errorString;
};

#endif // AUDIOPLAYER_H