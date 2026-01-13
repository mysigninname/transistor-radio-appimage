/*
SPDX-FileCopyrightText: 2024 Yuri Saurov <dr@i-glu4it.ru>
SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "StationDBModel.h"
#include "DatabaseManager.h"
#include "StationSearchModel.h"
#include <QCryptographicHash>
#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMetaObject>
#include <QSqlError>
#include <QSqlQuery>
#include <QUuid>
#include <qdir.h>

StationDBModel::StationDBModel(QObject *parent)
    : StationAbstractModel(parent)
    , m_db(DatabaseManager::instance().database())
    , m_stationExists(false)
{
    if (!m_db.isOpen() && !DatabaseManager::instance().open()) {
        return;
    }
}

StationDBModel::~StationDBModel()
{
    DatabaseManager::instance().close();
}

void StationDBModel::clearAll()
{
    StationAbstractModel::clearAll();
    m_uuidIndex.clear();
}

bool StationDBModel::stationExists(const QString &uuid) const
{
    QSqlQuery query;
    query.prepare(QStringLiteral("SELECT COUNT(*) FROM stations WHERE stationUuid = :stationUuid"));
    query.bindValue(QStringLiteral(":stationUuid"), uuid);
    if (!query.exec()) {
        return false;
    }
    if (query.next()) {
        return query.value(0).toInt() > 0;
    }
    return false;
}

int StationDBModel::getNextPosition()
{
    QSqlQuery positionQuery(m_db);
    positionQuery.prepare(QStringLiteral("SELECT COALESCE(MAX(position), -1) FROM stations"));
    if (!positionQuery.exec() || !positionQuery.next()) {
        return -1;
    }
    return positionQuery.value(0).toInt() + 1;
}

int StationDBModel::insertStationIntoDB(const QVariantMap &feed, int position)
{
    QSqlQuery query;
    query.prepare(
        QStringLiteral("INSERT INTO stations (stationName, stationUuid, stationImageSource, stationSource, stationCountry, stationTags, stationLanguage, "
                       "stationVotes, stationState, stationBitrate, stationCodec, stationHomepage, stationIsLocal, position)"
                       "VALUES (:stationName, :stationUuid, :stationImageSource, :stationSource, :stationCountry, :stationTags, :stationLanguage, "
                       ":stationVotes, :stationState, :stationBitrate, :stationCodec, :stationHomepage, :stationIsLocal, :position)"));
    query.bindValue(QStringLiteral(":stationName"), feed.value(QStringLiteral("stationName")).toString());
    query.bindValue(QStringLiteral(":stationUuid"), feed.value(QStringLiteral("stationUuid")).toString());
    query.bindValue(QStringLiteral(":stationImageSource"), feed.value(QStringLiteral("stationImageSource")).toString());
    query.bindValue(QStringLiteral(":stationSource"), feed.value(QStringLiteral("stationSource")).toString());
    query.bindValue(QStringLiteral(":stationCountry"), feed.value(QStringLiteral("stationCountry")).toString());
    query.bindValue(QStringLiteral(":stationTags"), feed.value(QStringLiteral("stationTags")).toString());
    query.bindValue(QStringLiteral(":stationLanguage"), feed.value(QStringLiteral("stationLanguage")).toString());
    query.bindValue(QStringLiteral(":stationVotes"), feed.value(QStringLiteral("stationVotes")).toInt());
    query.bindValue(QStringLiteral(":stationState"), feed.value(QStringLiteral("stationState")).toString());
    query.bindValue(QStringLiteral(":stationBitrate"), feed.value(QStringLiteral("stationBitrate")).toInt());
    query.bindValue(QStringLiteral(":stationCodec"), feed.value(QStringLiteral("stationCodec")).toString());
    query.bindValue(QStringLiteral(":stationHomepage"), feed.value(QStringLiteral("stationHomepage")).toString());
    query.bindValue(QStringLiteral(":stationIsLocal"), feed.value(QStringLiteral("stationIsLocal")).toBool());
    query.bindValue(QStringLiteral(":position"), position);

    if (!query.exec()) {
        return -1;
    }
    return query.lastInsertId().toInt();
}

void StationDBModel::addStationToModel(StationInfo *stationInfo)
{
    beginInsertRows(QModelIndex(), m_stationList.size(), m_stationList.size());
    m_stationList.append(stationInfo);
    m_uuidIndex[stationInfo->stationUuid()] = m_stationList.size() - 1;
    endInsertRows();
}

void StationDBModel::addStation(const QVariantMap &feed)
{
    const QString uuid = feed.value(QStringLiteral("stationUuid")).toString();
    const QString name = feed.value(QStringLiteral("stationName")).toString();
    if (uuid.isEmpty()) {
        return;
    }
    if (name.isEmpty()) {
        return;
    }
    if (findStationIndexByUuid(uuid) != -1) {
        return;
    }

    int newPosition = getNextPosition();
    if (newPosition == -1) {
        return;
    }

    int id = insertStationIntoDB(feed, newPosition);
    if (id == -1) {
        return;
    }

    StationInfo *stationInfo = StationInfo::createFromData(feed, this, id);
    // Добавить в кэш
    StationManager::instance()->addStationToCache(stationInfo);
    addStationToModel(stationInfo);

    if (uuid == m_currentUuid) {
        m_stationExists = true;
        Q_EMIT stationExistsChanged();
    }
}

void StationDBModel::removeFromDB(const QString &uuid)
{
    QSqlQuery query;
    query.prepare(QStringLiteral("DELETE FROM stations WHERE stationUuid = :stationUuid"));
    query.bindValue(QStringLiteral(":stationUuid"), uuid);
    if (!query.exec()) { }
}

void StationDBModel::removeFromModel(int index)
{
    beginRemoveRows(QModelIndex(), index, index);
    m_stationList.removeAt(index);
    endRemoveRows();
}

void StationDBModel::rebuildUuidIndex()
{
    m_uuidIndex.clear();
    for (int i = 0; i < m_stationList.size(); ++i) {
        m_uuidIndex[m_stationList[i]->stationUuid()] = i;
    }
}

void StationDBModel::removeStation(const QString &uuid)
{
    removeFromDB(uuid);
    int index = findStationIndexByUuid(uuid);
    if (index != -1) {
        removeFromModel(index);
        rebuildUuidIndex();
    }
    // Удалить из кэша
    StationManager::instance()->removeStationFromCache(uuid);

    if (uuid == m_currentUuid) {
        m_stationExists = false;
        Q_EMIT stationExistsChanged();
    }
}

void StationDBModel::loadStations()
{
    if (m_loadWatcher.isRunning()) {
        return; // Already loading
    }

    connect(&m_loadWatcher, &QFutureWatcher<QList<QVariantMap>>::finished, this, &StationDBModel::onLoadStationsFinished, Qt::UniqueConnection);
    m_loadWatcher.setFuture(QtConcurrent::run([this]() {
        return loadStationsAsync();
    }));
}

QList<QVariantMap> StationDBModel::loadStationsAsync()
{
    QList<QVariantMap> stationsData;
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral("SELECT * FROM stations ORDER BY position ASC"));
    if (!query.exec()) {
        return stationsData;
    }
    while (query.next()) {
        QVariantMap stationData;
        stationData[QStringLiteral("id")] = query.value("id").toInt();
        stationData[QStringLiteral("stationName")] = query.value("stationName").toString();
        stationData[QStringLiteral("stationUuid")] = query.value("stationUuid").toString();
        stationData[QStringLiteral("stationImageSource")] = query.value("stationImageSource").toString();
        stationData[QStringLiteral("stationSource")] = query.value("stationSource").toString();
        stationData[QStringLiteral("stationCountry")] = query.value("stationCountry").toString();
        stationData[QStringLiteral("stationTags")] = query.value("stationTags").toString();
        stationData[QStringLiteral("stationLanguage")] = query.value("stationLanguage").toString();
        stationData[QStringLiteral("stationVotes")] = query.value("stationVotes").toInt();
        stationData[QStringLiteral("stationState")] = query.value("stationState").toString();
        stationData[QStringLiteral("stationBitrate")] = query.value("stationBitrate").toInt();
        stationData[QStringLiteral("stationCodec")] = query.value("stationCodec").toString();
        stationData[QStringLiteral("stationHomepage")] = query.value("stationHomepage").toString();
        stationData[QStringLiteral("stationIsLocal")] = query.value("stationIsLocal").toBool();
        stationsData.append(stationData);
    }
    return stationsData;
}

void StationDBModel::clearStations()
{
    clearAll();
    m_uuidIndex.clear();
}

void StationDBModel::processStationsData(const QList<QVariantMap> &stationsData)
{
    beginInsertRows(QModelIndex(), 0, stationsData.size() - 1);
    for (const QVariantMap &stationData : stationsData) {
        QString uuid = stationData.value(QStringLiteral("stationUuid")).toString();
        StationInfo *stationInfo = StationManager::instance()->getStationFromCache(uuid);
        if (!stationInfo) {
            // Создать новый объект в главном потоке
            stationInfo = StationInfo::createFromData(stationData, this, stationData.value(QStringLiteral("id")).toInt());
            // Добавить в кэш
            StationManager::instance()->addStationToCache(stationInfo);
        }
        m_stationList.append(stationInfo);
    }
    endInsertRows();
    // Заполнить индекс после загрузки
    m_uuidIndex.clear();
    for (int i = 0; i < m_stationList.size(); ++i) {
        m_uuidIndex[m_stationList[i]->stationUuid()] = i;
    }
}

void StationDBModel::updateStationExists()
{
    if (!m_currentUuid.isEmpty()) {
        m_stationExists = stationExists(m_currentUuid);
        Q_EMIT stationExistsChanged();
    }
}

void StationDBModel::onLoadStationsFinished()
{
    clearStations();
    QList<QVariantMap> stationsData = m_loadWatcher.result();
    if (!stationsData.isEmpty()) {
        processStationsData(stationsData);
    }
    updateStationExists();
}

void StationDBModel::checkStationExists(const QString &uuid)
{
    m_currentUuid = uuid;
    m_stationExists = stationExists(uuid);
    Q_EMIT stationExistsChanged();
}

int StationDBModel::findStationIndexByUuid(const QString &uuid) const
{
    return m_uuidIndex.value(uuid, -1);
}

QString StationDBModel::generateUuidFromSeed(const QString &seed)
{
    QByteArray hash = QCryptographicHash::hash(seed.toUtf8(), QCryptographicHash::Sha256);
    QUuid uuid = QUuid::fromRfc4122(hash.left(16));
    return uuid.toString(QUuid::WithoutBraces);
}

void StationDBModel::move(int from, int to, int count)
{
    if (from < 0 || from >= m_stationList.size() || to < 0 || to >= m_stationList.size() || from == to) {
        return;
    }

    int newTo = to;
    if (to > from) {
        newTo++;
    }

    if (!beginMoveRows(QModelIndex(), from, from + count - 1, QModelIndex(), newTo)) {
        return;
    }
    StationInfo *item = m_stationList.takeAt(from);
    m_stationList.insert(to, item);
    endMoveRows();
    // Перестроить индекс после перемещения
    m_uuidIndex.clear();
    for (int i = 0; i < m_stationList.size(); ++i) {
        m_uuidIndex[m_stationList[i]->stationUuid()] = i;
    }
    updateDatabaseOrder();
}

void StationDBModel::updateDatabaseOrder()
{
    if (m_stationList.isEmpty()) {
        return;
    }

    m_db.transaction();

    QSqlQuery query(m_db);
    query.prepare(QStringLiteral("UPDATE stations SET position = ? WHERE stationUuid = ?"));

    for (int i = 0; i < m_stationList.size(); ++i) {
        query.addBindValue(i);
        query.addBindValue(m_stationList.at(i)->stationUuid());
    }

    if (!query.execBatch()) {
        m_db.rollback();
        return;
    }

    m_db.commit();
}

QJsonArray StationDBModel::buildStationsJson()
{
    QJsonArray stationsArray;
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral("SELECT * FROM stations ORDER BY position ASC"));
    if (!query.exec()) {
        return stationsArray;
    }

    while (query.next()) {
        QJsonObject station;
        station[QStringLiteral("stationName")] = query.value(QStringLiteral("stationName")).toString();
        station[QStringLiteral("stationUuid")] = query.value(QStringLiteral("stationUuid")).toString();
        station[QStringLiteral("stationImageSource")] = query.value(QStringLiteral("stationImageSource")).toString();
        station[QStringLiteral("stationSource")] = query.value(QStringLiteral("stationSource")).toString();
        station[QStringLiteral("stationCountry")] = query.value(QStringLiteral("stationCountry")).toString();
        station[QStringLiteral("stationTags")] = query.value(QStringLiteral("stationTags")).toString();
        station[QStringLiteral("stationLanguage")] = query.value(QStringLiteral("stationLanguage")).toString();
        station[QStringLiteral("stationVotes")] = query.value(QStringLiteral("stationVotes")).toInt();
        station[QStringLiteral("stationState")] = query.value(QStringLiteral("stationState")).toString();
        station[QStringLiteral("stationBitrate")] = query.value(QStringLiteral("stationBitrate")).toInt();
        station[QStringLiteral("stationCodec")] = query.value(QStringLiteral("stationCodec")).toString();
        station[QStringLiteral("stationHomepage")] = query.value(QStringLiteral("stationHomepage")).toString();
        station[QStringLiteral("stationIsLocal")] = query.value(QStringLiteral("stationIsLocal")).toBool();
        station[QStringLiteral("position")] = query.value(QStringLiteral("position")).toInt();

        stationsArray.append(station);
    }

    return stationsArray;
}

bool StationDBModel::writeJsonToFile(const QString &filePath, const QJsonDocument &doc)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    if (file.write(doc.toJson()) == -1) {
        return false;
    }
    return true;
}

bool StationDBModel::exportStations(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    QDir dir = fileInfo.dir();
    if (!dir.exists() && !dir.mkpath(dir.absolutePath())) {
        return false;
    }

    QJsonArray stationsArray = buildStationsJson();
    QJsonDocument doc(stationsArray);
    return writeJsonToFile(filePath, doc);
}

void StationDBModel::updateStationsFromApi()
{
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral("SELECT stationUuid FROM stations WHERE stationIsLocal = false"));
    if (!query.exec()) {
        return;
    }
    // Collect UUIDs
    QStringList uuids;
    while (query.next()) {
        uuids << query.value("stationUuid").toString();
    }

    if (uuids.isEmpty()) {
        return;
    }

    StationSearchModel *searchModel = new StationSearchModel(this);

    // Connect to the isSearchingChanged signal to know when the data is ready
    connect(searchModel, &StationSearchModel::isSearchingChanged, this, [this, searchModel]() {
        if (!searchModel->isSearching()) {
            m_db.transaction();
            bool success = true;

            // Prepare update query
            QSqlQuery updateQuery(m_db);
            updateQuery.prepare(
                QStringLiteral("UPDATE stations SET "
                               "stationName = :stationName, "
                               "stationImageSource = :stationImageSource, "
                               "stationSource = :stationSource, "
                               "stationCountry = :stationCountry, "
                               "stationTags = :stationTags, "
                               "stationLanguage = :stationLanguage, "
                               "stationVotes = :stationVotes, "
                               "stationState = :stationState, "
                               "stationBitrate = :stationBitrate, "
                               "stationCodec = :stationCodec, "
                               "stationHomepage = :stationHomepage "
                               "WHERE stationUuid = :stationUuid"));

            // Process each station from the API response
            for (int i = 0; i < searchModel->rowCount(); ++i) {
                QModelIndex index = searchModel->index(i, 0);
                QString uuid = searchModel->data(index, StationAbstractModel::StationUuidRole).toString();

                // Update station data
                updateQuery.bindValue(QStringLiteral(":stationName"), searchModel->data(index, StationAbstractModel::StationNameRole));
                updateQuery.bindValue(QStringLiteral(":stationImageSource"), searchModel->data(index, StationAbstractModel::StationImageSourceRole));
                updateQuery.bindValue(QStringLiteral(":stationSource"), searchModel->data(index, StationAbstractModel::StationSourceRole));
                updateQuery.bindValue(QStringLiteral(":stationCountry"), searchModel->data(index, StationAbstractModel::StationCountryRole));
                updateQuery.bindValue(QStringLiteral(":stationTags"), searchModel->data(index, StationAbstractModel::StationTagsRole));
                updateQuery.bindValue(QStringLiteral(":stationLanguage"), searchModel->data(index, StationAbstractModel::StationLanguageRole));
                updateQuery.bindValue(QStringLiteral(":stationVotes"), searchModel->data(index, StationAbstractModel::StationVotesRole));
                updateQuery.bindValue(QStringLiteral(":stationState"), searchModel->data(index, StationAbstractModel::StationStateRole));
                updateQuery.bindValue(QStringLiteral(":stationBitrate"), searchModel->data(index, StationAbstractModel::StationBitrateRole));
                updateQuery.bindValue(QStringLiteral(":stationCodec"), searchModel->data(index, StationAbstractModel::StationCodecRole));
                updateQuery.bindValue(QStringLiteral(":stationHomepage"), searchModel->data(index, StationAbstractModel::StationHomepageRole));
                updateQuery.bindValue(QStringLiteral(":stationUuid"), uuid);

                if (!updateQuery.exec()) {
                    success = false;
                    break;
                }
            }

            if (success) {
                m_db.commit();
                // Reload the model data to reflect changes
                loadStations();
            } else {
                m_db.rollback();
            }

            searchModel->deleteLater();
        }
    });


    QString uuidsParam = uuids.join(QStringLiteral(","));

    searchModel->getData(uuidsParam, uuids.size(), 0, true, false);
}
QJsonArray StationDBModel::parseJsonFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return QJsonArray();
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isArray()) {
        return QJsonArray();
    }

    return doc.array();
}

void StationDBModel::clearStationsFromDB()
{
    if (!m_db.isOpen()) {
        if (!DatabaseManager::instance().open()) {
            return;
        }
    }

    QSqlQuery clearQuery(m_db);
    clearQuery.prepare(QStringLiteral("DELETE FROM stations"));
    if (!clearQuery.exec()) {
        qWarning() << "SQL Error:" << clearQuery.lastError().text();
    }
}

bool StationDBModel::batchInsertStations(const QJsonArray &stationsArray, int maxPosition)
{
    if (!m_db.isOpen()) {
        if (!DatabaseManager::instance().open()) {
            return false;
        }
    }

    // Проверить существование таблицы stations
    QSqlQuery checkQuery(m_db);
    checkQuery.prepare(QStringLiteral("SELECT name FROM sqlite_master WHERE type='table' AND name='stations'"));
    if (!checkQuery.exec() || !checkQuery.next()) {
        qWarning() << "Table 'stations' does not exist";
        return false;
    }

    QSqlQuery insertQuery(m_db);
    insertQuery.prepare(
        QStringLiteral("INSERT INTO stations (stationName, stationUuid, stationImageSource, stationSource, stationCountry, stationTags, stationLanguage, "
                       "stationVotes, stationState, stationBitrate, stationCodec, stationHomepage, stationIsLocal, position)"
                       "VALUES (:stationName, :stationUuid, :stationImageSource, :stationSource, :stationCountry, :stationTags, :stationLanguage, "
                       ":stationVotes, :stationState, :stationBitrate, :stationCodec, :stationHomepage, :stationIsLocal, :position)"));

    for (int i = 0; i < stationsArray.size(); ++i) {
        const QJsonValue &value = stationsArray.at(i);
        if (!value.isObject()) {
            qWarning() << "Skipping non-object value at index" << i;
            continue;
        }

        QJsonObject station = value.toObject();

        // Валидация ключевых полей
        QString stationName = station.value(QStringLiteral("stationName")).toString();
        QString stationUuid = station.value(QStringLiteral("stationUuid")).toString();
        QString stationSource = station.value(QStringLiteral("stationSource")).toString();

        if (stationName.isEmpty() || stationUuid.isEmpty() || stationSource.isEmpty()) {
            qWarning() << "Skipping station with missing required fields at index" << i << "name:" << stationName << "uuid:" << stationUuid
                       << "source:" << stationSource;
            continue;
        }

        // Проверить на null/undefined
        if (station.value(QStringLiteral("stationName")).isUndefined() || station.value(QStringLiteral("stationUuid")).isUndefined()
            || station.value(QStringLiteral("stationSource")).isUndefined()) {
            qWarning() << "Skipping station with undefined required fields at index" << i;
            continue;
        }

        insertQuery.bindValue(QStringLiteral(":stationName"), stationName);
        insertQuery.bindValue(QStringLiteral(":stationUuid"), stationUuid);
        insertQuery.bindValue(QStringLiteral(":stationImageSource"), station.value(QStringLiteral("stationImageSource")).toString());
        insertQuery.bindValue(QStringLiteral(":stationSource"), stationSource);
        insertQuery.bindValue(QStringLiteral(":stationCountry"), station.value(QStringLiteral("stationCountry")).toString());
        insertQuery.bindValue(QStringLiteral(":stationTags"), station.value(QStringLiteral("stationTags")).toString());
        insertQuery.bindValue(QStringLiteral(":stationLanguage"), station.value(QStringLiteral("stationLanguage")).toString());
        insertQuery.bindValue(QStringLiteral(":stationVotes"), station.value(QStringLiteral("stationVotes")).toInt());
        insertQuery.bindValue(QStringLiteral(":stationState"), station.value(QStringLiteral("stationState")).toString());
        insertQuery.bindValue(QStringLiteral(":stationBitrate"), station.value(QStringLiteral("stationBitrate")).toInt());
        insertQuery.bindValue(QStringLiteral(":stationCodec"), station.value(QStringLiteral("stationCodec")).toString());
        insertQuery.bindValue(QStringLiteral(":stationHomepage"), station.value(QStringLiteral("stationHomepage")).toString());
        insertQuery.bindValue(QStringLiteral(":stationIsLocal"), station.value(QStringLiteral("stationIsLocal")).toBool());
        insertQuery.bindValue(QStringLiteral(":position"), maxPosition + i + 1);

        qWarning() << "Inserting station:" << stationName << "UUID:" << stationUuid;

        if (!insertQuery.exec()) {
            qWarning() << "SQL Error for station" << stationName << ":" << insertQuery.lastError().text() << "Error type:" << insertQuery.lastError().type();
            return false;
        }
    }

    return true;
}

int StationDBModel::getMaxPositionFromDB()
{
    if (!m_db.isOpen()) {
        if (!DatabaseManager::instance().open()) {
            return -1;
        }
    }

    QSqlQuery positionQuery(m_db);
    positionQuery.prepare(QStringLiteral("SELECT COALESCE(MAX(position), -1) FROM stations"));
    if (!positionQuery.exec()) {
        qWarning() << "SQL Error:" << positionQuery.lastError().text();
        return -1;
    }
    if (!positionQuery.next()) {
        return -1;
    }
    return positionQuery.value(0).toInt();
}

bool StationDBModel::importStations(const QString &filePath)
{
    if (!m_db.isOpen()) {
        if (!DatabaseManager::instance().open()) {
            return false;
        }
    }

    QJsonArray stationsArray = parseJsonFile(filePath);
    if (stationsArray.isEmpty()) {
        return false;
    }

    qWarning() << "Starting import transaction";
    m_db.transaction();

    qWarning() << "Clearing stations from DB";
    clearStationsFromDB();

    clearAll();
    m_uuidIndex.clear();

    int maxPosition = getMaxPositionFromDB();
    qWarning() << "Max position from DB:" << maxPosition;

    if (!batchInsertStations(stationsArray, maxPosition)) {
        qWarning() << "Batch insert failed, rolling back transaction";
        m_db.rollback();
        return false;
    }

    qWarning() << "Committing transaction";
    m_db.commit();
    loadStations();
    return true;
}
