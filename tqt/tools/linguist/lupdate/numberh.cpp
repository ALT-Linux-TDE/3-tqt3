/**********************************************************************
** Copyright (C) 2000-2008 Trolltech ASA.  All rights reserved.
**
** This file is part of TQt Linguist.
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

#include <metatranslator.h>

#include <ntqmemarray.h>
#include <ntqcstring.h>
#include <ntqmap.h>
#include <ntqstringlist.h>

#include <ctype.h>

typedef TQMap<TQCString, MetaTranslatorMessage> TMM;
typedef TQValueList<MetaTranslatorMessage> TML;

static bool isDigitFriendly( int c )
{
    return ispunct((uchar)c) || isspace((uchar)c);
}

static int numberLength( const char *s )
{
    int i = 0;

    if ( isdigit((uchar)s[0]) ) {
	do {
	    i++;
	} while (isdigit((uchar)s[i]) ||
		 (isDigitFriendly(s[i]) &&
		  (isdigit((uchar)s[i + 1]) ||
		   (isDigitFriendly(s[i + 1]) && isdigit((uchar)s[i + 2])))));
    }
    return i;
}

/*
  Returns a version of 'key' where all numbers have been replaced by zeroes.  If
  there were none, returns "".
*/
static TQCString zeroKey( const char *key )
{
    TQCString zeroed( (int)strlen(key) + 1 );
    char *z = zeroed.data();
    int i = 0, j = 0;
    int len;
    bool metSomething = FALSE;

    while ( key[i] != '\0' ) {
	len = numberLength( key + i );
	if ( len > 0 ) {
	    i += len;
	    z[j++] = '0';
	    metSomething = TRUE;
	} else {
	    z[j++] = key[i++];
	}
    }
    z[j] = '\0';

    if ( metSomething )
	return zeroed;
    else
	return "";
}

static TQString translationAttempt( const TQString& oldTranslation,
				   const char *oldSource,
				   const char *newSource )
{
    int p = zeroKey( oldSource ).contains( '0' );
    int oldSourceLen = tqstrlen( oldSource );
    TQString attempt;
    TQStringList oldNumbers;
    TQStringList newNumbers;
    TQMemArray<bool> met( p );
    TQMemArray<int> matchedYet( p );
    int i, j;
    int k = 0, ell, best;
    int m, n;
    int pass;

    /*
      This algorithm is hard to follow, so we'll consider an example
      all along: oldTranslation is "XeT 3.0", oldSource is "TeX 3.0"
      and newSource is "XeT 3.1".

      First, we set up two tables: oldNumbers and newNumbers. In our
      example, oldNumber[0] is "3.0" and newNumber[0] is "3.1".
    */
    for ( i = 0, j = 0; i < oldSourceLen; i++, j++ ) {
	m = numberLength( oldSource + i );
	n = numberLength( newSource + j );
	if ( m > 0 ) {
	    oldNumbers.append( TQCString(oldSource + i, m + 1) );
	    newNumbers.append( TQCString(newSource + j, n + 1) );
	    i += m;
	    j += n;
	    met[k] = FALSE;
	    matchedYet[k] = 0;
	    k++;
	}
    }

    /*
      We now go over the old translation, "XeT 3.0", one letter at a
      time, looking for numbers found in oldNumbers. Whenever such a
      number is met, it is replaced with its newNumber equivalent. In
      our example, the "3.0" of "XeT 3.0" becomes "3.1".
    */
    for ( i = 0; i < (int) oldTranslation.length(); i++ ) {
	attempt += oldTranslation[i];
	for ( k = 0; k < p; k++ ) {
	    if ( oldTranslation[i] == oldNumbers[k][matchedYet[k]] )
		matchedYet[k]++;
	    else
		matchedYet[k] = 0;
	}

	/*
	  Let's find out if the last character ended a match. We make
	  two passes over the data. In the first pass, we try to
	  match only numbers that weren't matched yet; if that fails,
	  the second pass does the trick. This is useful in some
	  suspicious cases, flagged below.
	*/
	for ( pass = 0; pass < 2; pass++ ) {
	    best = p; // an impossible value
	    for ( k = 0; k < p; k++ ) {
		if ( (!met[k] || pass > 0) &&
		     matchedYet[k] == (int) oldNumbers[k].length() &&
		     numberLength(oldTranslation.latin1() + (i + 1) -
				  matchedYet[k]) == matchedYet[k] ) {
		    // the longer the better
		    if ( best == p || matchedYet[k] > matchedYet[best] )
			best = k;
		}
	    }
	    if ( best != p ) {
		attempt.truncate( attempt.length() - matchedYet[best] );
		attempt += newNumbers[best];
		met[best] = TRUE;
		for ( k = 0; k < p; k++ )
		    matchedYet[k] = 0;
		break;
	    }
	}
    }

    /*
      We flag two kinds of suspicious cases. They are identified as
      such with comments such as "{2000?}" at the end.

      Example of the first kind: old source text "TeX 3.0" translated
      as "XeT 2.0" is flagged "TeX 2.0 {3.0?}", no matter what the
      new text is.
    */
    for ( k = 0; k < p; k++ ) {
	if ( !met[k] )
	    attempt += TQString( " {" ) + newNumbers[k] + TQString( "?}" );
    }

    /*
      Example of the second kind: "1 of 1" translated as "1 af 1",
      with new source text "1 of 2", generates "1 af 2 {1 or 2?}"
      because it's not clear which of "1 af 2" and "2 af 1" is right.
    */
    for ( k = 0; k < p; k++ ) {
	for ( ell = 0; ell < p; ell++ ) {
	    if ( k != ell && oldNumbers[k] == oldNumbers[ell] &&
		    newNumbers[k] < newNumbers[ell] )
		attempt += TQString( " {" ) + newNumbers[k] + TQString( " or " ) +
			   newNumbers[ell] + TQString( "?}" );
	}
    }
    return attempt;
}

/*
  Augments a MetaTranslator with translations easily derived from
  similar existing (probably obsolete) translations.

  For example, if "TeX 3.0" is translated as "XeT 3.0" and "TeX 3.1"
  has no translation, "XeT 3.1" is added to the translator and is
  marked Unfinished.
*/
void applyNumberHeuristic( MetaTranslator *tor, bool verbose )
{
    TMM translated, untranslated;
    TMM::Iterator t, u;
    TML all = tor->messages();
    TML::Iterator it;
    int inserted = 0;

    for ( it = all.begin(); it != all.end(); ++it ) {
	if ( (*it).type() == MetaTranslatorMessage::Unfinished ) {
	    if ( (*it).translation().isEmpty() )
		untranslated.insert(TQCString((*it).context()) + "\n" + (*it).sourceText() + "\n"
				    + (*it).comment(), *it);
	} else if ( !(*it).translation().isEmpty() ) {
	    translated.insert( zeroKey((*it).sourceText()), *it );
	}
    }

    for ( u = untranslated.begin(); u != untranslated.end(); ++u ) {
	t = translated.find( zeroKey((*u).sourceText()) );
	if ( t != translated.end() && !t.key().isEmpty() &&
	     qstrcmp((*t).sourceText(), (*u).sourceText()) != 0 ) {
	    MetaTranslatorMessage m( *u );
	    m.setTranslation(translationAttempt((*t).translation(), (*t).sourceText(),
						(*u).sourceText()));
	    tor->insert( m );
	    inserted++;
	}
    }
    if ( verbose && inserted != 0 )
	fprintf( stderr, " number heuristic provided %d translation%s\n",
		 inserted, inserted == 1 ? "" : "s" );
}
