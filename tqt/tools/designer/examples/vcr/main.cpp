#include <ntqapplication.h>
#include "vcr.h"

int main( int argc, char ** argv )
{
    TQApplication app( argc, argv );
    Vcr *vcr = new Vcr;
    vcr->show();
    return app.exec();
}
