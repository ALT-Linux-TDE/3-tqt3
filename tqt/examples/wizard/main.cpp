/****************************************************************************
**
** Copyright (C) 1992-2008 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for TQt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "wizard.h"
#include <ntqapplication.h>

int main(int argc,char **argv)
{
    TQApplication a(argc,argv);

    Wizard wizard;
    wizard.setCaption("TQt Example - Wizard");
    return wizard.exec();
}
