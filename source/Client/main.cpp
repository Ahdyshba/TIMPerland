#include "forms.h"

#include <QApplication>


int main(int argc,
         char *argv[])
{
    QApplication a(argc, argv);         // Create the main Qt application object
    Forms form;         // Create the main Forms object, which opens the login form

    return a.exec();            // Enter the Qt event loop — this keeps the application running
}
