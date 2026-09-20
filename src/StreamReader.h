/*
SPDX-FileCopyrightText: 2024 Yuri Saurov <dr@i-glu4it.ru>
SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef STREAMREADER_H
#define STREAMREADER_H

#include <QLoggingCategory>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QTimer>
#include <QUrl>
#include <QVector>

class StreamReader : public QObject
{
    Q_OBJECT

public:
    explicit StreamReader(QObject *parent = nullptr);
    ~StreamReader();

    void startStream(const QUrl &url);
    void stopStream();

    struct MetadataResult {
        QString title;
        QString url;
        bool hasTitle = false;
        bool hasUrl = false;
    };

    static MetadataResult parseMetadataStatic(const QByteArray &metadata);

Q_SIGNALS:
    void metadataParsed(const QString &title, const QString &url, bool hasTitle, bool hasUrl);

private Q_SLOTS:
    void onReadyRead();
    void onMetadataTimeout();

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

    QNetworkAccessManager *m_manager;
    QNetworkReply *m_reply;
    RingBuffer m_buffer;
    int m_icyMetaint;
    QTimer *m_metadataTimeoutTimer;
    qint64 m_lastMetadataTime;
};

#endif // STREAMREADER_H