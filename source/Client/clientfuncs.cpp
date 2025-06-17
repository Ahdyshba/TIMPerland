#include "clientfuncs.h"
#include "client.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QString>


// Attempts to log in a user with the given credentials
bool logIn(QString login,
           QString password)
{
    // Ensure there is a connection to the server
    if (!ensureServerConnection()) {
        qDebug() << "Login error: failed to connect to server";

        return false;
    }

    Client *client = Client::getInstance();         // Get the singleton Client instance
    bool result = client->logInUser(login, password);           // Try logging in, returns immediately (asynchronous result)

    // Connect to the login_result signal to capture the result
    QObject::connect(client, &Client::login_result, [&result](bool success, const QString &message) {
        result = success;           // This line will run when the signal is emitted
        qDebug() << "Login result: " << message;
    });

    // NOTE: Due to signal-slot being asynchronous, this return may be too early to reflect success, but this doesn't affect LAN-only connections as much as WAN connections
    return result;
}

// Attempts to sign up a new user
bool signUp(QString login,
            QString password,
            QString email)
{
    // Ensure the client is connected to the server
    if (!ensureServerConnection()) {
        qDebug() << "Signup error: failed to connect to server";

        return false;
    }

    Client *client = Client::getInstance();         // Get the singleton Client instance
    bool result = client->signUpUser(login, password, email);           // Try signing up, result is initially based on request success

    // Connect to the signup_result signal to handle server response
    QObject::connect(client, &Client::signup_result, [&result](bool success, const QString &message) {
        result = success;          // Capture whether signup was successful
        qDebug() << "Signup result: " << message;
    });

    // NOTE: Like login, this function may return before the signal is received
    return result;
}

// Ensures the client is connected to the server using settings in config.json
bool ensureServerConnection()
{
    Client *client = Client::getInstance();

    // If already connected, no need to reconnect
    if (client->connected())
        return true;

    // Load server settings from local config file
    QFile configFile(QDir::currentPath() + "/config.json");
    if (!configFile.open(QIODevice::ReadOnly)) {
        // If config file couldn't be opened, log an error and return
        qDebug() << "Connection error: failed to open config.json";
        qDebug() << "Current directory: " << QDir::currentPath();

        return false;
    }

    // Parse config file JSON
    QJsonObject config = QJsonDocument::fromJson(configFile.readAll()).object();
    QString ip = config["server_ip"].toString();            // Read IP
    int port = config["port"].toInt();          // Read port

    return client->serverConnect(ip, port);         // Attempt to connect to the server using loaded IP and port
}
