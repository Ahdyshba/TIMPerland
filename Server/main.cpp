#include "server.h"
#include <QCoreApplication>
#include <QtSql/QSqlDatabase>
#include <QDebug>


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    Server server1;
    return a.exec();
}
