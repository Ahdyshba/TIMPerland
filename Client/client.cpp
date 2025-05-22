#include "client.h"
#include <QDebug>

Client* Client::instance = nullptr;

Client* Client::getInstance()
{
    if (!instance) {
        instance = new Client();
    }
    return instance;
}

void Client::deleteInstance()
{
    if (instance) {
        delete instance;
        instance = nullptr;
    }
}

bool Client::serverConnect(const QString& host, quint16 port)
{
    if (socket->state() == QAbstractSocket::ConnectedState) {
        return true;
    }

    socket->connectToHost(host, port);
    return socket->waitForConnected(5000);
}

void Client::serverDisconnect()
{
    if (socket->state() == QAbstractSocket::ConnectedState) {
        socket->disconnectFromHost();
        socket->waitForDisconnected(5000);
    }
}

bool Client::connected() const
{
    return socket->state() == QAbstractSocket::ConnectedState;
}

Client::~Client()
{
    serverDisconnect();
    delete socket;
}

Client::Client(QObject *parent) : QObject(parent), socket(new QTcpSocket(this))
{
    connect(socket, &QTcpSocket::connected, this, &Client::onConnected);
    connect(socket, &QTcpSocket::disconnected, this, &Client::onDisconnected);
    connect(socket, &QTcpSocket::readyRead, this, &Client::onReadyRead);
    connect(socket, &QTcpSocket::errorOccurred, this, &Client::onError);
}

void Client::onConnected()
{
    emit connection_updated(true);
}

void Client::onDisconnected()
{
    emit connection_updated(false);
}

void Client::onReadyRead()
{
    QByteArray data = socket->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isObject()) {
        processResponse(doc.object());
    }
}

void Client::onError(QAbstractSocket::SocketError socketError)
{
    qDebug() << "Socket error: " << socketError << socket->errorString();
    emit connection_updated(false);
}

void Client::processResponse(const QJsonObject& response)
{
    QString message = response["message"].toString();

    if (message.contains("Signed up")) {
        emit signup_result(true, message);
    } else if (message.contains("Signup error")) {
        emit signup_result(false, message);
    } else if (message.contains("Logged in")) {
        emit login_result(true, message);
    } else if (message.contains("Login error")) {
        emit login_result(false, message);
    }
}

bool Client::signUpUser(const QString& username, const QString& password, const QString& email)
{
    if (!connected()) {
        emit signup_result(false, "Not connected to server");
        return false;
    }

    QJsonObject request;
    request["action"] = "sign_up";
    request["username"] = username;
    request["password"] = password;
    request["email"] = email;

    sendRequest(request);
    return true;
}

bool Client::logInUser(const QString& username, const QString& password)
{
    if (!connected()) {
        emit login_result(false, "Error: not connected to server");
        return false;
    }

    QJsonObject request;
    request["action"] = "log_in";
    request["username"] = username;
    request["password"] = password;

    sendRequest(request);
    return true;
}

void Client::sendRequest(const QJsonObject& request)
{
    QJsonDocument doc(request);
    QByteArray data = doc.toJson();
    socket->write(data);
    socket->flush();
}
