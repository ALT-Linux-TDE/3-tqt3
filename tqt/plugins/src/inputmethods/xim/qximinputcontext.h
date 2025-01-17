/****************************************************************************
** $Id: qximinputcontext_p.h,v 1.6 2004/06/22 06:47:27 daisuke Exp $
**
** Definition of TQXIMInputContext
**
** Copyright (C) 1992-2002 Trolltech AS.  All rights reserved.
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

#ifndef TQXIMINPUTCONTEXT_H
#define TQXIMINPUTCONTEXT_H


//
//  W A R N I N G
//  -------------
//
// This file is not part of the TQt API.  It exists for the convenience
// of internal files.  This header file may change from version to version
// without notice, or even be removed.
//
// We mean it.
//
//

#if !defined(Q_NO_IM)

#include "ntqglobal.h"
#include <ntqinputcontext.h>
#include <ntqfont.h>
#include <ntqcstring.h>

class TQKeyEvent;
class TQWidget;
class TQFont;
class TQString;


#ifdef TQ_WS_X11
#include "ntqarray.h"
#include "ntqwindowdefs.h"
#include <private/qt_x11_p.h>
#endif

class TQXIMInputContext : public TQInputContext
{
    TQ_OBJECT
public:
#ifdef TQ_WS_X11
    TQXIMInputContext();
    ~TQXIMInputContext();

    TQString identifierName();
    TQString language();

    bool x11FilterEvent( TQWidget *keywidget, XEvent *event );
    void reset();

    void setFocus();
    void unsetFocus();
    void setMicroFocus( int x, int y, int w, int h, TQFont *f = 0 );
    void mouseHandler( int x, TQEvent::Type type,
		       TQt::ButtonState button, TQt::ButtonState state );
    bool isPreeditRelocationEnabled();

    void setHolderWidget( TQWidget *widget );

    bool hasFocus() const;
    void resetClientState();
    void close( const TQString &errMsg );

    void sendIMEvent( TQEvent::Type type,
		      const TQString &text = TQString::null,
		      int cursorPosition = -1, int selLength = 0 );

    static void init_xim();
    static void create_xim();
    static void close_xim();

    void *ic;
    TQString composingText;
    TQFont font;
    XFontSet fontset;
    TQMemArray<bool> selectedChars;

protected:
    virtual bool isPreeditPreservationEnabled();  // not a TQInputContext func

    TQCString _language;

private:
    void setComposePosition(int, int);
    void setComposeArea(int, int, int, int);
    void setXFontSet(const TQFont &);

    int lookupString(XKeyEvent *, TQCString &, KeySym *, Status *) const;

#endif // TQ_WS_X11
};


#endif //Q_NO_IM

#endif // TQXIMINPUTCONTEXT_H
