/**********************************************************************
** Copyright (C) 2005-2008 Trolltech ASA.  All rights reserved.
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

#include "uic.h"
#include "domtool.h"
#if defined(UIB)
#include "ui2uib.h"
#endif
#include <ntqapplication.h>
#include <ntqfile.h>
#include <ntqstringlist.h>
#include <ntqdatetime.h>
#include <ntqsettings.h>
#define NO_STATIC_COLORS
#include <globaldefs.h>
#include <stdio.h>
#include <stdlib.h>

// see ### in widgetdatabase.cpp
extern bool dbnounload;
extern TQStringList *dbpaths;

int main( int argc, char * argv[] )
{
    bool impl = FALSE;
    bool subcl = FALSE;
    bool imagecollection = FALSE;
    bool imagecollection_tmpfile = FALSE;
#if defined(UIB)
    bool binary = FALSE;
#endif
    TQStringList images;
    const char *error = 0;
    const char* fileName = 0;
    const char* className = 0;
    const char* headerFile = 0;
    TQCString outputFile;
    TQCString image_tmpfile;
    const char* projectName = 0;
    const char* trmacro = 0;
    bool nofwd = FALSE;
    bool fix = FALSE;
    TQCString pchFile;
    TQApplication app(argc, argv, FALSE);

    TQString keybase( "/TQt Designer/" + 
	TQString::number( (TQT_VERSION >> 16) & 0xff ) +"." + TQString::number( (TQT_VERSION >> 8) & 0xff ) + "/" );
    TQSettings config;
    config.insertSearchPath( TQSettings::Windows, "/Trolltech" );
    TQStringList pluginPaths = config.readListEntry( keybase + "PluginPaths" );
    if (pluginPaths.count())
	TQApplication::setLibraryPaths(pluginPaths);

    for ( int n = 1; n < argc && error == 0; n++ ) {
	TQCString arg = argv[n];
	if ( arg[0] == '-' ) {			// option
	    TQCString opt = &arg[1];
	    if ( opt[0] == 'o' ) {		// output redirection
		if ( opt[1] == '\0' ) {
		    if ( !(n < argc-1) ) {
			error = "Missing output-file name";
			break;
		    }
		    outputFile = argv[++n];
		} else
		    outputFile = &opt[1];
	    } else if ( opt[0] == 'i' || opt == "impl" ) {
		impl = TRUE;
		if ( opt == "impl" || opt[1] == '\0' ) {
		    if ( !(n < argc-1) ) {
			error = "Missing name of header file";
			break;
		    }
		    headerFile = argv[++n];
		} else
		    headerFile = &opt[1];
	    } else if ( opt[0] == 'e' || opt == "embed" ) {
		imagecollection = TRUE;
		if ( opt == "embed" || opt[1] == '\0' ) {
		    if ( !(n < argc-1) ) {
			error = "Missing name of project";
			break;
		    }
		    projectName = argv[++n];
		} else {
		    projectName = &opt[1];
		}
		if ( argc > n+1 && qstrcmp( argv[n+1], "-f" ) == 0 ) {
		    imagecollection_tmpfile = TRUE;
		    image_tmpfile = argv[n+2];
		    n += 2;
		}
#if defined(UIB)

	    } else if ( opt == "binary" ) {
		binary = TRUE;
#endif
	    } else if ( opt == "nofwd" ) {
		nofwd = TRUE;
	    } else if ( opt == "nounload" ) {
		dbnounload = TRUE;
	    } else if ( opt == "subdecl" ) {
		subcl = TRUE;
		if ( !(n < argc-2) ) {
		    error = "Missing arguments";
		    break;
		}
		className = argv[++n];
		headerFile = argv[++n];
	    } else if ( opt == "subimpl" ) {
		subcl = TRUE;
		impl = TRUE;
		if ( !(n < argc-2) ) {
		    error = "Missing arguments";
		    break;
		}
		className = argv[++n];
		headerFile = argv[++n];
	    } else if ( opt == "tr" ) {
		if ( opt == "tr" || opt[1] == '\0' ) {
		    if ( !(n < argc-1) ) {
			error = "Missing tr macro.";
			break;
		    }
		    trmacro = argv[++n];
		} else {
		    trmacro = &opt[1];
		}
	    } else if ( opt == "L" ) {
		if ( !(n < argc-1) ) {
		    error = "Missing plugin path.";
		    break;
		}
		if ( !dbpaths )
		    dbpaths = new TQStringList();
		TQString fn = TQFile::decodeName( argv[++n] );
		dbpaths->append( fn );
		TQApplication::addLibraryPath( fn );
	    } else if ( opt == "version" ) {
		fprintf( stderr,
			 "User Interface Compiler for TQt version %s\n",
			 TQT_VERSION_STR );
		return 1;
	    } else if ( opt == "help" ) {
		break;
	    } else if ( opt == "fix" ) {
		fix = TRUE;
	    } else if ( opt == "pch") {
		if ( !(n < argc-1) ) {
		    error = "Missing name of PCH file";
		    break;
		}
		pchFile = argv[++n];
	    } else {
		error = "Unrecognized option";
	    }
	} else {
	    if ( imagecollection && !imagecollection_tmpfile )
		images << argv[n];
	    else if ( fileName )		// can handle only one file
		error = "Too many input files specified";
	    else
		fileName = argv[n];
	}
    }

    if ( argc < 2 || error || (!fileName && !imagecollection ) ) {
	fprintf( stderr, "TQt user interface compiler.\n" );
	if ( error )
	    fprintf( stderr, "uic: %s\n", error );

	fprintf( stderr, "Usage: %s  [options] [mode] <uifile>\n\n"
		 "Generate declaration:\n"
		 "   %s  [options] <uifile>\n"
		 "Generate implementation:\n"
		 "   %s  [options] -impl <headerfile> <uifile>\n"
		 "\t<headerfile>    name of the declaration file\n"
		 "Generate image collection:\n"
		 "   %s  [options] -embed <project> <image1> <image2> <image3> ...\n"
		 "or\n"
		 "   %s  [options] -embed <project> -f <temporary file containing image names>\n"
		 "\t<project>       project name\n"
		 "\t<image[1-N]>    image files\n"
#if defined(UIB)
		 "Generate binary UI file:\n"
		 "   %s  [options] -binary <uifile>\n"
#endif
		 "Generate subclass declaration:\n"
		 "   %s  [options] -subdecl <subclassname> <baseclassheaderfile> <uifile>\n"
		 "\t<subclassname>     name of the subclass to generate\n"
		 "\t<baseclassheaderfile>    declaration file of the baseclass\n"
		 "Generate subclass implementation:\n"
		 "   %s  [options] -subimpl <subclassname> <subclassheaderfile> <uifile>\n"
		 "\t<subclassname>     name of the subclass to generate\n"
		 "\t<subclassheaderfile>    declaration file of the subclass\n"
		 "Options:\n"
		 "\t-o file         Write output to file rather than stdout\n"
		 "\t-pch file       Add #include \"file\" as the first statement in implementation\n"
		 "\t-nofwd          Omit forward declarations of custom classes\n"
		 "\t-nounload       Don't unload plugins after processing\n"
		 "\t-tr func        Use func() instead of tr() for i18n\n"
		 "\t-L path         Additional plugin search path\n"
		 "\t-version        Display version of uic\n"
		 "\t-help           Display this information\n"
		 , argv[0], argv[0], argv[0], argv[0], argv[0], argv[0], argv[0]
#if defined(UIB)
		 , argv[0]
#endif
	    );
	return 1;
    }

    if ( imagecollection_tmpfile ) {
	TQFile ifile( image_tmpfile );
	if ( ifile.open( IO_ReadOnly ) ) {
	    TQTextStream ts( &ifile );
	    TQString s = ts.read();
	    s = s.simplifyWhiteSpace();
	    images = TQStringList::split( ' ', s );
	    for ( TQStringList::Iterator it = images.begin(); it != images.end(); ++it )
		*it = (*it).simplifyWhiteSpace();
	}
    }

#if defined(UIB)
    if ( binary && outputFile.isEmpty() ) {
	outputFile = fileName;
	if ( outputFile.mid(outputFile.length() - 3).lower() == ".ui" )
	    outputFile.truncate( outputFile.length() - 3 );
	outputFile += ".uib";
    }
#endif

    TQFile fileOut;
    if ( !outputFile.isEmpty() ) {
	fileOut.setName( outputFile );
	if (!fileOut.open( IO_WriteOnly ) ) {
	    tqWarning( "uic: Could not open output file '%s'", outputFile.data() );
	    return 1;
	}
    } else {
	fileOut.open( IO_WriteOnly, stdout );
    }
    TQTextStream out( &fileOut );

    if ( imagecollection ) {
	out.setEncoding( TQTextStream::Latin1 );
	Uic::embed( out, projectName, images );
	return 0;
    }

    out.setEncoding( TQTextStream::UnicodeUTF8 );

    TQFile file( fileName );
    if ( !file.open( IO_ReadOnly ) ) {
	tqWarning( "uic: Could not open file '%s'", fileName );
	return 1;
    }

    TQDomDocument doc;
    TQString errMsg;
    int errLine;
    if ( !doc.setContent( &file, &errMsg, &errLine ) ) {
	tqWarning( TQString("uic: Failed to parse %s: ") + errMsg + TQString (" in line %d"), fileName, errLine );
	return 1;
    }

    TQDomElement e = doc.firstChild().toElement();
    if ( e.hasAttribute("version") && e.attribute("version").toDouble() > 3.3 ) {
	tqWarning( TQString("uic: File generated with too recent version of TQt Designer (%s vs. %s)"),
		  e.attribute("version").latin1(), TQT_VERSION_STR );
	return 1;
    }

    DomTool::fixDocument( doc );

    if ( fix ) {
	out << doc.toString();
	return 0;
#if defined(UIB)
    } else if ( binary ) {
	out.unsetDevice();
	TQDataStream binaryOut( &fileOut );
	convertUiToUib( doc, binaryOut );
	return 0;
#endif
    }

    if ( !subcl ) {
	out << "/****************************************************************************" << endl;
	out << "** Form "<< (impl? "implementation" : "interface") << " generated from reading ui file '" << fileName << "'" << endl;
	out << "**" << endl;
	out << "** Created by: The TQt user interface compiler (TQt " << TQT_VERSION_STR << ")" << endl;
	out << "**" << endl;
	out << "** WARNING! All changes made in this file will be lost!" << endl;
	out << "****************************************************************************/" << endl << endl;
    }

    TQString protector;
    if ( subcl && className && !impl )
	protector = TQString::fromLocal8Bit( className ).upper() + "_H";

    if ( !protector.isEmpty() ) {
	out << "#ifndef " << protector << endl;
	out << "#define " << protector << endl;
    }

    if ( !pchFile.isEmpty() && impl ) {
	out << "#include \"" << pchFile << "\" // PCH include" << endl;
    }

    if ( headerFile ) {
	out << "#include \"" << headerFile << "\"" << endl << endl;
    }

    Uic( fileName, outputFile, out, doc, !impl, subcl, trmacro, className, nofwd );

    if ( !protector.isEmpty() ) {
	out << endl;
	out << "#endif // " << protector << endl;
    }
    if ( fileOut.status() != IO_Ok ) {
	tqWarning( "uic: Error writing to file" );
	if ( !outputFile.isEmpty() )
	    remove( outputFile );
    }
    return 0;
}
