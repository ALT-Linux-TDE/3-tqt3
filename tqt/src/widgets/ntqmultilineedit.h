/**********************************************************************
**
** Definition of TQMultiLineEdit widget class
**
** Created : 961005
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

#ifndef TQMULTILINEEDIT_H
#define TQMULTILINEEDIT_H

#ifndef QT_H
#include "ntqtextedit.h"
#endif // QT_H

#ifndef TQT_NO_MULTILINEEDIT

class TQMultiLineEditCommand;
class TQValidator;
class TQMultiLineEditData;

class TQ_EXPORT TQMultiLineEdit : public TQTextEdit
{
    TQ_OBJECT
    TQ_PROPERTY( int numLines READ numLines )
    TQ_PROPERTY( bool atBeginning READ atBeginning )
    TQ_PROPERTY( bool atEnd READ atEnd )
    TQ_PROPERTY( Alignment alignment READ alignment WRITE setAlignment )
    TQ_PROPERTY( bool edited READ edited WRITE setEdited DESIGNABLE false )

public:
    TQMultiLineEdit( TQWidget* parent=0, const char* name=0 );
    ~TQMultiLineEdit();

    TQString textLine( int line ) const;
    int numLines() const;

    virtual void insertLine( const TQString &s, int line = -1 );
    virtual void insertAt( const TQString &s, int line, int col ) {
	insertAt( s, line, col, FALSE );
    }
    virtual void insertAt( const TQString &s, int line, int col, bool mark );
    virtual void removeLine( int line );
    virtual void setCursorPosition( int line, int col ) {
	setCursorPosition( line, col, FALSE );
    }
    virtual void setCursorPosition( int line, int col, bool mark );
    bool atBeginning() const;
    bool atEnd() const;

    void setAlignment( int flags );
    int alignment() const;

    void setEdited( bool );
    bool edited() const;

    bool hasMarkedText() const;
    TQString markedText() const;

    void cursorWordForward( bool mark );
    void cursorWordBackward( bool mark );

    // noops
    bool autoUpdate() const { return TRUE; }
    virtual void setAutoUpdate( bool ) {}

    int totalWidth() const { return contentsWidth(); }
    int totalHeight() const { return contentsHeight(); }

    int maxLines() const { return TQWIDGETSIZE_MAX; }
    void setMaxLines( int ) {}

public slots:
    void deselect() { selectAll( FALSE ); }

protected:
    TQPoint	cursorPoint() const;

protected:
    virtual void insertAndMark( const TQString&, bool mark );
    virtual void newLine();
    virtual void killLine();
    virtual void pageUp( bool mark=FALSE );
    virtual void pageDown( bool mark=FALSE );
    virtual void cursorLeft( bool mark=FALSE, bool wrap = TRUE );
    virtual void cursorRight( bool mark=FALSE, bool wrap = TRUE );
    virtual void cursorUp( bool mark=FALSE );
    virtual void cursorDown( bool mark=FALSE );
    virtual void backspace();
    virtual void home( bool mark=FALSE );
    virtual void end( bool mark=FALSE );

    bool getMarkedRegion( int *line1, int *col1,
			  int *line2, int *col2 ) const;
    int lineLength( int row ) const;

private:
    TQMultiLineEditData *d;

private:	// Disabled copy constructor and operator=
#if defined(TQ_DISABLE_COPY)
    TQMultiLineEdit( const TQMultiLineEdit & );
    TQMultiLineEdit &operator=( const TQMultiLineEdit & );
#endif
};

#endif // TQT_NO_MULTILINEEDIT

#endif // TQMULTILINED_H
