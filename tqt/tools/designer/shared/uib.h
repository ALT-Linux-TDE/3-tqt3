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

#ifndef UIB_H
#define UIB_H

#include <ntqdatastream.h>

const TQ_UINT32 UibMagic = 0xb77c61d8;

enum BlockTag { Block_End = '$', Block_Actions = 'A', Block_Buddies = 'B',
		Block_Connections = 'C', Block_Functions = 'F',
		Block_Images = 'G', Block_Intro = 'I', Block_Menubar = 'M',
		Block_Slots = 'S', Block_Strings = 'Z', Block_Tabstops = 'T',
		Block_Toolbars = 'O', Block_Variables = 'V',
		Block_Widget = 'W' };

enum ObjectTag { Object_End = '$', Object_ActionRef = 'X',
		 Object_Attribute = 'B', Object_Column = 'C',
		 Object_Event = 'E', Object_FontProperty = 'F',
		 Object_GridCell = 'G', Object_Item = 'I',
		 Object_MenuItem = 'M', Object_PaletteProperty = 'P',
		 Object_Row = 'R', Object_Separator = 'S', Object_Spacer = 'Y',
		 Object_SubAction = 'A', Object_SubLayout = 'L',
		 Object_SubWidget = 'W', Object_TextProperty = 'T',
		 Object_VariantProperty = 'V' };

enum PaletteTag { Palette_End = '$', Palette_Active = 'A',
		  Palette_Inactive = 'I', Palette_Disabled = 'D',
		  Palette_Color = 'C', Palette_Pixmap = 'P' };

enum IntroFlag { Intro_Pixmapinproject = 0x1 };

enum FontFlag { Font_Family = 0x1, Font_PointSize = 0x2, Font_Bold = 0x4,
		Font_Italic = 0x8, Font_Underline = 0x10,
		Font_StrikeOut = 0x20 };

enum ConnectionFlag { Connection_Language = 0x1, Connection_Sender = 0x2,
		      Connection_Signal = 0x4, Connection_Receiver = 0x8,
		      Connection_Slot = 0x10 };

class UibStrTable
{
public:
    UibStrTable();

    inline int insertCString( const char *cstr );
    inline int insertString( const TQString& str );
    inline void readBlock( TQDataStream& in, int size );

    inline const char *asCString( int offset ) const;
    inline TQString asString( int offset ) const;
    inline TQByteArray block() const;

private:
    TQCString table;
    TQDataStream out;
    int start;
};

/*
  uic uses insertCString(), insertString(), and block();
  TQWidgetFactory uses readBlock(), asCString(), and asString(). By
  implementing these functions inline, we ensure that the binaries
  don't contain needless code.
*/

inline int UibStrTable::insertCString( const char *cstr )
{
    if ( cstr == 0 || cstr[0] == 0 ) {
	return 0;
    } else {
	int nextPos = table.size();
	int len = (int)strlen( cstr );
	int i;
	for ( i = 0; i < nextPos - len; i++ ) {
	    if ( memcmp(table.data() + i, cstr, len + 1) == 0 )
		return i;
	}
	for ( i = 0; i < len + 1; i++ )
	    out << (TQ_UINT8) cstr[i];
	return nextPos;
    }
}

inline int UibStrTable::insertString( const TQString& str )
{
    if ( str.contains('\0') || str[0] == TQChar(0x7f) ) {
	int nextPos = table.size();
	out << (TQ_UINT8) 0x7f;
	out << str;
	return nextPos;
    } else {
	return insertCString( str.utf8() );
    }
}

inline void UibStrTable::readBlock( TQDataStream& in, int size )
{
    table.resize( start + size );
    in.readRawBytes( table.data() + start, size );
}

inline TQString UibStrTable::asString( int offset ) const
{
    if ( table[offset] == 0x7f ) {
	TQDataStream in( table, IO_ReadOnly );
	in.device()->at( offset + 1 );
	TQString str;
	in >> str;
	return str;
    } else {
	return TQString::fromUtf8( asCString(offset) );
    }
}

inline const char *UibStrTable::asCString( int offset ) const
{
    return table.data() + offset;
}

inline TQByteArray UibStrTable::block() const
{
    TQByteArray block;
    block.duplicate( table.data() + start, table.size() - start );
    return block;
}

#endif
