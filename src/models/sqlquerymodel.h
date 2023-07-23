#pragma once

#include <QSqlQueryModel>
#include <QSqlQuery>

class SqlQueryModel : public QSqlQueryModel
{
    Q_OBJECT
    Q_PROPERTY(QString query READ queryString NOTIFY queryChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorStringChanged)
    Q_PROPERTY(QVariantMap boundValues READ boundValues NOTIFY queryChanged)
    Q_PROPERTY(QString localStorageName WRITE setLocalStorageName)

public:
    explicit SqlQueryModel(QObject *parent = nullptr);

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    QString errorString() const;

    QString queryString() const { return query().lastQuery(); }
    QVariantMap boundValues() const { return query().boundValues(); }

    QString connectionName() const { return m_connectionName; }
    void setConnectionName(const QString& connectionName);
    void setLocalStorageName(const QString& databaseName);

    Q_INVOKABLE void exec(const QString& query, const QVariant& parameters = QVariant());
    Q_INVOKABLE void reload();

signals:
    void connectionNameChanged();
    void errorStringChanged();
    void queryChanged();

private:
    QString m_connectionName;
    QSqlQuery m_query;

    void setNewQuery(const QSqlQuery& query);
    void queryChange() override;
};
