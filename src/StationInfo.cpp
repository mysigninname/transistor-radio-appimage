#include "StationInfo.h"

StationInfo::StationInfo(QObject *parent)
    : QObject{parent}
{
}

int StationInfo::songIndex() const
{
    return m_songIndex;
}

void StationInfo::setSongIndex(int newSongIndex)
{
    if (m_songIndex == newSongIndex)
        return;
    m_songIndex = newSongIndex;
    Q_EMIT songIndexChanged();
}

QString StationInfo::stationName() const
{
    return m_stationName;
}

void StationInfo::setStationName(const QString &newTitle)
{
    if (m_stationName == newTitle)
        return;
    m_stationName = newTitle;
    Q_EMIT stationNameChanged();
}

QString StationInfo::stationUuid() const
{
    return m_stationUuid;
}

void StationInfo::setStationUuid(const QString &uuid)
{
    if (m_stationUuid != uuid) {
        m_stationUuid = uuid;
        Q_EMIT stationUuidChanged();
    }
}

QUrl StationInfo::stationImageSource() const
{
    return m_stationImageSource;
}

void StationInfo::setStationImageSource(const QUrl &newImageSource)
{
    if (m_stationImageSource == newImageSource)
        return;
    m_stationImageSource = newImageSource;
    Q_EMIT stationImageSourceChanged();
}

QUrl StationInfo::stationSource() const
{
    return m_stationSource;
}

void StationInfo::setStationSource(const QUrl &newStationSource)
{
    if (m_stationSource == newStationSource)
        return;
    m_stationSource = newStationSource;
    Q_EMIT stationSourceChanged();
}

QString StationInfo::stationCountry() const
{
    return m_stationCountry;
}

void StationInfo::setStationCountry(const QString &newStationCountry)
{
    if (m_stationCountry == newStationCountry)
        return;
    m_stationCountry = newStationCountry;
    Q_EMIT stationCountryChanged();
}

QString StationInfo::stationTags() const
{
    return m_stationTags;
}

void StationInfo::setStationTags(const QString &newStationTags)
{
    if (m_stationTags == newStationTags)
        return;
    m_stationTags = newStationTags;
    Q_EMIT stationTagsChanged();
}

QString StationInfo::stationLanguage() const
{
    return m_stationLanguage;
}

void StationInfo::setStationLanguage(const QString &newStationLanguage)
{
    if (m_stationLanguage == newStationLanguage)
        return;
    m_stationLanguage = newStationLanguage;
    Q_EMIT stationLanguageChanged();
}

int StationInfo::stationVotes() const
{
    return m_stationVotes;
}

void StationInfo::setStationVotes(const int &newStationVotes)
{
    if (m_stationVotes == newStationVotes)
        return;
    m_stationVotes = newStationVotes;
    Q_EMIT stationVotesChanged();
}

QString StationInfo::stationState() const
{
    return m_stationState;
}

void StationInfo::setStationState(const QString &newStationState)
{
    if (m_stationState == newStationState)
        return;
    m_stationState = newStationState;
    Q_EMIT stationStateChanged();
}

int StationInfo::stationBitrate() const
{
    return m_stationBitrate;
}

void StationInfo::setStationBitrate(const int &newStationBitrate)
{
    if (m_stationBitrate == newStationBitrate)
        return;
    m_stationBitrate = newStationBitrate;
    Q_EMIT stationBitrateChanged();
}

QString StationInfo::stationCodec() const
{
    return m_stationCodec;
}

void StationInfo::setStationCodec(const QString &newStationCodec)
{
    if (m_stationCodec == newStationCodec)
        return;
    m_stationCodec = newStationCodec;
    Q_EMIT stationCodecChanged();
}

QString StationInfo::stationHomepage() const
{
    return m_stationHomepage;
}

void StationInfo::setStationHomepage(const QString &newStationHomepage)
{
    if (m_stationHomepage == newStationHomepage)
        return;
    m_stationHomepage = newStationHomepage;
    Q_EMIT stationHomepageChanged();
}

bool StationInfo::stationIsLocal() const
{
    return m_stationIsLocal;
}

void StationInfo::setStationIsLocal(const bool &newStationIsLocal)
{
    if (m_stationIsLocal == newStationIsLocal)
        return;
    m_stationIsLocal = newStationIsLocal;
    Q_EMIT stationIsLocalChanged();
}
