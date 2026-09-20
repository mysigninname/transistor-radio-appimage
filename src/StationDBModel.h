/*
SPDX-FileCopyrightText: 2024 Yuri Saurov <dr@i-glu4it.ru>
SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef STATIONDBMODEL_H
#define STATIONDBMODEL_H

#include "StationAbstractModel.h"
#include "StationManager.h"
#include <QCoreApplication>
#include <QFutureWatcher>
#include <QHash>
#include <QLoggingCategory>
#include <QSqlDatabase>
#include <QString>
#include <QThread>
#include <QVariantMap>
#include <QtConcurrent>

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
    void clearAll();

private Q_SLOTS:
    void addStationToModel(StationInfo *stationInfo);
    void removeFromModel(int index);
    void processStationsData(const QList<QVariantMap> &stationsData);

private:
    QList<QVariantMap> loadStationsAsync();
    void onLoadStationsFinished();
    void clearStations();
    void updateStationExists();
    int getNextPosition();
    int insertStationIntoDB(const QVariantMap &feed, int position);
    void removeFromDB(const QString &uuid);
    void rebuildUuidIndex();
    QJsonArray buildStationsJson();
    bool writeJsonToFile(const QString &filePath, const QJsonDocument &doc);
    QJsonArray parseJsonFile(const QString &filePath);
    void clearStationsFromDB();
    bool batchInsertStations(const QJsonArray &stationsArray, int maxPosition);
    int getMaxPositionFromDB();

    QSqlDatabase m_db;
    bool m_stationExists;
    QString m_currentUuid;
    QFutureWatcher<QList<QVariantMap>> m_loadWatcher;
    QHash<QString, int> m_uuidIndex;
};

Q_DECLARE_LOGGING_CATEGORY(transistorDb)

#endif // STATIONDBMODEL_H
