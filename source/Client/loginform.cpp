#include "loginform.h"
#include "client.h"
#include "clientfuncs.h"
#include "homewindow.h"
#include "ui_loginform.h"

#include <QMessageBox>


// Constructor: Initializes the login form UI and connects signals to slots
LoginForm::LoginForm(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoginForm)
{
    ui->setupUi(this);          // Set up the UI from the designer
    changeTab(false);           // Start in "Log In" mode

    Client *client = Client::getInstance();         // Get singleton instance of the client

    // Connect client signals to appropriate handlers
    connect(client, &Client::login_result, this, &LoginForm::handleLoginResult);
    connect(client, &Client::signup_result, this, &LoginForm::handleSignupResult);

    // Validator for login: only allows 3–16 characters of a-z, A-Z, 0–9, and _.-
    QRegularExpression loginRegex(R"(^[a-zA-Z0-9_.-]{3,16}$)");
    ui->lineEdit_login->setValidator(new QRegularExpressionValidator(loginRegex, this));

    // Set maximum password lengths
    ui->lineEdit_password->setMaxLength(32);
    ui->lineEdit_password_check->setMaxLength(32);

    // Hide password inputs for security
    ui->lineEdit_password->setEchoMode(QLineEdit::Password);
    ui->lineEdit_password_check->setEchoMode(QLineEdit::Password);
}

// Destructor: Cleans up dynamically allocated UI resources
LoginForm::~LoginForm()
{
    delete ui;
}

// Toggles between Log In and Sign Up tabs
void LoginForm::changeTab(bool tab)
{
    // Show additional fields only in Sign Up mode
    // Log In = false, Sign Up = true
    ui->label_password_check->setVisible(tab);
    ui->lineEdit_password_check->setVisible(tab);
    ui->label_email->setVisible(tab);
    ui->lineEdit_email->setVisible(tab);
    ui->sign_up->setVisible(tab);           // Show Sign Up button
    ui->log_in->setVisible(!tab);           // Show Log In button
    ui->switch_tabs->setText(tab ? "To Login" : "To Sign-up");
    ui->label_header->setText(tab ? "Sign Up" : "Log In");
}

// Slot to handle the result of a login attempt
void LoginForm::handleLoginResult(bool success,
                                  const QString &message)
{
    if (success) {
        ui->label_status->setText("Login successful");
        HomeWindow *homeWindow = new HomeWindow();          // Create and show main window
        homeWindow->show();
        this->close();          // Close login form
    } else {
        ui->label_status->setText(message);         // Display error message
        clearForm();            // Reset fields
    }
}

// Slot to handle the result of a signup attempt
void LoginForm::handleSignupResult(bool success,
                                   const QString &message)
{
    if (success) {
        ui->label_status->setText("Signup successful. Please login");
        changeTab(false);           // Switch to login mode
        clearForm();            // Reset fields
    } else {
        ui->label_status->setText(message);         // Show signup error
        clearForm();
    }
}

// Clears all input fields in the form
void LoginForm::clearForm()
{
    ui->lineEdit_login->clear();
    ui->lineEdit_password->clear();
    ui->lineEdit_password_check->clear();
    ui->lineEdit_email->clear();
}

// Slot called when the user clicks "To Sign-up" or "To Login"
void LoginForm::on_switch_tabs_clicked()
{
    // Flip mode based on current visibility of email label
    changeTab(!ui->label_email->isVisible());
}

// Slot for handling login button click
void LoginForm::on_log_in_clicked()
{
    QString login = ui->lineEdit_login->text();
    QString password = ui->lineEdit_password->text();

    // Validate required fields
    if (login.isEmpty() || password.isEmpty()) {
        ui->label_status->setText("Please fill in all fields");

        return;
    }

    // Password must be within allowed length
    if (password.length() < 8 || password.length() > 32) {
        ui->label_status->setText("Password must be 8–32 characters long");

        return;
    }

    ui->label_status->setText("Logging in...");
    logIn(login, password);         // Call the function to perform login
}

// Slot for handling sign up button click
void LoginForm::on_sign_up_clicked()
{
    QString login = ui->lineEdit_login->text();
    QString password = ui->lineEdit_password->text();
    QString passwordCheck = ui->lineEdit_password_check->text();
    QString email = ui->lineEdit_email->text();

    // Validate required fields
    if (login.isEmpty() || password.isEmpty() || email.isEmpty()) {
        ui->label_status->setText("Please fill in all fields");

        return;
    }

    // Validate email format
    QRegularExpression emailRegex(R"(^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Za-z]{2,}$)");
    if (!emailRegex.match(email).hasMatch()) {
        ui->label_status->setText("Invalid email format");

        return;
    }

    // Password length check
    if (password.length() < 8 || password.length() > 32) {
        ui->label_status->setText("Password must be 8–32 characters long");

        return;
    }

    // Password confirmation check
    if (password != passwordCheck) {
        ui->label_status->setText("Passwords do not match");

        return;
    }

    ui->label_status->setText("Signing up...");
    signUp(login, password, email);         // Call the function to perform signup
}
