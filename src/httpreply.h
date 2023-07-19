#ifndef HTTPREPLY_H
#define HTTPREPLY_H

#include <QObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QBasicTimer>
#include <QScopedPointer>

class QJsonDocument;


class HttpReply : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("RegisterEnumClassesUnscoped", "false")
    Q_PROPERTY(State readyState READ readyState NOTIFY readyStateChanged)

public:
    enum class State {
        Unsent, Opened, HeadersReceived, Loading, Done
    };
    Q_ENUM(State)

    enum Attribute {
        Timeout
    };
    Q_ENUM(Attribute)

    explicit HttpReply(QNetworkReply* reply);

    QByteArray getContent() const {
        return m_content;
    }

    QJsonDocument getJson() const;

    int statusCode() const {
        bool ok = false;
        int result = m_reply->attribute(
                    QNetworkRequest::Attribute::HttpStatusCodeAttribute)
                .toInt(&ok);
        if (ok) {
            return result;
        } else {
            return -1;
        }
    }

    State readyState() const {
        return m_readyState;
    }

signals:
    void readyStateChanged(State readyState);

    void finished();

    void success();
    void errorOccured(QNetworkReply::NetworkError code);

private:
    QScopedPointer<QNetworkReply> m_reply;
    State m_readyState;
    QByteArray m_content;
    QBasicTimer m_timeout;

    void timerEvent(QTimerEvent *event) override;

    void onReadyRead();
    void onFinished();
};

#endif // HTTPREPLY_H
