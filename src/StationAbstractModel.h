/*
SPDX-FileCopyrightText: 2024 Yuri Saurov <dr@i-glu4it.ru>
SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef STATIONABSTRACTMODEL_H
#define STATIONABSTRACTMODEL_H

#include "StationInfo.h"
#include <QAbstractListModel>

#include <qobject.h>
#include <qvariant.h>

class StationInfo;
class StationAbstractModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Role {
        StationNameRole = Qt::UserRole + 1,
        StationUuidRole,
        StationImageSourceRole,
        StationSourceRole,
        StationCountryRole,
        StationTagsRole,
        StationLanguageRole,
        StationVotesRole,
        StationStateRole,
        StationBitrateRole,
        StationCodecRole,
        StationHomepageRole,
        StationIsLocalRole
    };

    explicit StationAbstractModel(QObject *parent = nullptr);
    virtual ~StationAbstractModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual QHash<int, QByteArray> roleNames() const override;

public Q_SLOTS:
    void switchToStationByIndex(int index);
    void clearAll();

protected:
    QList<StationInfo *> m_stationList;
};

#endif // STATIONABSTRACTMODEL_H