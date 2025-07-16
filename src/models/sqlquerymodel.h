/*
 * Copyright 2025 Richard Liebscher <r1tschy@posteo.de>.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
 
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
