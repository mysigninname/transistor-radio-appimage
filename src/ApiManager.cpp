/*
SPDX-FileCopyrightText: 2024 Yuri Saurov <dr@i-glu4it.ru>
SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "ApiManager.h"
#include <QCoreApplication>
#include <QDateTime>
#include <QEventLoop>
#include <QFutureWatcher>
#include <QHostAddress>
#include <QLocale>
#include <QPromise>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QString>
#include <QUrlQuery>
#include <QtConcurrent>
#include <qdebug.h>
#include <qlogging.h>
#include <qobject.h>
#include <qvariant.h>

ApiManager::ApiManager(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_reply(nullptr)
    , m_dns(new QDnsLookup(this))
    , m_retryTimer(new QTimer(this))
    , m_timeoutTimer(new QTimer(this))
    , m_retryCount(0)
    , m_maxRetries(3)
    , m_baseRetryDelay(1000)
    , // 1 second
    m_cache(new QCache<QString, QList<QVariantMap>>())
    , m_currentServerIndex(0)
    , m_serverTries(0)
{
    m_cache->setMaxCost(100); // Max 100 cached results

    connect(m_retryTimer, &QTimer::timeout, this, &ApiManager::onRetryTimeout);
    connect(m_timeoutTimer, &QTimer::timeout, this, [this]() {
        if (m_reply) {
            m_reply->abort();
            handleError(QStringLiteral("Request timeout"));
        }
    });
    connect(m_dns, &QDnsLookup::finished, this, &ApiManager::onDnsLookupFinished);
}

ApiManager::~ApiManager()
{
    if (m_reply) {
        m_reply->abort();
        m_reply->deleteLater();
    }
    delete m_cache;
}

void ApiManager::performSearch(const QString &text, int limit, int offset, bool reset, bool search)
{
    m_currentText = text;
    m_currentLimit = limit;
    m_currentOffset = offset;
    m_currentReset = reset;
    m_currentSearch = search;

    // Check cache
    QString cacheKey = QStringLiteral("%1_%2_%3_%4_%5")
                           .arg(text,
                                QString::number(limit),
                                QString::number(offset),
                                reset ? QStringLiteral("1") : QStringLiteral("0"),
                                search ? QStringLiteral("1") : QStringLiteral("0"));
    QList<QVariantMap> *cachedResult = m_cache->object(cacheKey);
    if (cachedResult) {
        Q_EMIT searchFinished(*cachedResult);
        return;
    }

    Q_EMIT searchStarted();

    if (m_serverList.isEmpty()) {
        lookupServers();
    } else {
        sendRequest(buildUrl());
    }
}

QUrl ApiManager::buildUrl()
{
    QString source = QStringLiteral("https://") + m_mainUrl + QStringLiteral("/json/stations");
    if (m_currentSearch) {
        source += QStringLiteral("/search");
    }

    QUrlQuery query;
    if (m_currentSearch) {
        query.addQueryItem(QStringLiteral("name"), m_currentText);
    } else if (isListOfUUIDs(m_currentText)) {
        source += QStringLiteral("/byuuid");
        query.addQueryItem(QStringLiteral("uuids"), m_currentText);
    } else {
        query.addQueryItem(QStringLiteral("order"), m_currentText);
    }
    if (m_currentText == QStringLiteral("votes")) {
        query.addQueryItem(QStringLiteral("reverse"), QStringLiteral("true"));
    }
    query.addQueryItem(QStringLiteral("limit"), QString::number(m_currentLimit));
    query.addQueryItem(QStringLiteral("offset"), QString::number(m_currentOffset));
    query.addQueryItem(QStringLiteral("hidebroken"), QStringLiteral("true"));

    QUrl url(source);
    url.setQuery(query);
    return url;
}

bool ApiManager::isListOfUUIDs(const QString &input)
{
    QStringList uuidList = input.split(QStringLiteral("'"), Qt::SkipEmptyParts);

    for (const QString &uuidStr : uuidList) {
        QString trimmedUUID = uuidStr.trimmed();
        QUuid uuid(trimmedUUID);

        if (uuid.isNull()) {
            return false;
        }
    }

    return true;
}

void ApiManager::sendRequest(const QUrl &url)
{
    if (m_reply) {
        m_reply->abort();
        m_reply->deleteLater();
    }

    QNetworkRequest request(url);
    request.setTransferTimeout(10000); // 10 seconds timeout

    m_reply = m_networkManager->get(request);
    connect(m_reply, &QNetworkReply::finished, this, &ApiManager::onReplyFinished);

    m_timeoutTimer->start(15000); // 15 seconds total timeout
}

void ApiManager::onReplyFinished()
{
    m_timeoutTimer->stop();

    if (m_reply->error() == QNetworkReply::NoError) {
        QByteArray data = m_reply->readAll();
        m_retryCount = 0;
        m_serverTries = 0;

        // Run parsing in a separate thread using QPromise
        Q_EMIT parsingStarted();
        QPromise<QList<QVariantMap>> promise;
        QFuture<QList<QVariantMap>> future = promise.future();
        [[maybe_unused]] auto concurrentFuture = QtConcurrent::run([this, data, promise = std::move(promise)]() mutable {
            QList<QVariantMap> result = parseJsonResponse(data);
            promise.addResult(result);
            promise.finish();
        });
        QFutureWatcher<QList<QVariantMap>> *watcher = new QFutureWatcher<QList<QVariantMap>>(this);

        connect(watcher, &QFutureWatcher<QList<QVariantMap>>::finished, this, [this, watcher]() {
            QList<QVariantMap> stationsData = watcher->result();

            // Cache the result
            QString cacheKey = QStringLiteral("%1_%2_%3_%4_%5")
                                   .arg(m_currentText,
                                        QString::number(m_currentLimit),
                                        QString::number(m_currentOffset),
                                        m_currentReset ? QStringLiteral("1") : QStringLiteral("0"),
                                        m_currentSearch ? QStringLiteral("1") : QStringLiteral("0"));
            QList<QVariantMap> *cachedStationsData = new QList<QVariantMap>(stationsData);
            m_cache->insert(cacheKey, cachedStationsData, 1); // Cost 1

            Q_EMIT searchFinished(stationsData);
            Q_EMIT parsingFinished();
            watcher->deleteLater();
        });

        watcher->setFuture(future);
    } else if (m_reply->error() != QNetworkReply::OperationCanceledError) {
        retryRequest();
    }

    m_reply->deleteLater();
    m_reply = nullptr;
}

QList<QVariantMap> ApiManager::parseJsonResponse(const QByteArray &data)
{
    QJsonDocument jsonDocument = QJsonDocument::fromJson(data);
    QJsonArray results = jsonDocument.array();

    QList<QVariantMap> stationsData;
    for (const auto &result : results) {
        QJsonObject entry = result.toObject();
        QVariantMap stationData;
        stationData[QStringLiteral("stationName")] = entry[QStringLiteral("name")].toString();
        stationData[QStringLiteral("stationUuid")] = entry[QStringLiteral("stationuuid")].toString();
        stationData[QStringLiteral("stationImageSource")] = entry[QStringLiteral("favicon")].toString();
        if (entry[QStringLiteral("url_resolved")].toString().isEmpty()) {
            stationData[QStringLiteral("stationSource")] = entry[QStringLiteral("url")].toString();
        } else {
            stationData[QStringLiteral("stationSource")] = entry[QStringLiteral("url_resolved")].toString();
        }
        if (QLocale::territoryToString(QLocale::codeToTerritory(entry[QStringLiteral("countrycode")].toString())) == QStringLiteral("Default")) {
            stationData[QStringLiteral("stationCountry")] = QStringLiteral();
        } else {
            stationData[QStringLiteral("stationCountry")] =
                QLocale::territoryToString(QLocale::codeToTerritory(entry[QStringLiteral("countrycode")].toString()));
        }
        stationData[QStringLiteral("stationTags")] = entry[QStringLiteral("tags")].toString();
        stationData[QStringLiteral("stationLanguage")] = entry[QStringLiteral("language")].toString();
        stationData[QStringLiteral("stationVotes")] = entry[QStringLiteral("votes")].toInt();
        stationData[QStringLiteral("stationState")] = entry[QStringLiteral("state")].toString();
        stationData[QStringLiteral("stationBitrate")] = entry[QStringLiteral("bitrate")].toInt();
        stationData[QStringLiteral("stationCodec")] = entry[QStringLiteral("codec")].toString();
        stationData[QStringLiteral("stationHomepage")] = entry[QStringLiteral("homepage")].toString();
        stationData[QStringLiteral("stationIsLocal")] = false;
        stationsData << stationData;
    }

    return stationsData;
}

void ApiManager::retryRequest()
{
    if (m_retryCount < m_maxRetries) {
        m_retryCount++;
        int delay = m_baseRetryDelay * (1 << (m_retryCount - 1)); // Exponential backoff
        m_retryTimer->start(delay);
    } else {
        if (m_serverTries < m_serverList.size()) {
            switchToNextServer();
            sendRequest(buildUrl());
        } else {
            handleError(QStringLiteral("Max retries exceeded"));
        }
    }
}

void ApiManager::onRetryTimeout()
{
    sendRequest(buildUrl());
}

void ApiManager::handleError(const QString &error)
{
    Q_EMIT errorOccurred(error);
}

QString ApiManager::lookupServers()
{
    if (!m_mainUrl.isEmpty()) {
        return m_mainUrl;
    }

    m_dns->setType(QDnsLookup::SRV);
    m_dns->setName(QStringLiteral("_api._tcp.radio-browser.info"));
    m_dns->lookup();

    return QString(); // Will be set asynchronously
}

void ApiManager::onDnsLookupFinished()
{
    if (m_dns->error() != QDnsLookup::NoError) {
        handleError(QStringLiteral("DNS lookup failed"));
        return;
    }

    const auto records = m_dns->serviceRecords();
    if (records.isEmpty()) {
        handleError(QStringLiteral("No DNS records found"));
        return;
    }

    m_serverList = records;
    m_currentServerIndex = QRandomGenerator::global()->bounded(records.size());
    m_mainUrl = m_serverList[m_currentServerIndex].target();

    if (!m_currentText.isEmpty()) {
        sendRequest(buildUrl());
    }
}

void ApiManager::switchToNextServer()
{
    if (m_serverList.isEmpty()) {
        return;
    }

    m_currentServerIndex = (m_currentServerIndex + 1) % m_serverList.size();
    m_mainUrl = m_serverList[m_currentServerIndex].target();
    m_retryCount = 0;
    m_serverTries++;
}
