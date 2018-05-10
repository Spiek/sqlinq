#ifndef QSQLLINQ_H
#define QSQLLINQ_H

#include <QDebug>
#include <QList>
#include <QVariant>
#include <QMutex>

#include <QDateTime>

#include <QtSql/QSqlQuery>
#include <QtSql/QSqlRecord>
#include <QtSql/QSqlField>

// Argument helpers
#define VAR(v,number) v##number
#define DynamicArgumentsDefault(tpattern, pattern) VAR(tpattern, 1) VAR(pattern, 1) = VAR(tpattern, 1)(), VAR(tpattern, 2) VAR(pattern, 2) = VAR(tpattern, 2)(), VAR(tpattern, 3) VAR(pattern, 3) = VAR(tpattern, 3)(), VAR(tpattern, 4) VAR(pattern, 4) = VAR(tpattern, 4)(), VAR(tpattern, 5) VAR(pattern, 5) = VAR(tpattern, 5)(), VAR(tpattern, 6) VAR(pattern, 6) = VAR(tpattern, 6)(), VAR(tpattern, 7) VAR(pattern, 7) = VAR(tpattern, 7)(), VAR(tpattern, 8) VAR(pattern, 8) = VAR(tpattern, 8)(), VAR(tpattern, 9) VAR(pattern, 9) = VAR(tpattern, 9)(), VAR(tpattern, 10) VAR(pattern, 10) = VAR(tpattern, 10)()
#define DynamicArgumentsDefaultTemplate(tpattern) template<typename VAR(tpattern,1) = QVariant, typename VAR(tpattern,2) = QVariant, typename VAR(tpattern,3) = QVariant, typename VAR(tpattern,4) = QVariant, typename VAR(tpattern,5) = QVariant, typename VAR(tpattern,6) = QVariant, typename VAR(tpattern,7) = QVariant, typename VAR(tpattern,8) = QVariant, typename VAR(tpattern,9) = QVariant, typename VAR(tpattern,10) = QVariant>
#define DynamicArgumentsFwd(pattern) VAR(pattern,1), VAR(pattern,2), VAR(pattern,3), VAR(pattern,4), VAR(pattern,5), VAR(pattern,6), VAR(pattern,7), VAR(pattern,8), VAR(pattern,9), VAR(pattern,10)
#define DynamicArgumentsToVList(tpattern, pattern, list) \
        QVariant variant; \
        variant = QVariant::fromValue<VAR(tpattern,1)>(VAR(pattern,1)); if(variant.isValid()) list.append(variant); \
        variant = QVariant::fromValue<VAR(tpattern,2)>(VAR(pattern,2)); if(variant.isValid()) list.append(variant); \
        variant = QVariant::fromValue<VAR(tpattern,3)>(VAR(pattern,3)); if(variant.isValid()) list.append(variant); \
        variant = QVariant::fromValue<VAR(tpattern,4)>(VAR(pattern,4)); if(variant.isValid()) list.append(variant); \
        variant = QVariant::fromValue<VAR(tpattern,5)>(VAR(pattern,5)); if(variant.isValid()) list.append(variant); \
        variant = QVariant::fromValue<VAR(tpattern,6)>(VAR(pattern,6)); if(variant.isValid()) list.append(variant); \
        variant = QVariant::fromValue<VAR(tpattern,7)>(VAR(pattern,7)); if(variant.isValid()) list.append(variant); \
        variant = QVariant::fromValue<VAR(tpattern,8)>(VAR(pattern,8)); if(variant.isValid()) list.append(variant); \
        variant = QVariant::fromValue<VAR(tpattern,9)>(VAR(pattern,9)); if(variant.isValid()) list.append(variant); \
        variant = QVariant::fromValue<VAR(tpattern,10)>(VAR(pattern,10)); if(variant.isValid()) list.append(variant);

class Sqlinq
{
    public:
        // Types
        enum SortOrder { ASC, DESC };

        // Static implementations
        static void refreshSchema(QString connection);

        // Con and Decon
        Sqlinq(QString connection = QString());

        /// Query Builder Function

        // Select
        Sqlinq& Select(QString field, QString alias = QString());
        Sqlinq& SelectTable(QString table, QString tableAlias = QString());
        Sqlinq& SelectTableAs(QString table, QString alias = QString(), QString tableAlias = QString());
        Sqlinq& SelectExpression(QString expression, QString alias = QString());

        // From
        Sqlinq& From(QString table, QString alias = QString());

        // Where
        DynamicArgumentsDefaultTemplate(Arg)
        Sqlinq& Where(QString condition, DynamicArgumentsDefault(Arg, arg))
        {
            QVariantList variables;
            DynamicArgumentsToVList(Arg, arg, variables);
            if(!this->where.isEmpty()) this->where += " AND ";
            this->where += Sqlinq::constructCondition(condition, variables);
            return *this;
        }

        // Join
        DynamicArgumentsDefaultTemplate(Arg)
        inline Sqlinq& LeftJoin(QString table, QString onCondition, DynamicArgumentsDefault(Arg, arg)) {
            return this->Join("LEFT", table, onCondition, DynamicArgumentsFwd(arg));
        }
        DynamicArgumentsDefaultTemplate(Arg)
        inline Sqlinq& RightJoin(QString table, QString onCondition, DynamicArgumentsDefault(Arg, arg)) {
            return this->Join("RIGHT", table, onCondition, DynamicArgumentsFwd(arg));
        }
        DynamicArgumentsDefaultTemplate(Arg)
        inline Sqlinq& InnerJoin(QString table, QString onCondition, DynamicArgumentsDefault(Arg, arg)) {
            return this->Join("INNER", table, onCondition, DynamicArgumentsFwd(arg));
        }
        DynamicArgumentsDefaultTemplate(Arg)
        inline Sqlinq& OuterJoin(QString table, QString onCondition, DynamicArgumentsDefault(Arg, arg)) {
            return this->Join("OUTER", table, onCondition, DynamicArgumentsFwd(arg));
        }
        DynamicArgumentsDefaultTemplate(Arg)
        Sqlinq& Join(QString type, QString table, QString onCondition, DynamicArgumentsDefault(Arg, arg))
        {
            QVariantList join;
            DynamicArgumentsToVList(Arg, arg, join);
            if(!this->join.isEmpty()) this->join += " ";
            this->join += QString("%1 JOIN %2 ON %3")
                     .arg(type)
                     .arg(table)
                     .arg(Sqlinq::constructCondition(onCondition, join));
            return *this;
        }

        // Group by
        Sqlinq& GroupBy(QString fieldname);

        // Having
        DynamicArgumentsDefaultTemplate(Arg)
        Sqlinq& Having(QString condition, DynamicArgumentsDefault(Arg, arg))
        {
            QVariantList variables;
            DynamicArgumentsToVList(Arg, arg, variables);
            if(!this->having.isEmpty()) this->having += " AND ";
            this->having += Sqlinq::constructCondition(condition, variables);
            return *this;
        }

        // Order by
        Sqlinq& OrderBy(QString sorting, SortOrder order = SortOrder::ASC);

        // Limit and offset
        Sqlinq& Limit(int limit, int offset = 0);
        Sqlinq& Offset(int offset);

        /// Query Execution Function

        QString toString();

    private:
        // helpers
        static QString escape(QVariant value) {
            if(value.type() == QVariant::Char || value.type() == QVariant::String || value.type() == QVariant::ByteArray) {
                return Sqlinq::escape(value.toString());
            }
            return value.toString();
        }
        static QString escape(QString value, bool inQuotes = true) {
            value = value.replace("'", "\\'").replace("\"", "\\\"");
            if(inQuotes) value.prepend("'").append("'");
            return value;
        }
        static QString constructCondition(QString condition, QVariantList args);
        static QString constructConditionField(const QVariant &field);

        // Query
        QString connection;

        // select data
        QString select;

        // from data
        QString from;
        QString fromAlias;

        // conditions data: join, where and having
        QString join;
        QString where;
        QString having;

        // groups data
        QString group;

        // order data
        QString order;

        // limit data
        int limit = 0;
        int offset = 0;

        // Schema:
        // Connection
        // - Table
        // -- Column -> Datatype
        static QMap<QString, QMap<QString, QMap<QString, QSqlField>>> schema;
        static QMutex mutexSchema;
};

#endif // QSQLLINQ_H
