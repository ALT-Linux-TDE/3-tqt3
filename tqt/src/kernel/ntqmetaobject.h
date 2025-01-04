/****************************************************************************
**
** Definition of TQMetaObject class
**
** Created : 930419
**
** Copyright (C) 1992-2008 Trolltech ASA.  All rights reserved.
**
** This file is part of the kernel module of the TQt GUI Toolkit.
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

#ifndef TQMETAOBJECT_H
#define TQMETAOBJECT_H

#ifndef QT_H
#include "ntqconnection.h"
#include "ntqstrlist.h"
#endif // QT_H

#ifndef Q_MOC_OUTPUT_REVISION
#define Q_MOC_OUTPUT_REVISION 26
#endif

class TQObject;
struct TQUMethod;
class TQMetaObjectPrivate;

struct TQMetaData				// - member function meta data
{						//   for signal and slots
    const char *name;				// - member name
    const TQUMethod* method;			// - detailed method description
    enum Access { Private, Protected, Public };
    Access access;				// - access permission
};

#ifndef TQT_NO_PROPERTIES
struct TQMetaEnum				// enumerator meta data
{						//  for properties
    const char *name;				// - enumerator name
    uint count;					// - number of values
    struct Item					// - a name/value pair
    {
	const char *key;
	int value;
    };
    const Item *items;				// - the name/value pairs
    bool set;					// whether enum has to be treated as a set
};
#endif

#ifndef TQT_NO_PROPERTIES

class TQ_EXPORT TQMetaProperty			// property meta data
{
public:
    const char*	type() const { return t; }	// type of the property
    const char*	name() const { return n; }	// name of the property

    bool writable() const;
    bool isValid() const;

    bool isSetType() const;
    bool isEnumType() const;
    TQStrList enumKeys() const;			// enumeration names

    int keyToValue( const char* key ) const;	// enum and set conversion functions
    const char* valueToKey( int value ) const;
    int keysToValue( const TQStrList& keys ) const;
    TQStrList valueToKeys( int value ) const;

    bool designable( TQObject* = 0 ) const;
    bool scriptable( TQObject* = 0 ) const;
    bool stored( TQObject* = 0 ) const;

    bool reset( TQObject* ) const;

    const char* t;			// internal
    const char* n;			// internal

    enum Flags  {
	Invalid		= 0x00000000,
	Readable	= 0x00000001,
	Writable	= 0x00000002,
	EnumOrSet	= 0x00000004,
	UnresolvedEnum	= 0x00000008,
	StdSet		= 0x00000100,
	Override	= 0x00000200
    };

    uint flags; // internal
    bool testFlags( uint f ) const;	// internal
    bool stdSet() const; 		// internal
    int id() const; 			// internal

    TQMetaObject** meta; 		// internal

    const TQMetaEnum* enumData;		// internal
    int _id; 				// internal
    void clear(); 			// internal
};

inline bool TQMetaProperty::testFlags( uint f ) const
{ return (flags & (uint)f) != (uint)0; }

#endif // TQT_NO_PROPERTIES

struct TQClassInfo				// class info meta data
{
    const char* name;				// - name of the info
    const char* value;				// - value of the info
};

class TQ_EXPORT TQMetaObject			// meta object class
{
public:
    TQMetaObject( const char * const class_name, TQMetaObject *superclass,
		 const TQMetaData * const slot_data, int n_slots,
		 const TQMetaData * const signal_data, int n_signals,
#ifndef TQT_NO_PROPERTIES
		 const TQMetaProperty *const prop_data, int n_props,
		 const TQMetaEnum *const enum_data, int n_enums,
#endif
		 const TQClassInfo *const class_info, int n_info );

#ifndef TQT_NO_PROPERTIES
    TQMetaObject( const char * const class_name, TQMetaObject *superclass,
		 const TQMetaData * const slot_data, int n_slots,
		 const TQMetaData * const signal_data, int n_signals,
		 const TQMetaProperty *const prop_data, int n_props,
		 const TQMetaEnum *const enum_data, int n_enums,
		 bool (*tqt_static_property)(TQObject*, int, int, TQVariant*),
		 const TQClassInfo *const class_info, int n_info );
#endif


    virtual ~TQMetaObject();

    const char	*className()		const { return classname; }
    const char	*superClassName()	const { return superclassname; }

    TQMetaObject *superClass()		const { return superclass; }

    bool	inherits( const char* clname ) const;

    int	numSlots( bool super = FALSE ) const;
    int		numSignals( bool super = FALSE ) const;

    int		findSlot( const char *, bool super = FALSE ) const;
    int		findSignal( const char *, bool super = FALSE ) const;

    const TQMetaData 	*slot( int index, bool super = FALSE ) const;
    const TQMetaData 	*signal( int index, bool super = FALSE ) const;

    TQStrList	slotNames( bool super = FALSE ) const;
    TQStrList	signalNames( bool super = FALSE ) const;

    int		slotOffset() const;
    int		signalOffset() const;
    int		propertyOffset() const;

    int		numClassInfo( bool super = FALSE ) const;
    const TQClassInfo	*classInfo( int index, bool super = FALSE ) const;
    const char	*classInfo( const char* name, bool super = FALSE ) const;

#ifndef TQT_NO_PROPERTIES
    const TQMetaProperty	*property( int index, bool super = FALSE ) const;
    int findProperty( const char *name, bool super = FALSE ) const;
    int indexOfProperty( const TQMetaProperty*, bool super = FALSE ) const;
    const TQMetaProperty* resolveProperty( const TQMetaProperty* ) const;
    int resolveProperty( int ) const;
    TQStrList		propertyNames( bool super = FALSE ) const;
    int		numProperties( bool super = FALSE ) const;
#endif

    // static wrappers around constructors, necessary to work around a
    // Windows-DLL limitation: objects can only be deleted within a
    // DLL if they were actually created within that DLL.
    static TQMetaObject	*new_metaobject( const char *, TQMetaObject *,
					const TQMetaData *const, int,
					const TQMetaData *const, int,
#ifndef TQT_NO_PROPERTIES
					const TQMetaProperty *const prop_data, int n_props,
					const TQMetaEnum *const enum_data, int n_enums,
#endif
					const TQClassInfo *const  class_info, int n_info );
#ifndef TQT_NO_PROPERTIES
    static TQMetaObject	*new_metaobject( const char *, TQMetaObject *,
					const TQMetaData *const, int,
					const TQMetaData *const, int,
					const TQMetaProperty *const prop_data, int n_props,
					const TQMetaEnum *const enum_data, int n_enums,
					 bool (*tqt_static_property)(TQObject*, int, int, TQVariant*),
					const TQClassInfo *const  class_info, int n_info );
    TQStrList		enumeratorNames( bool super = FALSE ) const;
    int numEnumerators( bool super = FALSE ) const;
    const TQMetaEnum		*enumerator( const char* name, bool super = FALSE ) const;
#endif

    static TQMetaObject *metaObject( const char *class_name );
    static bool hasMetaObject( const char *class_name );

private:
    TQMemberDict		*init( const TQMetaData *, int );

    const char		*classname;		// class name
    const char		*superclassname;	// super class name
    TQMetaObject	*superclass;			// super class meta object
    TQMetaObjectPrivate	*d;			// private data for...
    void	*reserved;			// ...binary compatibility
    const TQMetaData		*slotData;	// slot meta data
    TQMemberDict	*slotDict;			// slot dictionary
    const TQMetaData		*signalData;	// signal meta data
    TQMemberDict	*signalDict;			// signal dictionary
    int signaloffset;
    int slotoffset;
#ifndef TQT_NO_PROPERTIES
    int propertyoffset;
public:
    bool tqt_static_property( TQObject* o, int id, int f, TQVariant* v);
private:
    friend class TQMetaProperty;
#endif

private:	// Disabled copy constructor and operator=
#if defined(TQ_DISABLE_COPY)
    TQMetaObject( const TQMetaObject & );
    TQMetaObject &operator=( const TQMetaObject & );
#endif
};

inline int TQMetaObject::slotOffset() const
{ return slotoffset; }

inline int TQMetaObject::signalOffset() const
{ return signaloffset; }

#ifndef TQT_NO_PROPERTIES
inline int TQMetaObject::propertyOffset() const
{ return propertyoffset; }
#endif

typedef TQMetaObject *(*TQtStaticMetaObjectFunction)();

class TQ_EXPORT TQMetaObjectCleanUp
{
public:
    TQMetaObjectCleanUp( const char *mo_name, TQtStaticMetaObjectFunction );
    TQMetaObjectCleanUp();
    ~TQMetaObjectCleanUp();

    void setMetaObject( TQMetaObject *&mo );

private:
    TQMetaObject **metaObject;
};

#endif // TQMETAOBJECT_H
