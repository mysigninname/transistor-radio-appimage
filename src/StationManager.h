#ifndef STATIONMANAGER_H
#define STATIONMANAGER_H

#include "StationInfo.h"
#include <QObject>

class StationManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(StationInfo *currentStation READ currentStation WRITE setCurrentStation NOTIFY currentStationChanged)

public:
    static StationManager *instance();

    StationInfo *currentStation() const;
    void setCurrentStation(StationInfo *station);

Q_SIGNALS:
    void currentStationChanged();

private:
    explicit StationManager(QObject *parent = nullptr);
    StationInfo *m_currentStation;
};

#endif // STATIONMANAGER_H