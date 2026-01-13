/*
SPDX-FileCopyrightText: 2024 Yuri Saurov <dr@i-glu4it.ru>
SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "StreamReader.h"
#include <QBuffer>
#include <QDebug>
#include <QEventLoop>
#include <QIODevice>
#include <QMetaObject>
#include <QNetworkDiskCache>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QTemporaryFile>
#include <QTimer>
#include <qmediaplayer.h>

StreamReader::StreamReader(QObject *parent)
    : QObject(parent)
    , m_manager(new QNetworkAccessManager(this))
    , m_reply(nullptr)
    , m_icyMetaint(0)
    , m_metadataTimeoutTimer(new QTimer(this))
    , m_lastMetadataTime(0)
{
    m_metadataTimeoutTimer->setInterval(30000); // 30 seconds
    connect(m_metadataTimeoutTimer, &QTimer::timeout, this, &StreamReader::onMetadataTimeout);
}

StreamReader::~StreamReader()
{
    stopStream();
}

// RingBuffer implementation
StreamReader::RingBuffer::RingBuffer(int maxSize, int minSize, int maxMaxSize)
    : m_head(0)
    , m_tail(0)
    , m_size(0)
    , m_maxSize(maxSize)
    , m_minSize(minSize)
    , m_maxMaxSize(maxMaxSize)
{
    m_data.resize(maxSize);
}

void StreamReader::RingBuffer::append(const QByteArray &data)
{
    int dataSize = data.size();
    if (dataSize == 0)
        return;

    if (dataSize > m_maxSize) {
        // If data is too large, skip
        return;
    }

    // Ensure space
    if (m_size + dataSize > m_maxSize) {
        // Remove old data to make space
        int toRemove = m_size + dataSize - m_maxSize;
        take(toRemove);
    }

    for (int i = 0; i < dataSize; ++i) {
        m_data[m_tail] = data[i];
        m_tail = (m_tail + 1) % m_maxSize;
    }
    m_size += dataSize;

    // Dynamic resizing based on usage
    if (m_size > 0.8 * m_maxSize && m_maxSize < m_maxMaxSize) {
        int newSize = qMin(m_maxMaxSize, m_maxSize * 2);
        setMaxSize(newSize);
    } else if (m_size < 0.2 * m_maxSize && m_maxSize > m_minSize) {
        int newSize = qMax(m_minSize, m_maxSize / 2);
        setMaxSize(newSize);
    }
}

QByteArray StreamReader::RingBuffer::take(int size)
{
    if (size > m_size)
        size = m_size;
    QByteArray result;
    result.resize(size);

    for (int i = 0; i < size; ++i) {
        result[i] = m_data[m_head];
        m_head = (m_head + 1) % m_maxSize;
    }
    m_size -= size;

    return result;
}

int StreamReader::RingBuffer::size() const
{
    return m_size;
}

void StreamReader::RingBuffer::clear()
{
    m_head = 0;
    m_tail = 0;
    m_size = 0;
}

void StreamReader::RingBuffer::setMaxSize(int maxSize)
{
    if (maxSize < m_size) {
        // Shrink by removing old data
        int toRemove = m_size - maxSize;
        take(toRemove);
    }
    m_maxSize = maxSize;
    m_data.resize(maxSize);
    if (m_head >= maxSize)
        m_head = 0;
    if (m_tail >= maxSize)
        m_tail = 0;
}

bool StreamReader::RingBuffer::isEmpty() const
{
    return m_size == 0;
}

char StreamReader::RingBuffer::peek(int index) const
{
    if (index >= m_size)
        return 0;
    int pos = (m_head + index) % m_maxSize;
    return m_data[pos];
}

void StreamReader::onMetadataTimeout()
{
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    qint64 timeSinceLastMetadata = currentTime - m_lastMetadataTime;
    if (timeSinceLastMetadata > 60000) { // 1 minute
    }
}

void StreamReader::startStream(const QUrl &url)
{
    stopStream();

    QNetworkRequest request(url);
    request.setRawHeader("Icy-Metadata", "1");
    m_reply = m_manager->get(request);
    connect(m_reply, &QNetworkReply::readyRead, this, &StreamReader::onReadyRead);
    m_lastMetadataTime = QDateTime::currentMSecsSinceEpoch();
    m_metadataTimeoutTimer->start();
}

void StreamReader::stopStream()
{
    if (m_reply) {
        m_reply->disconnect();
        m_reply->deleteLater();
        m_reply = nullptr;
    }
    m_buffer.clear();
    m_icyMetaint = 0;
    m_metadataTimeoutTimer->stop();
}

void StreamReader::onReadyRead()
{
    if (m_reply->error() != QNetworkReply::NoError) {
        stopStream();
        return;
    }

    if (m_icyMetaint == 0) {
        m_icyMetaint = m_reply->rawHeader("icy-metaint").toInt();
        if (m_icyMetaint <= 0) {
            stopStream();
            return;
        }
    }

    QByteArray newData = m_reply->readAll();
    m_buffer.append(newData);

    while (m_buffer.size() > m_icyMetaint) {
        QByteArray audioData = m_buffer.take(m_icyMetaint);

        if (m_buffer.isEmpty()) {
            break;
        }

        quint8 lengthByte = static_cast<quint8>(m_buffer.peek(0));
        uint metaLength = lengthByte * 16;

        if (metaLength == 0) {
            // No metadata, discard length byte
            m_buffer.take(1);
        } else if (static_cast<uint>(m_buffer.size()) >= 1 + metaLength) {
            // Enough data for length byte + metadata
            m_buffer.take(1); // discard the length byte
            QByteArray metadata = m_buffer.take(metaLength);
            m_lastMetadataTime = QDateTime::currentMSecsSinceEpoch();
            // Parse metadata synchronously to ensure timely processing
            auto result = parseMetadataStatic(metadata);
            // Emit signal directly
            Q_EMIT metadataParsed(result.title, result.url, result.hasTitle, result.hasUrl);
        } else {
            // Do not take length byte, wait for more data
            break;
        }
    }
}

StreamReader::MetadataResult StreamReader::parseMetadataStatic(const QByteArray &metadata)
{
    MetadataResult result;
    // Check for corrupted data: if metadata is empty or all zeros, skip
    if (metadata.isEmpty() || metadata == QByteArray(metadata.size(), '\0')) {
        return result;
    }

    QByteArray data = metadata;

    // Clean data: remove after first null byte
    QByteArray cleanData = data;
    int nullPos = cleanData.indexOf('\0');
    if (nullPos != -1) {
        cleanData.truncate(nullPos);
    }

    // If cleanData is empty or invalid, skip
    if (cleanData.isEmpty()) {
        return result;
    }

    QString decoded = QString::fromUtf8(cleanData).trimmed();

    // Use regular expression to parse metadata like in YuRadio
    static QRegularExpression re(QStringLiteral("([a-zA-Z]+)='(.*?)';"));

    for (const QRegularExpressionMatch &match : re.globalMatch(decoded)) {
        QString key = match.captured(1);
        QString value = match.captured(2);

        if (key.compare(QStringLiteral("StreamTitle"), Qt::CaseInsensitive) == 0) {
            result.title = value.trimmed();
            result.hasTitle = true;
            // Remove surrounding quotes if present
            if ((result.title.startsWith(QStringLiteral("'")) && result.title.endsWith(QStringLiteral("'")))
                || (result.title.startsWith(QStringLiteral("\"")) && result.title.endsWith(QStringLiteral("\"")))) {
                result.title = result.title.mid(1, result.title.length() - 2);
            }
        } else if (key.compare(QStringLiteral("StreamUrl"), Qt::CaseInsensitive) == 0) {
            result.url = value.trimmed();
            result.hasUrl = true;
            // Remove surrounding quotes if present
            if ((result.url.startsWith(QStringLiteral("'")) && result.url.endsWith(QStringLiteral("'")))
                || (result.url.startsWith(QStringLiteral("\"")) && result.url.endsWith(QStringLiteral("\"")))) {
                result.url = result.url.mid(1, result.url.length() - 2);
            }
            // Validate URL
            if (!result.url.startsWith(QStringLiteral("http"), Qt::CaseInsensitive)) {
                result.url = QString();
            }
        }
    }

    return result;
}