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