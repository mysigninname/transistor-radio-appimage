/*
SPDX-FileCopyrightText: 2024 Yuri Saurov <dr@i-glu4it.ru>
SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef APIMANAGER_H
#define APIMANAGER_H

#include "StationInfo.h"
#include <QCache>
#include <QDnsLookup>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QList>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QRunnable>
#include <QTimer>

class ApiManager : public QObject
{
    Q_OBJECT

public:
    explicit ApiManager(QObject *parent = nullptr);
    ~ApiManager();

    void performSearch(const QString &text, int limit, int offset, bool reset, bool search);
    QString lookupServers();

Q_SIGNALS:
    void searchFinished(const QList<QVariantMap> &stationsData);
    void errorOccurred(const QString &error);
    void searchStarted();
    void parsingStarted();
    void parsingFinished();

private Q_SLOTS:
    void onReplyFinished();
    void onRetryTimeout();
    void onDnsLookupFinished();

private:
    void sendRequest(const QUrl &url);
    QList<QVariantMap> parseJsonResponse(const QByteArray &data);
    void retryRequest();
    void handleError(const QString &error);
    QUrl buildUrl();
    bool isListOfUUIDs(const QString &input);
    void switchToNextServer();

    QNetworkAccessManager *m_networkManager;
    QNetworkReply *m_reply;
    QDnsLookup *m_dns;
    QTimer *m_retryTimer;
    QTimer *m_timeoutTimer;

    QString m_currentText;
    int m_currentLimit;
    int m_currentOffset;
    bool m_currentReset;
    bool m_currentSearch;

    int m_retryCount;
    int m_maxRetries;
    int m_baseRetryDelay; // in milliseconds

    QCache<QString, QList<QVariantMap>> *m_cache;
    QString m_mainUrl;

    QList<QDnsServiceRecord> m_serverList;
    int m_currentServerIndex;
    int m_serverTries;
};

#endif // APIMANAGER_H