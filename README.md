# Sqlinq

Sqlinq provides a [LINQ](https://en.wikipedia.org/wiki/Language_Integrated_Query) like way of creating SQL Queries in Qt.

### Example

we are using the following table in the example:
```sql
CREATE TABLE `user`
(
	`id` int(11) NOT NULL,
	`name` varchar(100) NOT NULL
);
```

Now the Sqlinq example:
```c++
#include "sqlinq.h"
#include <QCoreApplication>
#include <QSqlDatabase>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // create database connection
    QSqlDatabase database = QSqlDatabase::addDatabase("QMYSQL3");
    database.setDatabaseName("test");
    database.setHostName("127.0.0.1");
    database.setPort(3306);
    database.setUserName("root");

    // create query using sqling
    Sqlinq query = Sqlinq().

                   // select single field (with optional alias):
                   // format: {field} or {field} as {alias} ...
                   // example: user.name as 'User'
                   Select("name", "User").

                   // auto generate Select expression for all available fields (with optional alias):
                   // note: if from table not allready set, this function will set it for you.
                   // format: {alias or tablename}.id ...
                   // example result: User
                   SelectTable("user", "User").

                   // extended auto generate Select expression for all available fields (with optional alias):
                   // note: if from table not allready set, this function will set it for you.
                   // format: {tablealias or tablename}.id as '{alias or tablealias or tablename}.id' ...
                   // example result: User.name as 'Usr.name'
                   SelectTableAs("user", "Usr", "User").

                   // select expression (with optional alias):
                   // note: if from table not allready set, this function will set it for you.
                   // format: {expression} or {expression} as '{alias}'
                   // example: CURDATE() as 'today'
                   SelectExpression("CURDATE()", "today").

                   // set from table (with optional alias):
                   // format: {table} or {field} as {alias} ...
                   // example: FROM user user
                   From("user", "User").

                   /// Where
                   Where("id IN(%1)", QList<int> {1, 2, 3});

    // print sql query
    qDebug() << query.toString();

    return 0;
}
```

which generates the following sql query:
```sql
SELECT
	name as 'User',
	User.id,
	User.name,
	User.id as 'Usr.id',
	User.name as 'Usr.name',
	CURDATE() as 'today'
FROM user User
WHERE id IN(1,2,3);
```

### Licence
The [Sqlinq licence](https://github.com/Spiek/sqlinq/blob/master/LICENCE) is a modified version of the [LGPL](http://www.gnu.org/licenses/lgpl.html) licence, with a static linking exception.
