#ifndef CLIENT_H
#define CLIENT_H


#include <QObject>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QHostAddress>


class Client : public QObject
{
    Q_OBJECT

public:
    static Client* getInstance();
    static void deleteInstance();

    bool serverConnect(const QString& host, quint16 port);
    void serverDisconnect();
    bool connected() const;

    bool logInUser(const QString& username, const QString& password);
    bool signUpUser(const QString& username, const QString& password, const QString& email);

signals:
    void connection_updated(bool connected);
    void signup_result(bool success, const QString& message);
    void login_result(bool success, const QString& message);

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onError(QAbstractSocket::SocketError socketError);

private:
    static Client* instance;
    QTcpSocket* socket;
    ~Client();
    explicit Client(QObject *parent = nullptr);

    void processResponse(const QJsonObject& response);
    void sendRequest(const QJsonObject& request);
};


#endif // CLIENT_H
