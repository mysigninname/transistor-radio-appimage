#ifndef STATIONSEARCHMODEL_H
#define STATIONSEARCHMODEL_H

#include "StationAbstractModel.h"
#include <QAbstractListModel>
#include <QDnsLookup>
#include <QNetworkAccessManager>
#include <QTimer>

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

Q_SIGNALS:
    void isSearchingChanged();

private:
    QNetworkAccessManager m_networkManager;
    QNetworkReply *m_reply = nullptr;
    QNetworkReply *meta_reply = nullptr;
    bool m_isSearching = false;
    QDnsLookup *m_dns;
    
};

#endif // STATIONSEARCHMODEL_H
