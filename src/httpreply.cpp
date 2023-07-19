#include "httpreply.h"

#include <QJsonDocument>
#include <QMimeType>
#include <QMimeData>
#include <QMimeDatabase>
#include <QRegularExpression>
#include <QTimerEvent>
#include <QLoggingCategory>

static Q_LOGGING_CATEGORY(logger, "refuel.http");

static const QRegularExpression JSON_APPLICATION =
        QRegularExpression(QStringLiteral("^application/json(\\s*;\\s*charset\\s*=\\s*utf-8)?$"));

HttpReply::HttpReply(QNetworkReply* reply)
    : m_reply(reply)
    , m_readyState(State::Opened)
{
    bool hasTimeout = false;
    int timeout = reply->attribute((QNetworkRequest::Attribute)(int)Attribute::Timeout).toInt(&hasTimeout);
    if (hasTimeout && timeout > 0) {
        m_timeout.start(timeout, this);
    }

    connect(reply, &QIODevice::readyRead, this, &HttpReply::onReadyRead);
    connect(reply, &QNetworkReply::finished, this, &HttpReply::onFinished);
}

QJsonDocument HttpReply::getJson() const {
    if (m_readyState != State::Done) {
        qCWarning(logger) << "Requested non-finished content";
        return {};
    }

    auto contentType = m_reply->header(QNetworkRequest::ContentTypeHeader);
    if (!JSON_APPLICATION.match(contentType.toString()).isValid()) {
        qCWarning(logger)
                << "Requested JSON content but got content type" << contentType;
        return {};
    }

    QJsonParseError error;
    auto result = QJsonDocument::fromJson(m_content, &error);
    if (error.error != QJsonParseError::NoError) {
        qCWarning(logger)
                << "Requested JSON content but got invalid JSON"
                << error.errorString();
    }
    return result;
}


void HttpReply::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_timeout.timerId()) {
        m_reply->abort();
    }
}

void HttpReply::onReadyRead()
{
    m_readyState = State::Loading;

    if (!m_reply->bytesAvailable()) {
        return;
    }

    char tmpBuffer[8 * 1024];
    auto bytes = m_reply->read(tmpBuffer, 8 * 1024);
    if (bytes <= 0) {
        return;
    }

    // TODO: only copy once
    m_content.append(tmpBuffer, bytes);
}

void HttpReply::onFinished()
{
    if (m_reply->bytesAvailable()) {
        onReadyRead();
    }

    m_readyState = State::Done;
    if (m_reply->error() == QNetworkReply::NoError) {
        auto sc = statusCode();
        if (sc >= 200 && sc < 300) {
            emit success();
        } else {
            emit errorOccured(QNetworkReply::UnknownServerError);
        }
    } else {
        emit errorOccured(m_reply->error());
    }

    emit finished();
}
