#include "loginform.h"
#include "ui_loginform.h"
#include "clientfuncs.h"
#include "client.h"
#include "homewindow.h"
#include <QMessageBox>


LoginForm::~LoginForm()
{
    delete ui;
}

LoginForm::LoginForm(QWidget *parent) : QDialog(parent), ui(new Ui::LoginForm)
{
    ui->setupUi(this);
    changeTab(false);

    Client* client = Client::getInstance();
    connect(client, &Client::login_result, this, &LoginForm::handleLoginResult);
    connect(client, &Client::signup_result, this, &LoginForm::handleSignupResult);
}

void LoginForm::changeTab(bool tab)
{
    // Log In = false, Sign Up = true
    ui->label_password_check->setVisible(tab);
    ui->lineEdit_password_check->setVisible(tab);
    ui->label_email->setVisible(tab);
    ui->lineEdit_email->setVisible(tab);
    ui->pushButton_sign_up->setVisible(tab);
    ui->pushButton_log_in->setVisible(!tab);
    ui->pushButton_to_signup->setText(tab ? "To Login" : "To Sign-up");
    ui->label_header->setText(tab ? "Sign Up" : "Log In");
}

void LoginForm::handleLoginResult(bool success, const QString& message)
{
    if (success) {
        ui->label_status->setText("Login successful");
        HomeWindow *homeWindow = new HomeWindow();
        homeWindow->show();
        this->close();
    } else {
        ui->label_status->setText(message);
        clearForm();
    }
}

void LoginForm::handleSignupResult(bool success, const QString& message)
{
    if (success) {
        ui->label_status->setText("Signup successful. Please login");
        changeTab(false);
        clearForm();
    } else {
        ui->label_status->setText(message);
        clearForm();
    }
}

void LoginForm::clearForm()
{
    ui->lineEdit_login->clear();
    ui->lineEdit_password->clear();
    ui->lineEdit_password_check->clear();
    ui->lineEdit_email->clear();
}

void LoginForm::on_pushButton_to_signup_clicked()
{
    changeTab(!ui->label_email->isVisible());
}

void LoginForm::on_pushButton_log_in_clicked()
{
    QString login = ui->lineEdit_login->text();
    QString password = ui->lineEdit_password->text();
    
    if (login.isEmpty() || password.isEmpty()) {
        ui->label_status->setText("Please fill in all fields");
        return;
    }
    
    ui->label_status->setText("Logging in...");
    logIn(login, password);
}

void LoginForm::on_pushButton_sign_up_clicked()
{
    QString login = ui->lineEdit_login->text();
    QString password = ui->lineEdit_password->text();
    QString passwordCheck = ui->lineEdit_password_check->text();
    QString email = ui->lineEdit_email->text();
    
    if (login.isEmpty() || password.isEmpty() || email.isEmpty()) {
        ui->label_status->setText("Please fill in all fields");
        return;
    }
    
    if (password != passwordCheck) {
        ui->label_status->setText("Passwords do not match");
        return;
    }
    
    ui->label_status->setText("Signing up...");
    signUp(login, password, email);
}


