#include "StationDBModel.h"
#include "DatabaseManager.h"
#include "StationSearchModel.h"
#include <QCryptographicHash>
#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
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
        qCritical() << "Failed to open database";
        return;
    }
}

StationDBModel::~StationDBModel()
{
    DatabaseManager::instance().close();
}

bool StationDBModel::stationExists(const QString &uuid) const
{
    QSqlQuery query;
    query.prepare(QStringLiteral("SELECT COUNT(*) FROM stations WHERE stationUuid = :stationUuid"));
    query.bindValue(QStringLiteral(":stationUuid"), uuid);
    if (!query.exec()) {
        qCritical() << "Failed to check station existence:" << query.lastError().text();
        return false;
    }
    if (query.next()) {
        return query.value(0).toInt() > 0;
    }
    return false;
}

void StationDBModel::addStation(const QVariantMap &feed)
{
    const QString uuid = feed.value(QStringLiteral("stationUuid")).toString();
    const QString name = feed.value(QStringLiteral("stationName")).toString();
    if (uuid.isEmpty()) {
        qWarning() << "stationUuid cannot be empty.";
        return;
    }
    if (name.isEmpty()) {
        qWarning() << "stationName cannot be empty.";
        return;
    }
    if (findStationIndexByUuid(uuid) != -1) {
        qWarning() << "Station with UUID" << uuid << "already exists.";
        return;
    }

    QSqlQuery positionQuery(m_db);
    positionQuery.prepare(QStringLiteral("SELECT COALESCE(MAX(position), -1) FROM stations"));
    if (!positionQuery.exec() || !positionQuery.next()) {
        qCritical() << "Failed to get max position:" << positionQuery.lastError().text();
        return;
    }
    int newPosition = positionQuery.value(0).toInt() + 1;

    QSqlQuery query;
    query.prepare(
        QStringLiteral("INSERT INTO stations (stationName, stationUuid, stationImageSource, stationSource, stationCountry, stationTags, stationLanguage, "
                       "stationVotes, stationState, stationBitrate, stationCodec, stationHomepage, stationIsLocal, position)"
                       "VALUES (:stationName, :stationUuid, :stationImageSource, :stationSource, :stationCountry, :stationTags, :stationLanguage, "
                       ":stationVotes, :stationState, :stationBitrate, :stationCodec, :stationHomepage, :stationIsLocal, :position)"));
    query.bindValue(QStringLiteral(":stationName"), name);
    query.bindValue(QStringLiteral(":stationUuid"), uuid);
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
    query.bindValue(QStringLiteral(":position"), newPosition);

    if (!query.exec()) {
        qCritical() << "Failed to add station:" << query.lastError().text();
        return;
    }
    int id = query.lastInsertId().toInt();
    StationInfo *stationInfo = new StationInfo(this);
    stationInfo->setProperty("songIndex", id);
    stationInfo->setStationName(name);
    stationInfo->setStationUuid(uuid);
    stationInfo->setStationImageSource(QUrl(feed.value(QStringLiteral("stationImageSource")).toString()));
    stationInfo->setStationSource(QUrl(feed.value(QStringLiteral("stationSource")).toString()));
    stationInfo->setStationCountry(feed.value(QStringLiteral("stationCountry")).toString());
    stationInfo->setStationTags(feed.value(QStringLiteral("stationTags")).toString());
    stationInfo->setStationLanguage(feed.value(QStringLiteral("stationLanguage")).toString());
    stationInfo->setStationVotes(feed.value(QStringLiteral("stationVotes")).toInt());
    stationInfo->setStationState(feed.value(QStringLiteral("stationState")).toString());
    stationInfo->setStationBitrate(feed.value(QStringLiteral("stationBitrate")).toInt());
    stationInfo->setStationCodec(feed.value(QStringLiteral("stationCodec")).toString());
    stationInfo->setStationHomepage(feed.value(QStringLiteral("stationHomepage")).toString());
    stationInfo->setStationIsLocal(feed.value(QStringLiteral("stationIsLocal")).toBool());
    beginInsertRows(QModelIndex(), m_stationList.size(), m_stationList.size());
    m_stationList.append(stationInfo);
    endInsertRows();

    if (uuid == m_currentUuid) {
        m_stationExists = true;
        Q_EMIT stationExistsChanged();
    }
}

void StationDBModel::removeStation(const QString &uuid)
{
    QSqlQuery query;
    query.prepare(QStringLiteral("DELETE FROM stations WHERE stationUuid = :stationUuid"));
    query.bindValue(QStringLiteral(":stationUuid"), uuid);
    if (!query.exec()) {
        qCritical() << "Failed to remove station:" << query.lastError().text();
        return;
    }
    int index = findStationIndexByUuid(uuid);
    if (index != -1) {
        beginRemoveRows(QModelIndex(), index, index);
        m_stationList.removeAt(index);
        endRemoveRows();
    }

    if (uuid == m_currentUuid) {
        m_stationExists = false;
        Q_EMIT stationExistsChanged();
    }
}

void StationDBModel::loadStations()
{
    clearAll();
    QSqlQuery query(QStringLiteral("SELECT * FROM stations ORDER BY position ASC"));
    while (query.next()) {
        StationInfo *stationInfo = new StationInfo(this);
        stationInfo->setProperty("songIndex", query.value("id").toInt());
        stationInfo->setStationName(query.value("stationName").toString());
        stationInfo->setStationUuid(query.value("stationUuid").toString());
        stationInfo->setStationImageSource(QUrl(query.value("stationImageSource").toString()));
        stationInfo->setStationSource(QUrl(query.value("stationSource").toString()));
        stationInfo->setStationCountry(query.value("stationCountry").toString());
        stationInfo->setStationTags(query.value("stationTags").toString());
        stationInfo->setStationLanguage(query.value("stationLanguage").toString());
        stationInfo->setStationVotes(query.value("stationVotes").toInt());
        stationInfo->setStationState(query.value("stationState").toString());
        stationInfo->setStationBitrate(query.value("stationBitrate").toInt());
        stationInfo->setStationCodec(query.value("stationCodec").toString());
        stationInfo->setStationHomepage(query.value("stationHomepage").toString());
        stationInfo->setStationIsLocal(query.value("stationIsLocal").toBool());
        beginInsertRows(QModelIndex(), m_stationList.size(), m_stationList.size());
        m_stationList.append(stationInfo);
        endInsertRows();
    }

    if (!m_currentUuid.isEmpty()) {
        m_stationExists = stationExists(m_currentUuid);
        Q_EMIT stationExistsChanged();
    }
}

void StationDBModel::checkStationExists(const QString &uuid)
{
    m_currentUuid = uuid;
    m_stationExists = stationExists(uuid);
    Q_EMIT stationExistsChanged();
}

int StationDBModel::findStationIndexByUuid(const QString &uuid) const
{
    for (int i = 0; i < m_stationList.size(); ++i) {
        if (m_stationList[i]->stationUuid() == uuid) {
            return i;
        }
    }
    return -1;
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
        qDebug() << "Invalid move parameters";
        return;
    }

    int newTo = to;
    if (to > from) {
        newTo++;
    }

    if (!beginMoveRows(QModelIndex(), from, from + count - 1, QModelIndex(), newTo)) {
        qDebug() << "beginMoveRows failed";
        return;
    }
    auto item = m_stationList.takeAt(from);
    m_stationList.insert(to, item);
    endMoveRows();
    updateDatabaseOrder();
}

void StationDBModel::updateDatabaseOrder()
{
    m_db.transaction();

    QSqlQuery query(m_db);
    query.prepare(QStringLiteral("UPDATE stations SET position = :position WHERE stationUuid = :uuid"));

    for (int i = 0; i < m_stationList.size(); ++i) {
        query.bindValue(QStringLiteral(":position"), i);
        query.bindValue(QStringLiteral(":uuid"), m_stationList.at(i)->stationUuid());

        if (!query.exec()) {
            qDebug() << "Failed to update station position:" << query.lastError().text();
        }
    }

    m_db.commit();
}

bool StationDBModel::exportStations(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    QDir dir = fileInfo.dir();
    if (!dir.exists() && !dir.mkpath(dir.absolutePath())) {
        return false;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    QJsonArray stationsArray;
    QSqlQuery query(QStringLiteral("SELECT * FROM stations ORDER BY position ASC"), m_db);

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

    QJsonDocument doc(stationsArray);
    if (file.write(doc.toJson()) == -1) {
        return false;
    }
    return true;
}

void StationDBModel::updateStationsFromApi()
{
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral("SELECT stationUuid FROM stations WHERE stationIsLocal = false"));
    if (!query.exec()) {
        qCritical() << "Failed to get non-local stations:" << query.lastError().text();
        return;
    }
    // Collect UUIDs
    QStringList uuids;
    while (query.next()) {
        uuids << query.value("stationUuid").toString();
    }

    if (uuids.isEmpty()) {
        qDebug() << "No non-local stations to update";
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
                    qCritical() << "Failed to update station:" << updateQuery.lastError().text();
                    success = false;
                    break;
                }
            }

            if (success) {
                m_db.commit();
                // Reload the model data to reflect changes
                loadStations();
                qDebug() << "Successfully updated stations from API";
            } else {
                m_db.rollback();
                qCritical() << "Failed to update stations from API";
            }

            searchModel->deleteLater();
        }
    });


    QString uuidsParam = uuids.join(QStringLiteral(","));

    searchModel->getData(uuidsParam, uuids.size(), 0, true, false);
}
bool StationDBModel::importStations(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qCritical() << "Не удалось открыть файл для чтения:" << file.errorString();
        return false;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isArray()) {
        qCritical() << "Неверный формат файла: корневой элемент должен быть массивом";
        return false;
    }

    m_db.transaction();

    QSqlQuery clearQuery(QStringLiteral("DELETE FROM stations"), m_db);
    if (!clearQuery.exec()) {
        m_db.rollback();
        return false;
    }

    clearAll();

    QJsonArray stationsArray = doc.array();
    bool success = true;

    for (const QJsonValue &value : stationsArray) {
        if (!value.isObject()) {
            continue;
        }

        QJsonObject station = value.toObject();
        QVariantMap stationData;

        for (auto it = station.begin(); it != station.end(); ++it) {
            stationData[it.key()] = it.value().toVariant();
        }
        addStation(stationData);

        if (!m_db.isOpen()) {
            success = false;
            break;
        }
    }

    if (success) {
        m_db.commit();
        loadStations();
        return true;
    } else {
        m_db.rollback();
        return false;
    }
}
