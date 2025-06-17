#ifndef CLIENTFUNCS_H
#define CLIENTFUNCS_H

#include <QDebug>
#include <QObject>
#include <QString>

bool logIn(QString login, QString password);
bool signUp(QString login, QString password, QString email);
bool ensureServerConnection();

#endif // CLIENTFUNCS_H
