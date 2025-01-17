 /**********************************************************************
** Copyright (C) 2000-2008 Trolltech ASA.  All rights reserved.
**
** This file is part of TQt Designer.
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

#ifndef CLASSBROWSERINTERFACE_H
#define CLASSBROWSERINTERFACE_H

//
//  W A R N I N G  --  PRIVATE INTERFACES
//  --------------------------------------
//
// This file and the interfaces declared in the file are not
// public. It exists for internal purpose. This header file and
// interfaces may change from version to version (even binary
// incompatible) without notice, or even be removed.
//
// We mean it.
//
//

#include <private/qcom_p.h>
#include <ntqobject.h>
class TQListView;

// {4ede3c32-ae96-4b7a-9e38-9f1d93592391}
#ifndef IID_ClassBrowser
#define IID_ClassBrowser TQUuid( 0x4ede3c32, 0xae96, 0x4b7a, 0x9e, 0x38, 0x9f, 0x1d, 0x93, 0x59, 0x23, 0x91 )
#endif

struct ClassBrowserInterface : public TQUnknownInterface
{
    enum Type { Class, Function };

    virtual TQListView *createClassBrowser( TQWidget *parent ) const = 0;
    virtual void update( const TQString &code ) const = 0;
    virtual void clear() const = 0;

    virtual void onClick( TQObject *receiver, const char *slot ) = 0;
};

#endif
