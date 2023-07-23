#include "sqlquerymodel.h"

#include <QSqlRecord>
#include <QSqlError>
#include <QCryptographicHash>
#include <QLoggingCategory>
#include <QSqlDriver>

static Q_LOGGING_CATEGORY(logger, "qommons.models");

static QString localStorageConnectionName(const QString& databaseName) {
    QCryptographicHash md5(QCryptographicHash::Md5);
    md5.addData(databaseName.toUtf8());
    return md5.result().toHex();
}

SqlQueryModel::SqlQueryModel(QObject *parent)
    : QSqlQueryModel(parent)
{ }

QVariant SqlQueryModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || role <= Qt::UserRole) {
        return { };
    }

    int columnIdx = role - Qt::UserRole - 1;
    QModelIndex modelIndex = this->index(index.row(), columnIdx);
    return QSqlQueryModel::data(modelIndex, Qt::DisplayRole);
}

QHash<int, QByteArray> SqlQueryModel::roleNames() const
{
    auto rec = record();

    QHash<int, QByteArray> roleNames;
    roleNames.reserve(rec.count());
    for (int i = 0; i < rec.count(); i++) {
        roleNames.insert(Qt::UserRole + i + 1, rec.fieldName(i).toUtf8());
    }
    return roleNames;
}

QString SqlQueryModel::errorString() const
{
    return lastError().text();
}

void SqlQueryModel::setConnectionName(const QString &databaseId)
{
    if (m_connectionName != databaseId) {
        m_connectionName = databaseId;
        emit connectionNameChanged();
    }
}

void SqlQueryModel::setLocalStorageName(const QString &databaseName)
{
    setConnectionName(localStorageConnectionName(databaseName));
}

void SqlQueryModel::exec(const QString &queryString, const QVariant &parameters)
{
    QSqlQuery query { QSqlDatabase::database(m_connectionName) };
    if (parameters.type() == QVariant::List) {
        const QVariantList parameterList = parameters.toList();
        for (auto parameter : parameterList) {
            query.addBindValue(parameter);
        }
    } else if (parameters.type() == QVariant::Map) {
        const QVariantMap parameterMap = parameters.toMap();
        for (auto i = parameterMap.cbegin(); i != parameterMap.cend(); ++i) {
            query.bindValue(i.key(), i.value());
        }
    }
    query.exec(queryString);

    qCDebug(logger) << query.lastQuery() << query.lastError().text() << query.driver()->isOpen();

    setNewQuery(query);
}

void SqlQueryModel::reload()
{
    auto oldQuery = query();

    QSqlQuery query { QSqlDatabase::database(m_connectionName) };
    const QVariantMap parameterMap = oldQuery.boundValues();
    for (auto i = parameterMap.cbegin(); i != parameterMap.cend(); ++i) {
        query.bindValue(i.key(), i.value());
    }
    query.exec(oldQuery.lastQuery());

    setNewQuery(query);
}

void SqlQueryModel::setNewQuery(const QSqlQuery &query)
{
    setQuery(query);

    QSqlError error = lastError();
    if (error.type() != QSqlError::NoError) {
        qCWarning(logger) << "SQL error:" << error.text();
        emit errorStringChanged();
    } else {
        qCDebug(logger) << "Selected" << rowCount() << "rows";
    }
}


void SqlQueryModel::queryChange()
{
    emit queryChanged();
}
