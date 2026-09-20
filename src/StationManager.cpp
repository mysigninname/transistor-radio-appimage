/*
SPDX-FileCopyrightText: 2024 Yuri Saurov <dr@i-glu4it.ru>
SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "StationManager.h"
#include <qlogging.h>

StationManager *StationManager::instance()
{
    static StationManager manager;
    return &manager;
}

StationManager::StationManager(QObject *parent)
    : QObject(parent)
    , m_currentStation(nullptr)
{
}

StationInfo *StationManager::currentStation() const
{
    return m_currentStation;
}

void StationManager::setCurrentStation(StationInfo *station)
{
    if (m_currentStation != station) {
        m_currentStation = station;
        Q_EMIT currentStationChanged();
    }
}

// Реализация методов кэширования
StationInfo *StationManager::getStationFromCache(const QString &uuid)
{
    return m_stationCache.value(uuid, nullptr);
}

void StationManager::addStationToCache(StationInfo *station)
{
    if (station && !station->stationUuid().isEmpty()) {
        m_stationCache[station->stationUuid()] = station;
    }
}

void StationManager::removeStationFromCache(const QString &uuid)
{
    m_stationCache.remove(uuid);
}

void StationManager::clearStationCache()
{
    m_stationCache.clear();
}