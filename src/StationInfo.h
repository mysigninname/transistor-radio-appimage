#ifndef StationInfo_H
#define StationInfo_H

#include <QObject>
#include <QUrl>
#include <qqml.h>

class StationInfo : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int songIndex READ songIndex WRITE setSongIndex NOTIFY songIndexChanged REQUIRED)
    Q_PROPERTY(QString stationName READ stationName WRITE setStationName NOTIFY stationNameChanged)
    Q_PROPERTY(QString stationUuid READ stationUuid WRITE setStationUuid NOTIFY stationUuidChanged)
    Q_PROPERTY(QUrl stationImageSource READ stationImageSource WRITE setStationImageSource NOTIFY stationImageSourceChanged)
    Q_PROPERTY(QUrl stationSource READ stationSource WRITE setStationSource NOTIFY stationSourceChanged REQUIRED)
    Q_PROPERTY(QString stationCountry READ stationCountry WRITE setStationCountry NOTIFY stationCountryChanged)
    Q_PROPERTY(QString stationTags READ stationTags WRITE setStationTags NOTIFY stationTagsChanged)
    Q_PROPERTY(QString stationLanguage READ stationLanguage WRITE setStationLanguage NOTIFY stationLanguageChanged)
    Q_PROPERTY(int stationVotes READ stationVotes WRITE setStationVotes NOTIFY stationVotesChanged)
    Q_PROPERTY(QString stationState READ stationState WRITE setStationState NOTIFY stationStateChanged)
    Q_PROPERTY(int stationBitrate READ stationBitrate WRITE setStationBitrate NOTIFY stationBitrateChanged)
    Q_PROPERTY(QString stationCodec READ stationCodec WRITE setStationCodec NOTIFY stationCodecChanged)
    Q_PROPERTY(QString stationHomepage READ stationHomepage WRITE setStationHomepage NOTIFY stationHomepageChanged)
    Q_PROPERTY(bool stationIsLocal READ stationIsLocal WRITE setStationIsLocal NOTIFY stationIsLocalChanged)

public:
    explicit StationInfo(QObject *parent = nullptr);

    int songIndex() const;
    void setSongIndex(int newSongIndex);

    QString stationName() const;
    void setStationName(const QString &newTitle);

    QString stationUuid() const;
    void setStationUuid(const QString &uuid);

    QUrl stationImageSource() const;
    void setStationImageSource(const QUrl &newImageSource);

    QUrl stationSource() const;
    void setStationSource(const QUrl &newStationSource);

    QString stationCountry() const;
    void setStationCountry(const QString &newStationCountry);

    QString stationTags() const;
    void setStationTags(const QString &newStationTags);

    QString stationLanguage() const;
    void setStationLanguage(const QString &newStationLanguage);

    int stationVotes() const;
    void setStationVotes(const int &newStationVotes);

    QString stationState() const;
    void setStationState(const QString &newStationState);

    int stationBitrate() const;
    void setStationBitrate(const int &newStationBitrate);

    QString stationCodec() const;
    void setStationCodec(const QString &newStationCodec);

    QString stationHomepage() const;
    void setStationHomepage(const QString &newStationHomepage);

    bool stationIsLocal() const;
    void setStationIsLocal(const bool &newStationIsLocal);

Q_SIGNALS:
    void songIndexChanged();
    void stationNameChanged();
    void stationUuidChanged();
    void stationImageSourceChanged();
    void stationSourceChanged();
    void stationCountryChanged();
    void stationTagsChanged();
    void stationLanguageChanged();
    void stationVotesChanged();
    void stationStateChanged();
    void stationBitrateChanged();
    void stationCodecChanged();
    void stationHomepageChanged();
    void stationIsLocalChanged();

private:
    int m_songIndex;
    QString m_stationName;
    QString m_stationUuid;
    QUrl m_stationImageSource;
    QUrl m_stationSource;
    QString m_stationCountry;
    QString m_stationTags;
    QString m_stationLanguage;
    int m_stationVotes;
    QString m_stationState;
    int m_stationBitrate;
    QString m_stationCodec;
    QString m_stationHomepage;
    bool m_stationIsLocal;
};

#endif // StationInfo_H
