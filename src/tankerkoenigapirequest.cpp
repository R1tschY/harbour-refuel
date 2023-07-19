#include "tankerkoenigapirequest.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLoggingCategory>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QStringBuilder>
#include <QUrl>
#include <QUrlQuery>

static Q_LOGGING_CATEGORY(logger, "refuel.tankerkoenig");

static const QUrl LIST_URL = QUrl(QStringLiteral("https://creativecommons.tankerkoenig.de/json/list.php"));

TankerKoenigApiRequest::TankerKoenigApiRequest(QObject* parent)
    : Request(parent)
    , m_network()
    , m_apiKey(QStringLiteral("00000000-0000-0000-0000-000000000002"))
    , m_reply(nullptr)
{
}

void TankerKoenigApiRequest::list(
    double latitude, double longitude, double radius, Fuel spirit,
    Sorting sorting)
{
    QUrlQuery query {};
    query.addQueryItem(QStringLiteral("apikey"), m_apiKey);
    query.addQueryItem(QStringLiteral("lat"), QString::number(latitude));
    query.addQueryItem(QStringLiteral("lng"), QString::number(longitude));
    query.addQueryItem(QStringLiteral("rad"), QString::number(radius));

    QString spiritStr;
    switch (spirit) {
    case Request::Fuel::SuperE5:
        spiritStr = QStringLiteral("e5");
        break;
    case Request::Fuel::SuperE10:
        spiritStr = QStringLiteral("e10");
        break;
    case Request::Fuel::Diesel:
        spiritStr = QStringLiteral("diesel");
        break;
    case Request::Fuel::All:
        spiritStr = QStringLiteral("all");
        break;
    }
    query.addQueryItem(QStringLiteral("type"), spiritStr);

    QString sortingStr;
    switch (sorting) {
    case Request::Sorting::Price:
        sortingStr = QStringLiteral("price");
        break;
    case Request::Sorting::Distance:
        sortingStr = QStringLiteral("dist");
        break;
    }
    query.addQueryItem(QStringLiteral("sort"), sortingStr);

    QUrl url = LIST_URL;
    url.setQuery(query);

    QNetworkRequest request { url };
    request.setHeader(
        QNetworkRequest::KnownHeaders::UserAgentHeader,
        QStringLiteral("Refuel Sailfish OS/0.1.0"));
    request.setRawHeader("Accept", "application/json;charset=utf-8");

    qCInfo(logger) << "Start list request";
    qCDebug(logger) << "Request URL" << url;
    m_reply.reset(new HttpReply(m_network.get(request)));
    connect(m_reply.data(), &HttpReply::success, this, [this, spirit]() {
        HttpReply* reply = qobject_cast<HttpReply*>(sender());

        qCDebug(logger).noquote() << "Got content" << reply->getContent();

        auto jsonDoc = reply->getJson();
        if (jsonDoc.isEmpty()) {
            emit errorOccured(QStringLiteral("Server error: invalid JSON"));
            return;
        }

        auto root = jsonDoc.object();
        bool ok = root.value(QStringLiteral("ok")).toBool(false);
        if (!ok) {
            auto message = root.value("message").toString("<missing>");
            emit errorOccured(QStringLiteral("Server error: ") % message);
            return;
        }

        QString status = root.value(QStringLiteral("status")).toString();
        if (status != QStringLiteral("ok")) {
            emit errorOccured(QStringLiteral("Status error: ") % status);
            return;
        }

        const auto stations = root.value(QStringLiteral("stations")).toArray();
        qCInfo(logger) << "Got list result with" << stations.size() << "items";

        QVector<Station> result;
        for (auto station : stations) {
            auto stationObject = station.toObject();
            auto id = stationObject.value(QStringLiteral("id")).toString();
            auto name = stationObject.value(QStringLiteral("name")).toString();
            auto brand = stationObject.value(QStringLiteral("brand")).toString();
            auto street = stationObject.value(QStringLiteral("street")).toString();
            auto houseNumber = stationObject.value(QStringLiteral("houseNumber")).toString();
            auto postCode = stationObject.value(QStringLiteral("postCode")).toInt();
            auto place = stationObject.value(QStringLiteral("place")).toString();
            float lat = stationObject.value(QStringLiteral("lat")).toDouble();
            float lng = stationObject.value(QStringLiteral("lng")).toDouble();
            float dist = stationObject.value(QStringLiteral("dist")).toDouble();
            auto isOpen = stationObject.value(QStringLiteral("isOpen")).toBool();

            //            QString postCodeStr;
            //            QTextStream postCodeStream(&postCodeStr, QIODevice::ReadOnly);
            //            postCodeStream.setFieldWidth(5);
            //            postCodeStream.setFieldAlignment(QTextStream::AlignRight);
            //            postCodeStream.setPadChar('0');
            // QStringLiteral("%1").arg(postCode, 5, '0')

            QString address = street % QChar(' ') % houseNumber % QStringLiteral(", ")
                % QString::number(postCode).rightJustified(5, '0')
                % QChar(' ') % place;

            float price;
            if (spirit == Request::Fuel::All) {
                price = stationObject.value(QStringLiteral("e10")).toDouble();
            } else {
                price = stationObject.value(QStringLiteral("price")).toDouble();
            }

            result.append(Station {
                .id = id,
                .name = name,
                .brand = brand,
                .address = address,
                .latitude = lat,
                .longitude = lng,
                .distance = dist,
                .isOpen = isOpen,
                .price = price });
        }

        emit listReceived(result);
    });
}
