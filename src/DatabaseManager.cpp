/*
SPDX-FileCopyrightText: 2024 Yuri Saurov <dr@i-glu4it.ru>
SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "DatabaseManager.h"
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QSqlError>
#include <QSqlQuery>

Q_LOGGING_CATEGORY(transistorDb, "transistor.db")

DatabaseManager &DatabaseManager::instance()
{
    static DatabaseManager manager;
    return manager;
}

DatabaseManager::DatabaseManager()
{
    m_db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"));
    QString dbPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/stations.db");
    QDir dir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    if (!dir.exists()) {
        dir.mkpath(QStringLiteral("."));
    }
    m_db.setDatabaseName(dbPath);
}

DatabaseManager::~DatabaseManager()
{
    close();
}

bool DatabaseManager::open()
{
    if (!m_db.open()) {
        return false;
    }

    QSqlQuery query;
    query.exec(
        QStringLiteral("CREATE TABLE IF NOT EXISTS stations ("
                       "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                       "stationName TEXT NOT NULL,"
                       "stationUuid TEXT NOT NULL,"
                       "stationImageSource TEXT,"
                       "stationSource TEXT NOT NULL,"
                       "stationCountry TEXT,"
                       "stationTags TEXT,"
                       "stationLanguage TEXT,"
                       "stationVotes INTEGER,"
                       "stationState TEXT,"
                       "stationBitrate INTEGER,"
                       "stationCodec TEXT,"
                       "stationHomepage TEXT,"
                       "stationIsLocal INTEGER,"
                       "position INTEGER DEFAULT 0"
                       ");"));

    // Create indexes for frequently used fields
    query.exec(QStringLiteral("CREATE INDEX IF NOT EXISTS idx_stationUuid ON stations(stationUuid);"));
    query.exec(QStringLiteral("CREATE INDEX IF NOT EXISTS idx_position ON stations(position);"));
    query.exec(QStringLiteral("CREATE INDEX IF NOT EXISTS idx_stationIsLocal ON stations(stationIsLocal);"));

    return true;
}

void DatabaseManager::close()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
}

QSqlDatabase DatabaseManager::database() const
{
    return m_db;
}