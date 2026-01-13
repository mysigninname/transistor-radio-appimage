/*
SPDX-FileCopyrightText: 2024 Yuri Saurov <dr@i-glu4it.ru>
SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "StationAbstractModel.h"
#include "StationInfo.h"
#include "StationManager.h"
#include <QMetaObject>
#include <QVariant>

StationAbstractModel::StationAbstractModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

StationAbstractModel::~StationAbstractModel()
{
}

void StationAbstractModel::clearAll()
{
    beginResetModel();
    m_stationList.clear();
    endResetModel();
}

int StationAbstractModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_stationList.size();
}

QVariant StationAbstractModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid() && index.row() >= 0 && index.row() < m_stationList.size()) {
        StationInfo *stationInfo = m_stationList[index.row()];
        switch ((Role)role) {
        case StationNameRole:
            return stationInfo->stationName();
        case StationUuidRole:
            return stationInfo->stationUuid();
        case StationImageSourceRole:
            return stationInfo->stationImageSource();
        case StationSourceRole:
            return stationInfo->stationSource();
        case StationCountryRole:
            return stationInfo->stationCountry();
        case StationTagsRole:
            return stationInfo->stationTags();
        case StationLanguageRole:
            return stationInfo->stationLanguage();
        case StationVotesRole:
            return stationInfo->stationVotes();
        case StationStateRole:
            return stationInfo->stationState();
        case StationBitrateRole:
            return stationInfo->stationBitrate();
        case StationCodecRole:
            return stationInfo->stationCodec();
        case StationHomepageRole:
            return stationInfo->stationHomepage();
        case StationIsLocalRole:
            return stationInfo->stationIsLocal();
        }
    }
    return {};
}

QHash<int, QByteArray> StationAbstractModel::roleNames() const
{
    QHash<int, QByteArray> names;
    names[StationNameRole] = "stationName";
    names[StationUuidRole] = "stationUuid";
    names[StationImageSourceRole] = "stationImageSource";
    names[StationSourceRole] = "stationSource";
    names[StationCountryRole] = "stationCountry";
    names[StationTagsRole] = "stationTags";
    names[StationLanguageRole] = "stationLanguage";
    names[StationVotesRole] = "stationVotes";
    names[StationStateRole] = "stationState";
    names[StationBitrateRole] = "stationBitrate";
    names[StationCodecRole] = "stationCodec";
    names[StationHomepageRole] = "stationHomepage";
    names[StationIsLocalRole] = "stationIsLocal";
    return names;
}

void StationAbstractModel::switchToStationByIndex(int index)
{
    if (index >= 0 && index < m_stationList.length()) {
        StationManager::instance()->setCurrentStation(m_stationList[index]);
    }
}