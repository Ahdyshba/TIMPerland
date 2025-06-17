#include "forms.h"


// Constructor for Forms class
Forms::Forms(QWidget *parent)
    : QMainWindow(parent)           // Initialize base QMainWindow with parent
{
    this->loginForm = new LoginForm();          // Create the login form when the application starts
    this->loginForm->show();            // Show the login form window
}

// Destructor for Forms class
Forms::~Forms() {}
