#include "StationSearchModel.h"
#include "StationInfo.h"
#include <QCoreApplication>
#include <QDateTime>
#include <QDnsLookup>
#include <QEventLoop>
#include <QHostAddress>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocale>
#include <QNetworkReply>
#include <QRegularExpression>
#include <QString>
#include <QTimer>
#include <QUrlQuery>
#include <qdebug.h>
#include <qlogging.h>
#include <qobject.h>
#include <qvariant.h>

QString StationSearchModel::lookupServers()
{
    QString main_url;
    QEventLoop loop;
    QDnsLookup dns;
    dns.setType(QDnsLookup::SRV);
    dns.setName(QStringLiteral("_api._tcp.radio-browser.info"));

    connect(&dns, &QDnsLookup::finished, [&]() {
        if (dns.error() != QDnsLookup::NoError) {
            qWarning("DNS lookup failed");
            loop.quit();
            return;
        }

        const auto records = dns.serviceRecords();
        if (records.isEmpty()) {
            qWarning() << QStringLiteral("EMPTY");
            loop.quit();
            return;
        }

        QDnsServiceRecord record = records[rand() % records.size()];
        main_url = record.target();
        loop.quit();
    });

    dns.lookup();
    loop.exec();

    return main_url;
}

StationSearchModel::StationSearchModel(QObject *parent)
    : StationAbstractModel(parent)

{
}

bool StationSearchModel::isListOfUUIDs(const QString &input)
{
    QStringList uuidList = input.split(QStringLiteral("'"), Qt::SkipEmptyParts);

    for (const QString &uuidStr : uuidList) {
        QString trimmedUUID = uuidStr.trimmed();
        QUuid uuid(trimmedUUID);

        if (uuid.isNull()) {
            return false;
        }
    }

    return true;
}

void StationSearchModel::getData(const QString &text, const int &limit, const int &offset, const bool &reset, const bool &search)
{
    if (reset) {
        clearAll();
    }
    if(isSearching()) {
        return;
    }
    if (!text.trimmed().isEmpty()) {
        if (m_reply) {
            m_reply->abort();
            m_reply = nullptr;
        }
        setIsSearching(true);
        QString safeText(text);
        safeText.replace(QLatin1Char(' '), QLatin1Char('+'));
        QString source = QStringLiteral("https://") + lookupServers() + QStringLiteral("/json/stations");
        if (search) {
            source = source + QStringLiteral("/search");
        }

        QUrlQuery query;
        if (search) {
            query.addQueryItem(QStringLiteral("name"), text);
        } else if (isListOfUUIDs(text)) {
            source = source + QStringLiteral("/byuuid");
            query.addQueryItem(QStringLiteral("uuids"), text);
        } else {
            query.addQueryItem(QStringLiteral("order"), text);
        }
        if (text == QStringLiteral("votes")) {
            query.addQueryItem(QStringLiteral("reverse"), QStringLiteral("true"));
        }
        query.addQueryItem(QStringLiteral("limit"), QString::number(limit));
        query.addQueryItem(QStringLiteral("offset"), QString::number(offset));
        query.addQueryItem(QStringLiteral("hidebroken"), QStringLiteral("true"));
        QUrl url(source);
        url.setQuery(query);
        m_reply = m_networkManager.get(QNetworkRequest(QUrl(url)));
        connect(m_reply, &QNetworkReply::finished, this, [this]() {
            if (m_reply->error() == QNetworkReply::NoError) {
                QByteArray data = m_reply->readAll();
                QJsonDocument jsonDocument = QJsonDocument::fromJson(data);
                QJsonArray results = jsonDocument.array();
                const auto rows = rowCount();
                beginInsertRows({}, rows, rows + results.size() - 1);
                for (const auto &result : results) {
                    QJsonObject entry = result.toObject();
                    StationInfo *stationInfo = new StationInfo(this);
                    stationInfo->setStationName(entry[QStringLiteral("name")].toString());
                    stationInfo->setStationUuid(entry[QStringLiteral("stationuuid")].toString());
                    stationInfo->setStationImageSource(QUrl(entry[QStringLiteral("favicon")].toString()));
                    if (entry[QStringLiteral("url_resolved")].toString().isEmpty()) {
                        stationInfo->setStationSource(QUrl(entry[QStringLiteral("url")].toString()));    
                    } else {
                    stationInfo->setStationSource(QUrl(entry[QStringLiteral("url_resolved")].toString()));
                    }
                    if (QLocale::territoryToString(QLocale::codeToTerritory(entry[QStringLiteral("countrycode")].toString())) == QStringLiteral("Default")) {
                        stationInfo->setStationCountry(QStringLiteral());
                    } else {
                    stationInfo->setStationCountry(QLocale::territoryToString(QLocale::codeToTerritory(entry[QStringLiteral("countrycode")].toString())));
                    }
                    stationInfo->setStationTags(entry[QStringLiteral("tags")].toString());
                    stationInfo->setStationLanguage(entry[QStringLiteral("language")].toString());
                    stationInfo->setStationVotes(entry[QStringLiteral("votes")].toInt());
                    stationInfo->setStationState(entry[QStringLiteral("state")].toString());
                    stationInfo->setStationBitrate(entry[QStringLiteral("bitrate")].toInt());
                    stationInfo->setStationCodec(entry[QStringLiteral("codec")].toString());
                    stationInfo->setStationHomepage(entry[QStringLiteral("homepage")].toString());
                    stationInfo->setStationIsLocal(false);
                    m_stationList << stationInfo;
                }
                endInsertRows();

            } else if (m_reply->error() != QNetworkReply::OperationCanceledError) {
                qCritical() << "Reply failed, eror:" << m_reply->errorString();
            }

            m_reply->deleteLater();
            m_reply = nullptr;
            setIsSearching(false);
        });
    }
}

bool StationSearchModel::isSearching() const
{
    return m_isSearching;
}

void StationSearchModel::setIsSearching(bool newIsSearching)
{
    if (m_isSearching == newIsSearching)
        return;
    m_isSearching = newIsSearching;
    Q_EMIT isSearchingChanged();
}
