/**********************************************************************
**
** Copyright (C) 2005-2008 Trolltech ASA.  All rights reserved.
**
** This file is part of TQt Designer
**
** This file may be used under the terms of the GNU General
** Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the files LICENSE.GPL2
** and LICENSE.GPL3 included in the packaging of this file.
** Alternatively you may (at your option) use any later version
** of the GNU General Public License if such license has been
** publicly approved by Trolltech ASA (or its successors, if any)
** and the KDE Free TQt Foundation.
**
** Please review the following information to ensure GNU General
** Public Licensing requirements will be met:
** http://trolltech.com/products/qt/licenses/licensing/opensource/.
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** Licensees holding valid TQt Commercial licenses may use this file in
** accordance with the TQt Commercial License Agreement provided with
** the Software.
**
** This file is provided "AS IS" with NO WARRANTY OF ANY KIND,
** INCLUDING THE WARRANTIES OF DESIGN, MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE. Trolltech reserves all rights not granted
** herein.
**
**********************************************************************/

#include <filterinterface.h>

#include <ntqapplication.h>
#include <ntqstring.h>
#include <ntqdir.h>

#include "kdevdlg2ui.h"

class KDevDlgFilter : public ImportFilterInterface, public TQLibraryInterface
{
public:
    KDevDlgFilter();

    TQRESULT queryInterface( const TQUuid&, TQUnknownInterface **iface );
    TQ_REFCOUNT;

    TQStringList featureList() const;
    TQStringList import( const TQString& filter, const TQString& filename );

    bool init();
    void cleanup();
    bool canUnload() const;
};

KDevDlgFilter::KDevDlgFilter()
{
}

TQRESULT KDevDlgFilter::queryInterface( const TQUuid &uuid, TQUnknownInterface **iface )
{
    *iface = 0;
    if ( uuid == IID_QUnknown )
	*iface = (TQUnknownInterface*)(ImportFilterInterface*)this;
    else if ( uuid == IID_QFeatureList )
	*iface = (TQFeatureListInterface*)this;
    else if ( uuid == IID_ImportFilter )
	*iface = (ImportFilterInterface*)this;
    else if ( uuid == IID_QLibrary )
	*iface = (TQLibraryInterface*)this;
    else
	return TQE_NOINTERFACE;

    (*iface)->addRef();
    return TQS_OK;
}

TQStringList KDevDlgFilter::featureList() const
{
    TQStringList list;
    list << "KDevelop Dialog Files (*.kdevdlg)" ;
    return list;
}

TQStringList KDevDlgFilter::import( const TQString &, const TQString& filename )
{
    TQFile file( filename );
    if ( !file.open( IO_ReadOnly ) )
	tqWarning( "uic: Could not open file '%s' ", filename.latin1() );
    TQTextStream in;
    in.setDevice( &file );

    TQString name = filename.right( filename.length() - filename.findRev( TQDir::separator() ) - 1 ).section( ".", 0, 0 );
    KDEVDLG2UI c( &in, name );
    TQStringList files;
    c.parse();
    return c.targetFiles;
}

bool KDevDlgFilter::init()
{
    return TRUE;
}

void KDevDlgFilter::cleanup()
{
}

bool KDevDlgFilter::canUnload() const
{
    return TRUE;
}

Q_EXPORT_COMPONENT()
{
    Q_CREATE_INSTANCE( KDevDlgFilter )
}
