#include "clientfuncs.h"
#include "client.h"
#include <QFile>
#include <QString>
#include <QDebug>
#include <QDir>


bool logIn(QString login, QString password)
{
    if (!ensureServerConnection()) {
        qDebug() << "Login error: failed to connect to server";
        return false;
    }

    Client* client = Client::getInstance();
    bool result = client->logInUser(login, password);

    QObject::connect(client, &Client::login_result, [&result](bool success, const QString& message) {
        result = success;
        qDebug() << "Login result: " << message;
    });

    return result;
}

bool signUp(QString login, QString password, QString email)
{
    if (!ensureServerConnection()) {
        qDebug() << "Signup error: failed to connect to server";
        return false;
    }

    Client* client = Client::getInstance();
    bool result = client->signUpUser(login, password, email);

    QObject::connect(client, &Client::signup_result, [&result](bool success, const QString& message) {
        result = success;
        qDebug() << "Signup result: " << message;
    });

    return result;
}

bool ensureServerConnection() {
    Client* client = Client::getInstance();

    if (client->connected())
        return true;

    QFile configFile(QDir::currentPath() + "/config.json");
    if (!configFile.open(QIODevice::ReadOnly)) {
        qDebug() << "Connection error: failed to open config.json";
        qDebug() << "Current directory: " << QDir::currentPath();
        return false;
    }
    QJsonObject config = QJsonDocument::fromJson(configFile.readAll()).object();
    QString ip = config["server_ip"].toString();
    int port = config["port"].toInt();

    return client->serverConnect(ip, port);
}
