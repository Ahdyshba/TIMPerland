#include "server.h"
#include <QCoreApplication>
#include <QString>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSslSocket>
#include <QByteArray>


Server::~Server()
{
    for (auto client : clients) {
        client->close();
        delete client;
    }
    tcpServer->close();
    db.close();
}

Server::Server(QObject *parent) : QObject(parent)
{
    tcpServer = new QTcpServer(this);

    connect(tcpServer, &QTcpServer::newConnection, this, &Server::acceptSocket);

    if (!tcpServer->listen(QHostAddress::Any, 55555)) {
        qDebug() << "Server is not started";
    } else {
        qDebug() << "Server is started";
    }

    setUpDb();
}

void Server::acceptSocket()
{
    QTcpSocket* clientSocket = tcpServer->nextPendingConnection();
    clients.append(clientSocket);

    connect(clientSocket, &QTcpSocket::readyRead, this, &Server::socketRead);
    connect(clientSocket, &QTcpSocket::disconnected, this, &Server::socketDisconnect);

    clientSocket->write("Connected to server\n");
}

void Server::socketRead()
{
    QTcpSocket* clientSocket = qobject_cast<QTcpSocket*>(sender());
    if (!clientSocket) return;

    QByteArray socketData = clientSocket->readAll();
    qDebug() << "Received data: " << socketData;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(socketData);
    if (!jsonDoc.isObject()) {
        qDebug() << "Read error: Invalid JSON received";
        return;
    }
    QJsonObject json = jsonDoc.object();

    QString action = json["action"].toString();
    QString username = json["username"].toString();
    QString password = json["password"].toString();
    QString email = json["email"].toString();

    QJsonObject responseJson;

    if (action == "sign_up") {
        if (signUpUser(username, password, email)) {
            responseJson["message"] = "Signed up successfully";
        } else {
            responseJson["message"] = "Signup error: Username or email already exists";
        }
    } else if (action == "log_in") {
        if (logInUser(username, password)) {
            responseJson["message"] = "Logged in successfuly";
        } else {
            responseJson["message"] = "Login error: Invalid username or password";
        }
    } else {
        responseJson["message"] = "Read error: Unknown action";
    }

    QJsonDocument responseDoc(responseJson);
    clientSocket->write(responseDoc.toJson());
    clientSocket->flush();
}

void Server::socketDisconnect()
{
    QTcpSocket* clientSocket = qobject_cast<QTcpSocket*>(sender());
    if (!clientSocket) return;

    clients.removeOne(clientSocket);
    clientSocket->close();
    clientSocket->deleteLater();
}

bool Server::signUpUser(const QString &username, const QString &password, const QString &email)
{
    if (!db.isOpen()) {
        qDebug() << "Signup error: Database is not open";
        return false;
    }

    QSqlQuery query(db);
    query.prepare("INSERT INTO users (username, password, email) VALUES (:username, :password, :email)");
    query.bindValue(":username", username);
    query.bindValue(":password", hash(password));
    query.bindValue(":email", email);

    qDebug() << "Executing query: " << query.lastQuery();
    qDebug() << "Username: " << username << ", Email: " << email;

    if (!query.exec()) {
        qDebug() << "Signup error: " << query.lastError().text();
        return false;
    }
    return true;
}

bool Server::logInUser(const QString &username, const QString &password)
{
    QSqlQuery query;
    query.prepare("SELECT password FROM users WHERE username = :username");
    query.bindValue(":username", username);

    if (!query.exec() || !query.next()) {
        qDebug() << "Login error: " << query.lastError().text();
        return false;
    }

    QString storedPassword = query.value(0).toString();
    return storedPassword == hash(password);
}

QString Server::hash(const QString &password)
{
    return QString(QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex());
}

void Server::setUpDb()
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("users.db");

    if (!db.open()) {
        qDebug() << "Database access error: " << db.lastError().text();
        return;
    }

    qDebug() << "Current working directory: " << QDir::currentPath();

    QSqlQuery query;
    QString CREATE_TABLE =
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "username TEXT UNIQUE, "
        "password TEXT, "
        "email TEXT UNIQUE"
        ")";
    if (!query.exec(CREATE_TABLE)) {
        qDebug() << "Table creation error: " << query.lastError().text();
    } else {
        qDebug() << "Table ready";
    }
}
