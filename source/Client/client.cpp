#include "client.h"

#include <QDebug>


// Constructor: initializes socket and connects its signals to the appropriate slots
Client::Client(QObject *parent)
    : QObject(parent)
    , socket(new QTcpSocket(this))          // Create a new QTcpSocket, set this as its parent
{
    // Connect socket signals to class slots for handling various events
    connect(socket, &QTcpSocket::connected, this, &Client::onConnected);
    connect(socket, &QTcpSocket::disconnected, this, &Client::onDisconnected);
    connect(socket, &QTcpSocket::readyRead, this, &Client::onReadyRead);
    connect(socket, &QTcpSocket::errorOccurred, this, &Client::onError);
}

// Destructor: Ensures clean disconnection and socket deletion
Client::~Client()
{
    serverDisconnect();         // Disconnect from server if connected
    delete socket;          // Delete the socket to free memory
}

// Static instance pointer for singleton pattern
Client *Client::instance = nullptr;

//                      SLOTS                       //

// Called when socket successfully connects to the server
void Client::onConnected()
{
    emit connection_updated(true);          // Notify UI or other parts that connection is established
}

// Called when socket is disconnected
void Client::onDisconnected()
{
    emit connection_updated(false);         // Notify about lost connection
}

// Called when data is available to read from the server
void Client::onReadyRead()
{
    QByteArray data = socket->readAll();            // Read all incoming data
    QJsonDocument doc = QJsonDocument::fromJson(data);          // Parse it as a JSON document
    if (doc.isObject())         // Ensure it's a valid JSON object
        processResponse(doc.object());          // Process the server response
}

// Called when a socket error occurs
void Client::onError(QAbstractSocket::SocketError socketError)
{
    qDebug() << "Socket error: " << socketError << socket->errorString();           // Print error details
    emit connection_updated(false);         // Notify about the connection issue
}

//                      FUNCTIONS                       //

// Singleton accessor: creates a new instance if it doesn't exist
Client *Client::getInstance()
{
    if (!instance)
        instance = new Client();

    return instance;
}

// Singleton deleter: cleans up the instance
void Client::deleteInstance()
{
    if (instance) {
        delete instance;
        instance = nullptr;
    }
}

// Attempts to connect to the server at given host and port
bool Client::serverConnect(const QString &host,
                           quint16 port)
{
    if (socket->state() == QAbstractSocket::ConnectedState)
        return true;            // Already connected

    socket->connectToHost(host, port);          // Initiate connection

    return socket->waitForConnected(5000);          // Wait up to 5 seconds for connection
}

// Cleanly disconnects from the server if currently connected
void Client::serverDisconnect()
{
    if (socket->state() == QAbstractSocket::ConnectedState) {
        socket->disconnectFromHost();           // Ask socket to disconnect
        socket->waitForDisconnected(5000);          // Wait for disconnection to complete
    }
}

// Returns true if socket is currently connected to server
bool Client::connected() const
{
    return socket->state() == QAbstractSocket::ConnectedState;
}

// Sends signup request to server with username, password, and email
bool Client::signUpUser(const QString &username,
                        const QString &password,
                        const QString &email)
{
    if (!connected()) {
        emit signup_result(false, "Not connected to server");           // Emit error if not connected

        return false;
    }

    // Construct the JSON request
    QJsonObject request;
    request["action"] = "sign_up";
    request["username"] = username;
    request["password"] = password;
    request["email"] = email;

    sendRequest(request);           // Send it to the server

    return true;
}

// Sends login request to server with username and password
bool Client::logInUser(const QString &username,
                       const QString &password)
{
    if (!connected()) {
        emit login_result(false, "Error: not connected to server");         // Emit error if not connected

        return false;
    }

    // Construct the JSON request
    QJsonObject request;
    request["action"] = "log_in";
    request["username"] = username;
    request["password"] = password;

    sendRequest(request);           // Send it to the server

    return true;
}

// Processes server's JSON response
void Client::processResponse(const QJsonObject &response)
{
    QString message = response["message"].toString();           // Extract message from response

    // Emit appropriate signal based on message content
    if (message.contains("Signed up"))
        emit signup_result(true, message);
    else if (message.contains("Signup error"))
        emit signup_result(false, message);
    else if (message.contains("Logged in"))
        emit login_result(true, message);
    else if (message.contains("Login error"))
        emit login_result(false, message);
}

// Sends a JSON request to the server
void Client::sendRequest(const QJsonObject &request)
{
    QJsonDocument doc(request);         // Convert JSON object to document
    QByteArray data = doc.toJson();         // Serialize to QByteArray
    socket->write(data);            // Send through socket
    socket->flush();            // Ensure immediate transmission
}
