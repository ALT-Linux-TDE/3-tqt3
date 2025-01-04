/****************************************************************************
** $Id$
**
** Implementation of TQNoneInputContextPlugin class
**
** Copyright (C) 2004 immodule for TQt Project.  All rights reserved.
**
** This file is written to contribute to Trolltech AS under their own
** licence. You may use this file under your TQt license. Following
** description is copied from their original file headers. Contact
** immodule-qt@freedesktop.org if any conditions of this licensing are
** not clear to you.
**
**
** This file is part of the input method module of the TQt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.TQPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid TQt Enterprise Edition or TQt Professional Edition
** licenses may use this file in accordance with the TQt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about TQt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for TQPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef TQT_NO_IM
#include "qnoneinputcontextplugin.h"
#include <ntqinputcontextfactory.h>
#include <ntqsettings.h>
#include <cstdlib>


TQNoneInputContextPlugin::TQNoneInputContextPlugin()
{
}

TQNoneInputContextPlugin::~TQNoneInputContextPlugin()
{
}

TQStringList TQNoneInputContextPlugin::keys() const
{
    // input method switcher should named with "imsw-" prefix to
    // prevent to be listed in ordinary input method list.
    return TQStringList( "imsw-none" );
}

TQInputContext *TQNoneInputContextPlugin::create( const TQString &key )
{
    TQString actuallySpecifiedKey;

    bool isIMSwitcher = key.startsWith( "imsw-" );
    if ( ! isIMSwitcher )
	return 0;

    if ( getenv( "TQT_IM_MODULE" ) ) {
        actuallySpecifiedKey = getenv( "TQT_IM_MODULE" );
    } else {
	TQSettings settings;
        actuallySpecifiedKey = settings.readEntry( "/qt/DefaultInputMethod", "xim" );
    }

    return TQInputContextFactory::create( actuallySpecifiedKey, 0 );
}

TQStringList TQNoneInputContextPlugin::languages( const TQString &key )
{
    return TQStringList( "" );
}

TQString TQNoneInputContextPlugin::displayName( const TQString &key )
{
    return tr( "Dummy Input Method Switcher" );
}

TQString TQNoneInputContextPlugin::description( const TQString &key )
{
    return tr( "Dummy input method switcher that uses the context menu of the text widgets" );
}


TQ_EXPORT_PLUGIN( TQNoneInputContextPlugin )

#endif
