#include <ntqapplication.h>
#include "mainform.h"

int main( int argc, char ** argv )
{
    TQApplication a( argc, argv );
    MainForm *w = new MainForm;
    w->show();
    return a.exec();
}
