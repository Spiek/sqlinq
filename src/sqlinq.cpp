#include "sqlinq.h"

// static initial
QMap<QString, QMap<QString, QMap<QString, QSqlField>>> Sqlinq::schema;
QMutex Sqlinq::mutexSchema;

void Sqlinq::refreshSchema(QString connection)
{
    // refresh schema
    QSqlDatabase database = connection.isNull() ? QSqlDatabase::database() : QSqlDatabase::database(connection);
    QMap<QString, QMap<QString, QSqlField>> connectionFields;
    for(QString& table : database.tables()) {
        QSqlRecord record = database.record(table);
        QMap<QString, QSqlField>& fields = connectionFields.insert(table, QMap<QString, QSqlField>()).value();
        for(int i = 0; i < record.count(); i++) {
            QSqlField field = record.field(i);
            fields.insert(field.name(), field);
        }
    }

    // save schema (threadsave!)
    QMutexLocker locker(&Sqlinq::mutexSchema);
    Sqlinq::schema.insert(connection, connectionFields);
}

Sqlinq::Sqlinq(QString connection) : connection(connection)
{
    // generate schema if neccessary
    if(!this->schema.contains(connection)) {
        Sqlinq::refreshSchema(connection);
    }
}

Sqlinq& Sqlinq::Select(QString field, QString alias)
{
    return this->SelectExpression(Sqlinq::escape(field, false), alias);
}

Sqlinq& Sqlinq::SelectTable(QString table, QString tableAlias)
{
    // add all columns of table in format: {table}.{column}...
    QString queryTable = tableAlias.isEmpty() ? table : Sqlinq::escape(tableAlias, false);
    for(QString& tableField : this->schema.value(this->connection).value(table).keys()) {
        if(!this->select.isEmpty()) this->select += ", ";
        this->select += QString("%1.%2").arg(queryTable,
                                             tableField);
    }
    if(this->from.isEmpty()) this->from = queryTable;
    return *this;
}

Sqlinq& Sqlinq::SelectTableAs(QString table, QString alias, QString tableAlias)
{
    // add all columns of table in format: {table}.{column} as '{table}.{column}'...
    QString queryTable = tableAlias.isEmpty() ? table : Sqlinq::escape(tableAlias, false);
    QString queryTableAlias = alias.isEmpty() ? queryTable : Sqlinq::escape(alias, false);
    for(QString& tableField : this->schema.value(this->connection).value(table).keys()) {
        if(!this->select.isEmpty()) this->select += ", ";
        this->select += QString("%1.%2 as '%3.%2'").arg(queryTable,
                                                        tableField,
                                                        queryTableAlias);
    }
    if(this->from.isEmpty()) this->from = queryTable;
    return *this;
}

Sqlinq& Sqlinq::SelectExpression(QString expression, QString alias)
{
    // exit if no field is set
    if(expression.isEmpty()) return *this;

    // add expression
    // add base field
    if(!this->select.isEmpty()) this->select += ", ";
    if(!alias.isNull()) {
        this->select += QString("%1 as %2").arg(expression, Sqlinq::escape(alias));
    } else {
        this->select += expression;
    }
    return *this;
}


Sqlinq& Sqlinq::From(QString table, QString alias)
{
    this->from = table;
    this->fromAlias = alias;
    return *this;
}

Sqlinq& Sqlinq::GroupBy(QString field)
{
    if(field.isEmpty()) return *this;
    if(!this->group.isEmpty()) this->group += ", ";
    this->group += Sqlinq::escape(field, false);
    return *this;
}

Sqlinq& Sqlinq::OrderBy(QString sorting, SortOrder order)
{
    if(sorting.isEmpty()) return *this;
    if(!this->order.isEmpty()) this->order += ", ";
    this->order += QString("%1 %2").arg(Sqlinq::escape(sorting, false), order == SortOrder::ASC ? "ASC" : "DESC");
    return *this;
}

Sqlinq& Sqlinq::Offset(int offset)
{
    this->offset = offset;
    return *this;
}

QString Sqlinq::toString()
{
    // simplifier
    QString query;

    // Select
    if(!this->select.isEmpty()) {
        query += "SELECT " + this->select;

        // if we have no From table, inform user
        if(this->from.isEmpty()) {
            qWarning("Cannot detect From table, query will fail!");
        }
    }

    // From
    if(!this->from.isEmpty()) {
        query.append(" FROM ").append(this->from);
        if(!this->fromAlias.isEmpty()) {
            query.append(" ").append(this->fromAlias);
        }
    }

    // Joins
    if(!this->join.isEmpty()) {
        query.append(" ").append(this->join);
    }

    // Where
    if(!this->where.isEmpty()) {
        query.append(" WHERE ").append(this->where);
    }

    // Group by
    if(!this->group.isEmpty()) {
        query.append(" GROUP BY ").append(this->group);
    }

    // Having
    if(!this->having.isEmpty()) {
        query.append(" HAVING ").append(this->having);
    }

    // Order
    if(!this->order.isEmpty()) {
        query.append(" ORDER BY ").append(this->order);
    }

    // Limit
    if(this->limit > 0) {
        query.append(QString(" LIMIT %1").arg(this->limit));
    }

    // Offset
    if(this->offset > 0) {
        query.append(QString(" OFFSET %1").arg(this->offset));
    }

    // query generation finished!
    return query.append(";");
}

QString Sqlinq::constructCondition(QString condition, QVariantList args)
{
    if(condition.isEmpty()) condition = args.takeFirst().value<QString>();
    for(QVariant& variant : args) {
        condition = condition.arg(variant.isValid() ? Sqlinq::constructConditionField(variant) : "");
    }
    return condition;
}

QString Sqlinq::constructConditionField(const QVariant& field)
{
    QString value;

    // QStringList IN()-field
    if(field.type() == QVariant::StringList) {
        for(QString& subString : field.value<QStringList>()){
            if(!value.isEmpty()) value.append(",");
            value += Sqlinq::escape(subString);
        }
    }

    // handle date and time types
    else if(field.type() == QVariant::Date) value = field.value<QDate>().toString("''yyyy-MM-dd''");
    else if(field.type() == QVariant::Time) value = field.value<QTime>().toString("''hh:mm:ss''");
    else if(field.type() == QVariant::DateTime) value = field.value<QDateTime>().toString("''yyyy-MM-dd hh:mm:ss''");

    // QVariantlist (recursive!)
    else if(field.canConvert<QVariantList>()) {
        for (const QVariant &subVariant : field.value<QSequentialIterable>()) {
            if(!value.isEmpty()) value.append(",");
            value += Sqlinq::constructConditionField(subVariant);
        }
    }
    // normal value
    else {
        value = Sqlinq::escape(field);
    }

    return value;
}



