#ifndef FORMS_H
#define FORMS_H


#include <QMainWindow>
#include "loginform.h"
#include "mainwindow.h"


class Forms : public QMainWindow
{
    Q_OBJECT

public:
    Forms(QWidget *parent = nullptr);
    ~Forms();

private:
    LoginForm *loginForm;
    MainWindow *mainForm;
};


#endif // FORMS_H
