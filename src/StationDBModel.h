#ifndef STATIONDBMODEL_H
#define STATIONDBMODEL_H

#include "StationAbstractModel.h"
#include "StationManager.h"
#include <QSqlDatabase>
#include <QString>
#include <QVariantMap>

class StationDBModel : public StationAbstractModel
{
    Q_OBJECT

public:
    explicit StationDBModel(QObject *parent = nullptr);
    ~StationDBModel();

    Q_INVOKABLE bool stationExists(const QString &uuid) const;
    Q_PROPERTY(bool stationExists READ isStationExists NOTIFY stationExistsChanged)

    bool isStationExists() const
    {
        return m_stationExists;
    }

Q_SIGNALS:
    void stationExistsChanged();

public Q_SLOTS:
    void addStation(const QVariantMap &feed);
    void removeStation(const QString &uuid);
    void loadStations();
    void checkStationExists(const QString &uuid);
    Q_INVOKABLE void move(int from, int to, int count);
    void updateDatabaseOrder();
    int findStationIndexByUuid(const QString &uuid) const;
    QString generateUuidFromSeed(const QString &seed);
    Q_INVOKABLE bool exportStations(const QString &filePath);
    Q_INVOKABLE bool importStations(const QString &filePath);
    Q_INVOKABLE void updateStationsFromApi();

private:
    QSqlDatabase m_db;
    bool m_stationExists;
    QString m_currentUuid;
};

#endif // STATIONDBMODEL_H
