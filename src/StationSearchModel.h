/*
SPDX-FileCopyrightText: 2024 Yuri Saurov <dr@i-glu4it.ru>
SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef STATIONSEARCHMODEL_H
#define STATIONSEARCHMODEL_H

#include "ApiManager.h"
#include "StationAbstractModel.h"
#include <QAbstractListModel>
#include <QCoreApplication>
#include <QLoggingCategory>
#include <QThread>

class StationSearchModel : public StationAbstractModel
{
    Q_OBJECT
    Q_PROPERTY(bool isSearching READ isSearching NOTIFY isSearchingChanged)

public:
    explicit StationSearchModel(QObject *parent = nullptr);

    bool isSearching() const;

    void setIsSearching(bool newIsSearching);
    bool isListOfUUIDs(const QString &input);

public Q_SLOTS:
    void getData(const QString &text, const int &limit, const int &offset, const bool &reset, const bool &search);
    QString lookupServers();
    void onSearchFinished(const QList<QVariantMap> &stationsData);
    void onErrorOccurred(const QString &error);
    void onSearchStarted();

Q_SIGNALS:
    void isSearchingChanged();

private:
    ApiManager *m_apiManager;
    bool m_isSearching = false;
};

Q_DECLARE_LOGGING_CATEGORY(transistorApi)

#endif // STATIONSEARCHMODEL_H
