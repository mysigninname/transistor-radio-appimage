/*
SPDX-FileCopyrightText: 2024 Yuri Saurov <dr@i-glu4it.ru>
SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QLoggingCategory>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QString>

class DatabaseManager
{
public:
    static DatabaseManager &instance();
    ~DatabaseManager();

    bool open();
    void close();

    QSqlDatabase database() const;

private:
    DatabaseManager();
    DatabaseManager(const DatabaseManager &) = delete;
    DatabaseManager &operator=(const DatabaseManager &) = delete;

    QSqlDatabase m_db;
};

Q_DECLARE_LOGGING_CATEGORY(transistorDb)

#endif // DATABASEMANAGER_H