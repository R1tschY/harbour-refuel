#ifndef TANKERKOENIGAPIREQUEST_H
#define TANKERKOENIGAPIREQUEST_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QScopedPointer>

#include "request.h"
#include "httpreply.h"

class TankerKoenigApiRequest : public Request
{
    Q_OBJECT
public:
    explicit TankerKoenigApiRequest(QObject *parent = nullptr);

    void list(
            double latitude, double longitude, double radius,
            Request::Fuel spirit, Request::Sorting sorting);

signals:
    void listReceived(const QVector<Station>& stations);
    void errorOccured(const QString& error);

private:
    QNetworkAccessManager m_network;
    QString m_apiKey;
    QScopedPointer<HttpReply> m_reply;
};

#endif // TANKERKOENIGAPIREQUEST_H
