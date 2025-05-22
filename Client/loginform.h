#ifndef LOGINFORM_H
#define LOGINFORM_H


#include <QDialog>


namespace Ui {
class LoginForm;
}

class LoginForm : public QDialog
{
    Q_OBJECT

public:
    explicit LoginForm(QWidget *parent = nullptr);
    ~LoginForm();

private slots:
    void handleLoginResult(bool success, const QString& message);
    void handleSignupResult(bool success, const QString& message);

    void on_pushButton_to_signup_clicked();
    void on_pushButton_log_in_clicked();
    void on_pushButton_sign_up_clicked();

signals:
    void login_ok(QString);

private:
    Ui::LoginForm *ui;

    void changeTab(bool);
    void clearForm();
};


#endif // LOGINFORM_H
