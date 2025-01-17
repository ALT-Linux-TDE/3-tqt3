/****************************************************************************
**
** Implementation of the TQTextView class
**
** Created : 990101
**
** Copyright (C) 1992-2008 Trolltech ASA.  All rights reserved.
**
** This file is part of the widgets module of the TQt GUI Toolkit.
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
** This file may be used under the terms of the Q Public License as
** defined by Trolltech ASA and appearing in the file LICENSE.TQPL
** included in the packaging of this file.  Licensees holding valid TQt
** Commercial licenses may use this file in accordance with the TQt
** Commercial License Agreement provided with the Software.
**
** This file is provided "AS IS" with NO WARRANTY OF ANY KIND,
** INCLUDING THE WARRANTIES OF DESIGN, MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE. Trolltech reserves all rights not granted
** herein.
**
**********************************************************************/

#include "ntqtextview.h"

#ifndef TQT_NO_TEXTVIEW

/*! \class TQTextView
    \brief The TQTextView class provides a rich-text viewer.

  \obsolete

  This class wraps a read-only \l TQTextEdit.
  Use a \l TQTextEdit instead, and call setReadOnly(TRUE)
  to disable editing.
*/

/*! \reimp */

TQTextView::TQTextView( const TQString& text, const TQString& context,
		      TQWidget *parent, const char *name )
    : TQTextEdit( text, context, parent, name )
{
    setReadOnly( TRUE );
}

/*! \reimp */

TQTextView::TQTextView( TQWidget *parent, const char *name )
    : TQTextEdit( parent, name )
{
    setReadOnly( TRUE );
}

/*! \reimp */

TQTextView::~TQTextView()
{
}

/*!
    \property TQTextView::undoDepth
    \brief the number of undoable steps
*/

/*!
    \property TQTextView::overwriteMode
    \brief whether new text overwrites or pushes aside existing text
*/

/*!
    \property TQTextView::modified
    \brief Whether the text view's contents have been modified.
*/

/*!
    \property TQTextView::readOnly
    \brief Whether the text view's contents are read only.
*/

/*!
    \property TQTextView::undoRedoEnabled
    \brief Whether undo and redo are enabled.
*/

#endif
