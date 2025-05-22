#ifndef SERVER_H
#define SERVER_H


#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QtNetwork>
#include <QByteArray>
#include <QDebug>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QCryptographicHash>


class Server : public QObject
{
    Q_OBJECT

public:
    ~Server();
    explicit Server(QObject *parent = nullptr);

private slots:
    void acceptSocket();
    void socketRead();
    void socketDisconnect();

private:
    QTcpServer * tcpServer;
    QList<QTcpSocket*> clients;
    QSqlDatabase db;

    bool signUpUser(const QString &username, const QString &password, const QString &email);
    bool logInUser(const QString &username, const QString &password);
    bool updateUserPassword(const QString &username, const QString &newPassword);
    QString hash(const QString &password);

    void setUpDb();
};


#endif // SERVER_H
