#include "forms.h"


Forms::~Forms() {}

Forms::Forms(QWidget *parent)
    : QMainWindow(parent)
{
    this->loginForm = new LoginForm();
    this->loginForm->show();
}
