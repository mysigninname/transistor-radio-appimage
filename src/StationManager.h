/*
SPDX-FileCopyrightText: 2024 Yuri Saurov <dr@i-glu4it.ru>
SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef STATIONMANAGER_H
#define STATIONMANAGER_H

#include "StationInfo.h"
#include <QHash>
#include <QObject>

class StationManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(StationInfo *currentStation READ currentStation WRITE setCurrentStation NOTIFY currentStationChanged)

public:
    static StationManager *instance();

    StationInfo *currentStation() const;
    void setCurrentStation(StationInfo *station);

    // Кэширование станций
    StationInfo *getStationFromCache(const QString &uuid);
    void addStationToCache(StationInfo *station);
    void removeStationFromCache(const QString &uuid);
    void clearStationCache();

Q_SIGNALS:
    void currentStationChanged();

private:
    explicit StationManager(QObject *parent = nullptr);
    StationInfo *m_currentStation;
    QHash<QString, StationInfo *> m_stationCache; // Кэш для хранения объектов StationInfo по UUID
};

#endif // STATIONMANAGER_H