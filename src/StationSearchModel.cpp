/*
SPDX-FileCopyrightText: 2024 Yuri Saurov <dr@i-glu4it.ru>
SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "StationSearchModel.h"
#include "StationInfo.h"
#include <QCoreApplication>
#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocale>
#include <QMetaObject>
#include <QRegularExpression>
#include <QString>
#include <QUrlQuery>
#include <qdebug.h>
#include <qlogging.h>
#include <qobject.h>
#include <qvariant.h>

Q_LOGGING_CATEGORY(transistorApi, "transistor.api")

QString StationSearchModel::lookupServers()
{
    return m_apiManager->lookupServers();
}

StationSearchModel::StationSearchModel(QObject *parent)
    : StationAbstractModel(parent)
{
    m_apiManager = new ApiManager(this);
    connect(m_apiManager, &ApiManager::searchFinished, this, &StationSearchModel::onSearchFinished);
    connect(m_apiManager, &ApiManager::errorOccurred, this, &StationSearchModel::onErrorOccurred);
    connect(m_apiManager, &ApiManager::searchStarted, this, &StationSearchModel::onSearchStarted);
}

void StationSearchModel::getData(const QString &text, const int &limit, const int &offset, const bool &reset, const bool &search)
{
    if (reset) {
        clearAll();
    }
    if (isSearching()) {
        return;
    }
    if (!text.trimmed().isEmpty()) {
        m_apiManager->performSearch(text, limit, offset, reset, search);
    }
}

bool StationSearchModel::isSearching() const
{
    return m_isSearching;
}

void StationSearchModel::setIsSearching(bool newIsSearching)
{
    if (m_isSearching == newIsSearching)
        return;
    m_isSearching = newIsSearching;
    Q_EMIT isSearchingChanged();
}

void StationSearchModel::onSearchFinished(const QList<QVariantMap> &stationsData)
{
    if (!stationsData.isEmpty()) {
        const auto rows = rowCount();
        beginInsertRows({}, rows, rows + stationsData.size() - 1);
        for (const QVariantMap &stationData : stationsData) {
            StationInfo *stationInfo = StationInfo::createFromData(stationData, this);
            m_stationList << stationInfo;
        }
        endInsertRows();
    }
    setIsSearching(false);
}

void StationSearchModel::onErrorOccurred([[maybe_unused]] const QString &error)
{
    setIsSearching(false);
}

void StationSearchModel::onSearchStarted()
{
    setIsSearching(true);
}
