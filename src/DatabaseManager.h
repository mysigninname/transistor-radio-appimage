#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

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

#endif // DATABASEMANAGER_H