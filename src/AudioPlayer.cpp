#include "AudioPlayer.h"
#include "StationManager.h"
#include "transistorconfig.h"
#include <KNotification>
#include <QMediaMetaData>
#include <QPixmap>
#include <QRegularExpression>
#include <QTimer>
#include <qdebug.h>
#include <qlogging.h>
#include <qmediaplayer.h>

AudioPlayer::AudioPlayer(QObject *parent)
    : QObject(parent)
    , m_audioOutput(new QAudioOutput(this))
    , m_manager(new QNetworkAccessManager(this))
    , m_reply(nullptr)
    , m_icyMetaint(0)
{
    const auto &audioOutputs = QMediaDevices::audioOutputs();
    if (!audioOutputs.isEmpty()) {
        m_audioOutput.setDevice(audioOutputs.first());
        m_mediaPlayer.setAudioOutput(&m_audioOutput);
    }
    connect(&m_mediaPlayer, &QMediaPlayer::mediaStatusChanged, this, &AudioPlayer::onMediaStatusChanged);
    connect(&m_audioOutput, &QAudioOutput::mutedChanged, this, &AudioPlayer::mutedChanged);
    connect(this, &AudioPlayer::streamTitleChanged, this, &AudioPlayer::showNotification);
}

QString AudioPlayer::streamTitle() const
{
    return m_streamTitle;
}

void AudioPlayer::setStreamTitle(const QString &newStreamTitle)
{
    if (m_streamTitle != newStreamTitle) {
        m_streamTitle = newStreamTitle;
        Q_EMIT streamTitleChanged();
    }
}

void AudioPlayer::startStream(const QUrl &url)
{
    stopStream();

    QNetworkRequest request(url);
    request.setRawHeader("Icy-Metadata", "1");
    m_reply = m_manager->get(request);
    connect(m_reply, &QNetworkReply::readyRead, this, &AudioPlayer::onReadyRead);
}

void AudioPlayer::stopStream()
{
    setStreamTitle(QString());
    setStreamUrl(QString());
    Q_EMIT streamTitleChanged();
    Q_EMIT streamUrlChanged();
    if (m_reply) {
        m_reply->disconnect();
        m_reply->deleteLater();
        m_reply = nullptr;
    }
    m_buffer.clear();
    m_icyMetaint = 0;
}

void AudioPlayer::setStreamUrl(const QString &newStreamUrl)
{
    if (m_streamUrl != newStreamUrl) {
        m_streamUrl = newStreamUrl;
        Q_EMIT streamUrlChanged();
    }
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

    KNotification *notification = new KNotification(QStringLiteral("songChanged"));
    notification->setTitle(StationManager::instance()->currentStation()->stationName().trimmed());
    notification->setText(streamTitle());

    QTimer::singleShot(500, this, [this, notification]() {
        QUrl imageUrl = !streamUrl().isEmpty() ? QUrl(streamUrl()) : StationManager::instance()->currentStation()->stationImageSource();

        if (!imageUrl.isEmpty()) {
            QNetworkRequest request(imageUrl);
            QNetworkReply *m_reply = m_manager->get(request);

            connect(m_reply, &QNetworkReply::finished, this, [notification, m_reply]() {
                if (m_reply->error() == QNetworkReply::NoError) {
                    QByteArray imageData = m_reply->readAll();
                    QPixmap pixmap;
                    if (pixmap.loadFromData(imageData)) {
                        notification->setPixmap(pixmap);
                    }
                }
                notification->sendEvent();
                m_reply->deleteLater();
            });
        } else {
            notification->setIconName(QStringLiteral("transistor"));
            notification->sendEvent();
        }
    });
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
void AudioPlayer::onReadyRead()
{
    if (m_reply->error() != QNetworkReply::NoError) {
        qDebug() << "Failed to connect to stream. Error:" << m_reply->errorString();
        stopStream();
        return;
    }

    if (m_icyMetaint == 0) {
        m_icyMetaint = m_reply->rawHeader("icy-metaint").toInt();
        if (m_icyMetaint <= 0) {
            qDebug() << "Icy-Metaint header not found or metaint is 0.";
            stopStream();
            return;
        }
    }
    m_buffer.append(m_reply->readAll());

    while (m_buffer.size() > m_icyMetaint) {
        QByteArray audioData = m_buffer.left(m_icyMetaint);

        m_buffer.remove(0, m_icyMetaint);

        if (m_buffer.isEmpty())
            break;

        uint metaLength = static_cast<uint>(m_buffer[0]) * 16;
        m_buffer.remove(0, 1);

        if (metaLength > 0 && m_buffer.size() >= metaLength) {
            QByteArray metadata = m_buffer.left(metaLength);
            if (metadata.size() > 1 || metadata[0] == '\0') {
                parseMetadata(metadata);
            }
            m_buffer.remove(0, metaLength);
        }
    }
}

void AudioPlayer::parseMetadata(const QByteArray &metadata)
{
    QByteArray cleanData = metadata;
    int nullPos = cleanData.indexOf('\0');
    if (nullPos != -1) {
        cleanData.truncate(nullPos);
    }

    QString decoded = QString::fromUtf8(cleanData).trimmed();

    QRegularExpression re(QStringLiteral(R"(StreamTitle=([\'"]?)(.*?)\1;)"),
                          QRegularExpression::CaseInsensitiveOption | QRegularExpression::DotMatchesEverythingOption);

    QRegularExpressionMatch match = re.match(decoded);
    if (match.hasMatch()) {
        QString title = match.captured(2).trimmed();
        title.remove(QRegularExpression(QStringLiteral("^['\"]|['\"]$")));
        setStreamTitle(title);

        QRegularExpression urlRe(QStringLiteral(R"(StreamUrl=([\'"]?)(.*?)\1;)"),
                                 QRegularExpression::CaseInsensitiveOption | QRegularExpression::DotMatchesEverythingOption);

        QRegularExpressionMatch urlMatch = urlRe.match(decoded);
        if (urlMatch.hasMatch()) {
            QString url = urlMatch.captured(2).trimmed();
            url.remove(QRegularExpression(QStringLiteral("^['\"]|['\"]$")));
            setStreamUrl(url.startsWith(QStringLiteral("http"), Qt::CaseInsensitive) ? url : QString());
        } else {
            setStreamUrl(QString());
        }
    }
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
