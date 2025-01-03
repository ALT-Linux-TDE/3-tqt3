/****************************************************************************
**
** Implementation of TQMakeProject class.
**
** Copyright (C) 1992-2008 Trolltech ASA.  All rights reserved.
**
** This file is part of qmake.
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

#include "project.h"
#include "property.h"
#include "option.h"
#include <ntqfile.h>
#include <ntqdir.h>
#include <ntqregexp.h>
#include <ntqtextstream.h>
#include <ntqvaluestack.h>
#ifdef Q_OS_UNIX
# include <unistd.h>
#endif
#include <stdio.h>
#include <stdlib.h>

#ifdef Q_OS_WIN32
#define QT_POPEN _popen
#else
#define QT_POPEN popen
#endif

struct parser_info {
    TQString file;
    int line_no;
} parser;

static void qmake_error_msg(const char *msg)
{
    fprintf(stderr, "%s:%d: %s\n", parser.file.latin1(), parser.line_no, msg);
}

TQStringList qmake_mkspec_paths()
{
    TQStringList ret;
    const TQString concat = TQDir::separator() + TQString("mkspecs");
    if(const char *qmakepath = getenv("QMAKEPATH")) {
#ifdef Q_OS_WIN
	TQStringList lst = TQStringList::split(';', qmakepath);
	for(TQStringList::Iterator it = lst.begin(); it != lst.end(); ++it) {
	    TQStringList lst2 = TQStringList::split(':', (*it));
	    for(TQStringList::Iterator it2 = lst2.begin(); it2 != lst2.end(); ++it2)
		ret << ((*it2) + concat);
	}
#else
	TQStringList lst = TQStringList::split(':', qmakepath);
	for(TQStringList::Iterator it = lst.begin(); it != lst.end(); ++it)
	    ret << ((*it) + concat);
#endif
    }
    if(const char *qtdir = getenv("TQTDIR"))
	ret << (TQString(qtdir) + concat);
#ifdef QT_INSTALL_PREFIX
    ret << (QT_INSTALL_PREFIX + concat);
#endif
#if defined(HAVE_QCONFIG_CPP)
    ret << (tqInstallPath() + concat);
#endif
#ifdef QT_INSTALL_DATA
    ret << (QT_INSTALL_DATA + concat);
#endif
#if defined(HAVE_QCONFIG_CPP)
    ret << (tqInstallPathData() + concat);
#endif

    /* prefer $TQTDIR if it is set */
    if (getenv("TQTDIR"))
	ret << getenv("TQTDIR");
    ret << tqInstallPathData();
    return ret;
}

static TQString varMap(const TQString &x)
{
    TQString ret(x);
    if(ret.startsWith("TMAKE")) //tmake no more!
	ret = "QMAKE" + ret.mid(5);
    else if(ret == "INTERFACES")
	ret = "FORMS";
    else if(ret == "QMAKE_POST_BUILD")
	ret = "QMAKE_POST_LINK";
    else if(ret == "TARGETDEPS")
	ret = "POST_TARGETDEPS";
    else if(ret == "LIBPATH")
	ret = "QMAKE_LIBDIR";
    else if(ret == "QMAKE_EXT_MOC")
	ret = "QMAKE_EXT_CPP_MOC";
    else if(ret == "QMAKE_MOD_MOC")
	ret = "QMAKE_H_MOD_MOC";
    else if(ret == "QMAKE_LFLAGS_SHAPP")
	ret = "QMAKE_LFLAGS_APP";
    else if(ret == "PRECOMPH")
	ret = "PRECOMPILED_HEADER";
    else if(ret == "PRECOMPCPP")
	ret = "PRECOMPILED_SOURCE";
    else if(ret == "INCPATH")
	ret = "INCLUDEPATH";
    return ret;
}

static TQStringList split_arg_list(const TQString &params)
{
    TQChar quote = 0;
    TQStringList args;
    for(int x = 0, last = 0, parens = 0; x <= (int)params.length(); x++) {
	if(x == (int)params.length()) {
	    TQString mid = params.mid(last, x - last).stripWhiteSpace();
            if(quote) {
                if(mid[(int)mid.length()-1] == quote)
                    mid.truncate(1);
                quote = 0;
            }
	    args << mid;
	} else if(params[x] == ')') {
	    parens--;
	} else if(params[x] == '(') {
	    parens++;
	} else if(params[x] == quote) {
	    quote = 0;
	} else if(!quote && (params[x] == '\'' || params[x] == '"')) {
	    quote = params[x];
	} else if(!parens && !quote && params[x] == ',') {
	    args << params.mid(last, x - last).stripWhiteSpace();
	    last = x+1;
	}
    }
    return args;
}

static TQStringList split_value_list(const TQString &vals, bool do_semicolon=FALSE)
{
    TQString build;
    TQStringList ret;
    TQValueStack<TQChar> quote;
    for(int x = 0; x < (int)vals.length(); x++) {
	if(x != (int)vals.length()-1 && vals[x] == '\\' && (vals[x+1] == '\'' || vals[x+1] == '"'))
	    build += vals[x++]; //get that 'escape'
	else if(!quote.isEmpty() && vals[x] == quote.top())
	    quote.pop();
	else if(vals[x] == '\'' || vals[x] == '"')
	    quote.push(vals[x]);

	if(quote.isEmpty() && ((do_semicolon && vals[x] == ';') ||  vals[x] == ' ')) {
	    ret << build;
	    build = "";
	} else {
	    build += vals[x];
	}
    }
    if(!build.isEmpty())
	ret << build;
    return ret;
}

TQMakeProject::TQMakeProject()
{
    prop = NULL;
}

TQMakeProject::TQMakeProject(TQMakeProperty *p)
{
    prop = p;
}

void
TQMakeProject::reset()
{
    /* scope blocks start at true */
    test_status = TestNone;
    scope_flag = 0x01;
    scope_block = 0;
}

bool
TQMakeProject::parse(const TQString &t, TQMap<TQString, TQStringList> &place)
{
    TQString s = t.simplifyWhiteSpace();
    int hash_mark = s.find("#");
    if(hash_mark != -1) //good bye comments
	s = s.left(hash_mark);
    if(s.isEmpty()) /* blank_line */
	return TRUE;

    if(s.stripWhiteSpace().left(1) == "}") {
	debug_msg(1, "Project Parser: %s:%d : Leaving block %d", parser.file.latin1(),
		  parser.line_no, scope_block);
	test_status = ((scope_flag & (0x01 << scope_block)) ? TestFound : TestSeek);
	scope_block--;
	s = s.mid(1).stripWhiteSpace();
	if(s.isEmpty())
	    return TRUE;
    }
    if(!(scope_flag & (0x01 << scope_block))) {
	/* adjust scope for each block which appears on a single line */
	for(int i = (s.contains('{')-s.contains('}')); i>0; i--)
	    scope_flag &= ~(0x01 << (++scope_block));
	debug_msg(1, "Project Parser: %s:%d : Ignored due to block being false.",
		  parser.file.latin1(), parser.line_no);
	return TRUE;
    }

    TQString scope, var, op;
    TQStringList val;
#define SKIP_WS(d) while(*d && (*d == ' ' || *d == '\t')) d++
    const char *d = s.latin1();
    SKIP_WS(d);
    bool scope_failed = FALSE, else_line = FALSE, or_op=FALSE;
    int parens = 0, scope_count=0;
    while(*d) {
	if(!parens) {
	    if(*d == '=')
		break;
	    if(*d == '+' || *d == '-' || *d == '*' || *d == '~') {
		if(*(d+1) == '=') {
		    break;
		} else if(*(d+1) == ' ') {
		    const char *k = d + 1;
		    SKIP_WS(k);
		    if(*k == '=') {
			TQString msg;
			qmake_error_msg(*d + "must be followed immediately by =");
			return FALSE;
		    }
		}
	    }
	}

	if ( *d == '(' )
	    ++parens;
	else if ( *d == ')' )
	    --parens;

	if(!parens && (*d == ':' || *d == '{' || *d == ')' || *d == '|')) {
	    scope_count++;
	    scope = var.stripWhiteSpace();
	    if ( *d == ')' )
		scope += *d; /* need this */
	    var = "";

	    bool test = scope_failed;
	    if(scope.lower() == "else") {
		if(scope_count != 1 || test_status == TestNone) {
		    qmake_error_msg("Unexpected " + scope + " ('" + s + "')");
		    return FALSE;
		}
		else_line = TRUE;
		test = (test_status == TestSeek);
		debug_msg(1, "Project Parser: %s:%d : Else%s %s.", parser.file.latin1(), parser.line_no,
			  scope == "else" ? "" : TQString(" (" + scope + ")").latin1(),
			  test ? "considered" : "excluded");
	    } else {
		TQString comp_scope = scope;
		bool invert_test = (comp_scope.left(1) == "!");
		if(invert_test)
		    comp_scope = comp_scope.right(comp_scope.length()-1);
		int lparen = comp_scope.find('(');
		if(or_op == scope_failed) {
		    if(lparen != -1) { /* if there is an lparen in the scope, it IS a function */
			int rparen = comp_scope.findRev(')');
			if(rparen == -1) {
			    TQCString error;
			    error.sprintf("Function missing right paren: %s ('%s')",
					  comp_scope.latin1(), s.latin1());
			    qmake_error_msg(error);
			    return FALSE;
			}
			TQString func = comp_scope.left(lparen);
			test = doProjectTest(func, comp_scope.mid(lparen+1, rparen - lparen - 1), place);
			if ( *d == ')' && !*(d+1) ) {
			    if(invert_test)
				test = !test;
			    test_status = (test ? TestFound : TestSeek);
			    return TRUE;  /* assume we are done */
			}
		    } else {
			test = isActiveConfig(comp_scope.stripWhiteSpace(), TRUE, &place);
		    }
		    if(invert_test)
			test = !test;
		}
	    }
	    if(!test && !scope_failed)
		debug_msg(1, "Project Parser: %s:%d : Test (%s) failed.", parser.file.latin1(),
			  parser.line_no, scope.latin1());
	    if(test == or_op)
		scope_failed = !test;
	    or_op = (*d == '|');
	    if(*d == '{') { /* scoping block */
		if(!scope_failed)
		    scope_flag |= (0x01 << (++scope_block));
		else
		    scope_flag &= ~(0x01 << (++scope_block));
		debug_msg(1, "Project Parser: %s:%d : Entering block %d (%d).", parser.file.latin1(),
			  parser.line_no, scope_block, !scope_failed);
	    }
	} else if(!parens && *d == '}') {
	    if(!scope_block) {
		warn_msg(WarnParser, "Possible braces mismatch %s:%d", parser.file.latin1(), parser.line_no);
	    } else {
		debug_msg(1, "Project Parser: %s:%d : Leaving block %d", parser.file.latin1(),
			  parser.line_no, scope_block);
		--scope_block;
	    }
	} else {
	    var += *d;
	}
	d++;
    }
    var = var.stripWhiteSpace();
    if(!scope_count || (scope_count == 1 && else_line))
	test_status = TestNone;
    else if(!else_line || test_status != TestFound)
	test_status = (scope_failed ? TestSeek : TestFound);
    if(!*d) {
	if(!var.stripWhiteSpace().isEmpty())
	    qmake_error_msg("Parse Error ('" + s + "')");
	return var.isEmpty(); /* allow just a scope */
    }

    SKIP_WS(d);
    for( ; *d && op.find('=') == -1; op += *(d++))
	;
    op.replace(TQRegExp("\\s"), "");

    SKIP_WS(d);
    TQString vals(d); /* vals now contains the space separated list of values */
    int rbraces = vals.contains('}'), lbraces = vals.contains('{');
    if(scope_block && rbraces - lbraces == 1) {
	debug_msg(1, "Project Parser: %s:%d : Leaving block %d", parser.file.latin1(),
		  parser.line_no, scope_block);
	test_status = ((scope_flag & (0x01 << scope_block)) ? TestFound : TestSeek);
	scope_block--;
	vals.truncate(vals.length()-1);
    } else if(rbraces != lbraces) {
	warn_msg(WarnParser, "Possible braces mismatch {%s} %s:%d",
		 vals.latin1(), parser.file.latin1(), parser.line_no);
    }
    doVariableReplace(vals, place);
    if(scope_failed)
	return TRUE; /* oh well */
#undef SKIP_WS

    if(!var.isEmpty() && Option::mkfile::do_preprocess) {
	static TQString last_file("*none*");
	if(parser.file != last_file) {
	    fprintf(stdout, "#file %s:%d\n", parser.file.latin1(), parser.line_no);
	    last_file = parser.file;
	}
	fprintf(stdout, "%s %s %s\n", var.latin1(), op.latin1(), vals.latin1());
    }
    var = varMap(var); //backwards compatability

    /* vallist is the broken up list of values */
    TQStringList vallist = split_value_list(vals, (var == "DEPENDPATH" || var == "INCLUDEPATH"));
    if(!vallist.grep("=").isEmpty())
	warn_msg(WarnParser, "Detected possible line continuation: {%s} %s:%d",
		 var.latin1(), parser.file.latin1(), parser.line_no);

    TQStringList &varlist = place[var]; /* varlist is the list in the symbol table */
    debug_msg(1, "Project Parser: %s:%d :%s: :%s: (%s)", parser.file.latin1(), parser.line_no,
	var.latin1(), op.latin1(), vallist.isEmpty() ? "" : vallist.join(" :: ").latin1());

    /* now do the operation */
    if(op == "~=") {
	if(vallist.count() != 1) {
	    qmake_error_msg("~= operator only accepts one right hand paramater ('" +
		s + "')");
	    return FALSE;
	}
	TQString val(vallist.first());
	if(val.length() < 4 || val.at(0) != 's') {
	    qmake_error_msg("~= operator only can handle s/// function ('" +
		s + "')");
	    return FALSE;
	}
	TQChar sep = val.at(1);
	TQStringList func = TQStringList::split(sep, val, TRUE);
	if(func.count() < 3 || func.count() > 4) {
	    qmake_error_msg("~= operator only can handle s/// function ('" +
		s + "')");
	    return FALSE;
	}
	bool global = FALSE, case_sense = TRUE;
	if(func.count() == 4) {
	    global = func[3].find('g') != -1;
	    case_sense = func[3].find('i') == -1;
	}
	TQRegExp regexp(func[1], case_sense);
	for(TQStringList::Iterator varit = varlist.begin();
	    varit != varlist.end(); ++varit) {
	    if((*varit).contains(regexp)) {
		(*varit) = (*varit).replace(regexp, func[2]);
		if(!global)
		    break;
	    }
	}
    } else {
	if(op == "=") {
	    if(!varlist.isEmpty())
		warn_msg(WarnParser, "Operator=(%s) clears variables previously set: %s:%d",
			 var.latin1(), parser.file.latin1(), parser.line_no);
	    varlist.clear();
	}
	for(TQStringList::Iterator valit = vallist.begin();
	    valit != vallist.end(); ++valit) {
	    if((*valit).isEmpty())
		continue;
	    if((op == "*=" && !(*varlist.find((*valit)))) ||
	       op == "=" || op == "+=")
		varlist.append((*valit));
	    else if(op == "-=")
		varlist.remove((*valit));
	}
    }
    if(var == "REQUIRES") /* special case to get communicated to backends! */
	doProjectCheckReqs(vallist, place);

    return TRUE;
}

bool
TQMakeProject::read(const TQString &file, TQMap<TQString, TQStringList> &place)
{
    parser_info pi = parser;
    reset();

    TQString filename = Option::fixPathToLocalOS(file);
    doVariableReplace(filename, place);
    bool ret = FALSE, using_stdin = FALSE;
    TQFile qfile;
    if(!strcmp(filename, "-")) {
	qfile.setName("");
	ret = qfile.open(IO_ReadOnly, stdin);
	using_stdin = TRUE;
    } else {
	qfile.setName(filename);
	ret = qfile.open(IO_ReadOnly);
    }
    if ( ret ) {
	TQTextStream t( &qfile );
	TQString s, line;
	parser.file = filename;
	parser.line_no = 0;
	while ( !t.eof() ) {
	    parser.line_no++;
	    line = t.readLine().stripWhiteSpace();
	    int prelen = line.length();

	    int hash_mark = line.find("#");
	    if(hash_mark != -1) //good bye comments
		line = line.left(hash_mark);
	    if(!line.isEmpty() && line.right(1) == "\\") {
		if (!line.startsWith("#")) {
		    line.truncate(line.length() - 1);
		    s += line + " ";
		}
	    } else if(!line.isEmpty() || (line.isEmpty() && !prelen)) {
		if(s.isEmpty() && line.isEmpty())
		    continue;
		if(!line.isEmpty())
		    s += line;
		if(!s.isEmpty()) {
		    if(!(ret = parse(s, place)))
			break;
		    s = "";
		}
	    }
	}
	if(!using_stdin)
	    qfile.close();
    }
    parser = pi;
    if(scope_block != 0)
	warn_msg(WarnParser, "%s: Unterminated conditional at end of file.",
		 file.latin1());
    return ret;
}

bool
TQMakeProject::read(const TQString &project, const TQString &, uchar cmd)
{
    pfile = project;
    return read(cmd);
}

bool
TQMakeProject::read(uchar cmd)
{
    if(cfile.isEmpty()) {
	// hack to get the Option stuff in there
	base_vars["QMAKE_EXT_CPP"] = Option::cpp_ext;
	base_vars["QMAKE_EXT_H"] = Option::h_ext;
	if(!Option::user_template_prefix.isEmpty())
	    base_vars["TEMPLATE_PREFIX"] = Option::user_template_prefix;

	if(cmd & ReadCache && Option::mkfile::do_cache) {	/* parse the cache */
	    if(Option::mkfile::cachefile.isEmpty())  { //find it as it has not been specified
		TQString dir = TQDir::convertSeparators(Option::output_dir);
		while(!TQFile::exists((Option::mkfile::cachefile = dir +
				      TQDir::separator() + ".qmake.cache"))) {
		    dir = dir.left(dir.findRev(TQDir::separator()));
		    if(dir.isEmpty() || dir.find(TQDir::separator()) == -1) {
			Option::mkfile::cachefile = "";
			break;
		    }
		    if(Option::mkfile::cachefile_depth == -1)
			Option::mkfile::cachefile_depth = 1;
		    else
			Option::mkfile::cachefile_depth++;
		}
	    }
	    if(!Option::mkfile::cachefile.isEmpty()) {
		read(Option::mkfile::cachefile, cache);
		if(Option::mkfile::qmakespec.isEmpty() && !cache["QMAKESPEC"].isEmpty())
		    Option::mkfile::qmakespec = cache["QMAKESPEC"].first();
	    }
	}
	if(cmd & ReadConf) { 	    /* parse mkspec */
	    TQStringList mkspec_roots = qmake_mkspec_paths();
	    if(Option::mkfile::qmakespec.isEmpty()) {
		for(TQStringList::Iterator it = mkspec_roots.begin(); it != mkspec_roots.end(); ++it) {
		    TQString mkspec = (*it) + TQDir::separator() + "default";
                    TQFileInfo default_info(mkspec);
                    if(default_info.exists() && default_info.isSymLink()) {
			Option::mkfile::qmakespec = mkspec;
			break;
		    }
		}
		if(Option::mkfile::qmakespec.isEmpty()) {
		    fprintf(stderr, "QMAKESPEC has not been set, so configuration cannot be deduced.\n");
		    return FALSE;
		}
	    }

	    if(TQDir::isRelativePath(Option::mkfile::qmakespec)) {
		bool found_mkspec = FALSE;
		for(TQStringList::Iterator it = mkspec_roots.begin(); it != mkspec_roots.end(); ++it) {
		    TQString mkspec = (*it) + TQDir::separator() + Option::mkfile::qmakespec;
		    if(TQFile::exists(mkspec)) {
			found_mkspec = TRUE;
			Option::mkfile::qmakespec = mkspec;
			break;
		    }
		}
		if(!found_mkspec) {
		    fprintf(stderr, "Could not find mkspecs for your QMAKESPEC after trying:\n\t%s\n",
			    mkspec_roots.join("\n\t").latin1());
		    return FALSE;
		}
	    }

	    /* parse qmake configuration */
	    while(Option::mkfile::qmakespec.endsWith(TQString(TQChar(TQDir::separator()))))
		Option::mkfile::qmakespec.truncate(Option::mkfile::qmakespec.length()-1);
	    TQString spec = Option::mkfile::qmakespec + TQDir::separator() + "qmake.conf";
	    if(!TQFile::exists(spec) &&
	       TQFile::exists(Option::mkfile::qmakespec + TQDir::separator() + "tmake.conf"))
		spec = Option::mkfile::qmakespec + TQDir::separator() + "tmake.conf";
	    debug_msg(1, "QMAKESPEC conf: reading %s", spec.latin1());
	    if(!read(spec, base_vars)) {
		fprintf(stderr, "Failure to read QMAKESPEC conf file %s.\n", spec.latin1());
		return FALSE;
	    }
	    if(Option::mkfile::do_cache && !Option::mkfile::cachefile.isEmpty()) {
		debug_msg(1, "QMAKECACHE file: reading %s", Option::mkfile::cachefile.latin1());
		read(Option::mkfile::cachefile, base_vars);
	    }
	}
	if(cmd & ReadCmdLine) {
	    /* commandline */
	    cfile = pfile;
	    parser.line_no = 1; //really arg count now.. duh
	    parser.file = "(internal)";
	    reset();
	    for(TQStringList::Iterator it = Option::before_user_vars.begin();
		it != Option::before_user_vars.end(); ++it) {
		if(!parse((*it), base_vars)) {
		    fprintf(stderr, "Argument failed to parse: %s\n", (*it).latin1());
		    return FALSE;
		}
		parser.line_no++;
	    }
	}
    }

    vars = base_vars; /* start with the base */

    if(cmd & ReadProFile) { /* parse project file */
	debug_msg(1, "Project file: reading %s", pfile.latin1());
	if(pfile != "-" && !TQFile::exists(pfile) && !pfile.endsWith(".pro"))
	    pfile += ".pro";
	if(!read(pfile, vars))
	    return FALSE;
    }

    if(cmd & ReadPostFiles) { /* parse post files */
	const TQStringList l = vars["QMAKE_POST_INCLUDE_FILES"];
	for(TQStringList::ConstIterator it = l.begin(); it != l.end(); ++it)
	    read((*it), vars);
    }

    if(cmd & ReadCmdLine) {
	parser.line_no = 1; //really arg count now.. duh
	parser.file = "(internal)";
	reset();
	for(TQStringList::Iterator it = Option::after_user_vars.begin();
	    it != Option::after_user_vars.end(); ++it) {
	    if(!parse((*it), vars)) {
		fprintf(stderr, "Argument failed to parse: %s\n", (*it).latin1());
		return FALSE;
	    }
	    parser.line_no++;
	}
    }

    /* now let the user override the template from an option.. */
    if(!Option::user_template.isEmpty()) {
	debug_msg(1, "Overriding TEMPLATE (%s) with: %s", vars["TEMPLATE"].first().latin1(),
		  Option::user_template.latin1());
	vars["TEMPLATE"].clear();
	vars["TEMPLATE"].append(Option::user_template);
    }

    TQStringList &templ = vars["TEMPLATE"];
    if(templ.isEmpty())
	templ.append(TQString("app"));
    else if(vars["TEMPLATE"].first().endsWith(".t"))
	templ = TQStringList(templ.first().left(templ.first().length() - 2));
    if ( !Option::user_template_prefix.isEmpty() )
	templ.first().prepend(Option::user_template_prefix);

    if(vars["TARGET"].isEmpty()) {
	// ### why not simply use:
	// TQFileInfo fi(pfile);
	// fi.baseName();
	TQString tmp = pfile;
	if(tmp.findRev('/') != -1)
	    tmp = tmp.right( tmp.length() - tmp.findRev('/') - 1 );
	if(tmp.findRev('.') != -1)
	    tmp = tmp.left(tmp.findRev('.'));
	vars["TARGET"].append(tmp);
    }

    TQString test_version = getenv("TQTESTVERSION");
    if (!test_version.isEmpty()) {
	TQString s = vars["TARGET"].first();
	if (s == "qt" || s == "tqt-mt" || s == "qte" || s == "qte-mt") {
	    TQString &ver = vars["VERSION"].first();
//	    fprintf(stderr,"Current QT version number: " + ver + "\n");
	    if (ver != "" && ver != test_version) {
		ver = test_version;
		fprintf(stderr,"Changed QT version number to %s!\n", test_version.ascii());
	    }
	}
    }
    return TRUE;
}

bool
TQMakeProject::isActiveConfig(const TQString &x, bool regex, TQMap<TQString, TQStringList> *place)
{
    if(x.isEmpty())
	return TRUE;

    if((Option::target_mode == Option::TARG_MACX_MODE || Option::target_mode == Option::TARG_QNX6_MODE ||
	Option::target_mode == Option::TARG_UNIX_MODE) && x == "unix")
	return TRUE;
    else if(Option::target_mode == Option::TARG_MACX_MODE && x == "macx")
	return TRUE;
    else if(Option::target_mode == Option::TARG_QNX6_MODE && x == "qnx6")
	return TRUE;
    else if(Option::target_mode == Option::TARG_MAC9_MODE && x == "mac9")
	return TRUE;
    else if((Option::target_mode == Option::TARG_MAC9_MODE || Option::target_mode == Option::TARG_MACX_MODE) &&
	    x == "mac")
	return TRUE;
    else if(Option::target_mode == Option::TARG_WIN_MODE && x == "win32")
	return TRUE;


    TQRegExp re(x, FALSE, TRUE);
    TQString spec = Option::mkfile::qmakespec.right(Option::mkfile::qmakespec.length() -
						   (Option::mkfile::qmakespec.findRev(TQDir::separator())+1));
    if((regex && re.exactMatch(spec)) || (!regex && spec == x))
	return TRUE;
#ifdef Q_OS_UNIX
    else if(spec == "default") {
	static char *buffer = NULL;
	if(!buffer)
	    buffer = (char *)malloc(1024);
	int l = readlink(Option::mkfile::qmakespec, buffer, 1023);
	if(l != -1) {
	    buffer[l] = '\0';
	    TQString r = buffer;
	    if(r.findRev('/') != -1)
		r = r.mid(r.findRev('/') + 1);
	    if((regex && re.exactMatch(r)) || (!regex && r == x))
		return TRUE;
	}
    }
#endif


    TQStringList &configs = (place ? (*place)["CONFIG"] : vars["CONFIG"]);
    for(TQStringList::Iterator it = configs.begin(); it != configs.end(); ++it) {
	if((regex && re.exactMatch((*it))) || (!regex && (*it) == x))
	if(re.exactMatch((*it)))
	    return TRUE;
    }
    return FALSE;
}

bool
TQMakeProject::doProjectTest(const TQString& func, const TQString &params, TQMap<TQString, TQStringList> &place)
{
    TQStringList args = split_arg_list(params);
    for(TQStringList::Iterator arit = args.begin(); arit != args.end(); ++arit) {
	TQString tmp = (*arit).stripWhiteSpace();
	if((tmp[0] == '\'' || tmp[0] == '"') && tmp.right(1) == tmp.left(1))
	    tmp = tmp.mid(1, tmp.length() - 2);
    }
    return doProjectTest(func.stripWhiteSpace(), args, place);
}

bool
TQMakeProject::doProjectTest(const TQString& func, TQStringList args, TQMap<TQString, TQStringList> &place)
{
    for(TQStringList::Iterator arit = args.begin(); arit != args.end(); ++arit) {
	(*arit) = (*arit).stripWhiteSpace(); // blah, get rid of space
	doVariableReplace((*arit), place);
    }
    debug_msg(1, "Running project test: %s( %s )", func.latin1(), args.join("::").latin1());

    if(func == "requires") {
	return doProjectCheckReqs(args, place);
    } else if(func == "equals") {
	if(args.count() != 2) {
	    fprintf(stderr, "%s:%d: equals(variable, value) requires two arguments.\n", parser.file.latin1(),
		    parser.line_no);
	    return FALSE;
	}
	TQString value = args[1];
	if((value.left(1) == "\"" || value.left(1) == "'") && value.right(1) == value.left(1))
	    value = value.mid(1, value.length()-2);
	return vars[args[0]].join(" ") == value;
    } else if(func == "exists") {
	if(args.count() != 1) {
	    fprintf(stderr, "%s:%d: exists(file) requires one argument.\n", parser.file.latin1(),
		    parser.line_no);
	    return FALSE;
	}
	TQString file = args.first();
	file = Option::fixPathToLocalOS(file);
	doVariableReplace(file, place);

	if(TQFile::exists(file))
	    return TRUE;
	//regular expression I guess
	TQString dirstr = TQDir::currentDirPath();
	int slsh = file.findRev(Option::dir_sep);
	if(slsh != -1) {
	    dirstr = file.left(slsh+1);
	    file = file.right(file.length() - slsh - 1);
	}
	TQDir dir(dirstr, file);
	return dir.count() != 0;
    } else if(func == "system") {
	if(args.count() != 1) {
	    fprintf(stderr, "%s:%d: system(exec) requires one argument.\n", parser.file.latin1(),
		    parser.line_no);
	    return FALSE;
	}
	return system(args.first().latin1()) == 0;
    } else if(func == "contains") {
	if(args.count() != 2) {
	    fprintf(stderr, "%s:%d: contains(var, val) requires two arguments.\n", parser.file.latin1(),
		    parser.line_no);
	    return FALSE;
	}
	TQRegExp regx(args[1]);
	TQStringList &l = place[args[0]];
	for(TQStringList::ConstIterator it = l.begin(); it != l.end(); ++it) {
	    if(regx.exactMatch((*it)))
		return TRUE;
	}
	return FALSE;
    } else if(func == "infile") {
	if(args.count() < 2 || args.count() > 3) {
	    fprintf(stderr, "%s:%d: infile(file, var, val) requires at least 2 arguments.\n",
		    parser.file.latin1(), parser.line_no);
	    return FALSE;
	}
	TQMakeProject proj;
	TQString file = args[0];
	doVariableReplace(file, place);
	fixEnvVariables(file);
	int di = file.findRev(Option::dir_sep);
	TQDir sunworkshop42workaround = TQDir::current();
	TQString oldpwd = sunworkshop42workaround.currentDirPath();
	if(di != -1) {
	    if(!TQDir::setCurrent(file.left(file.findRev(Option::dir_sep)))) {
		fprintf(stderr, "Cannot find directory: %s\n", file.left(di).latin1());
		return FALSE;
	    }
	    file = file.right(file.length() - di - 1);
	}
	parser_info pi = parser;
	bool ret = !proj.read(file, oldpwd);
	parser = pi;
	if(ret) {
	    fprintf(stderr, "Error processing project file: %s\n", file.latin1());
	    TQDir::setCurrent(oldpwd);
	    return FALSE;
	}
	if(args.count() == 2) {
	    ret = !proj.isEmpty(args[1]);
	} else {
	    TQRegExp regx(args[2]);
	    TQStringList &l = proj.values(args[1]);
	    for(TQStringList::ConstIterator it = l.begin(); it != l.end(); ++it) {
		if(regx.exactMatch((*it))) {
		    ret = TRUE;
		    break;
		}
	    }
	}
	TQDir::setCurrent(oldpwd);
	return ret;
    } else if(func == "count") {
	if(args.count() != 2) {
	    fprintf(stderr, "%s:%d: count(var, count) requires two arguments.\n", parser.file.latin1(),
		    parser.line_no);
	    return FALSE;
	}
	return vars[args[0]].count() == args[1].toUInt();
    } else if(func == "isEmpty") {
	if(args.count() != 1) {
	    fprintf(stderr, "%s:%d: isEmpty(var) requires one argument.\n", parser.file.latin1(),
		    parser.line_no);
	    return FALSE;
	}
	return vars[args[0]].isEmpty();
    } else if(func == "include" || func == "load") {
	TQString seek_var;
	if(args.count() == 2 && func == "include") {
	    seek_var = args[1];
	} else if(args.count() != 1) {
	    TQString func_desc = "include(file)";
	    if(func == "load")
		func_desc = "load(feature)";
	    fprintf(stderr, "%s:%d: %s requires one argument.\n", parser.file.latin1(),
		    parser.line_no, func_desc.latin1());
	    return FALSE;
	}

	TQString file = args.first();
	file = Option::fixPathToLocalOS(file);
	file.replace("\"", "");
	doVariableReplace(file, place);
	if(func == "load") {
	    if(!file.endsWith(Option::prf_ext))
		file += Option::prf_ext;
	    if(file.find(Option::dir_sep) == -1 || !TQFile::exists(file)) {
		bool found = FALSE;
		const TQString concat = TQDir::separator() + TQString("mkspecs") +
				       TQDir::separator() + TQString("features");
		TQStringList feature_roots;
		if(const char *mkspec_path = getenv("QMAKEFEATURES")) {
#ifdef Q_OS_WIN
		    TQStringList lst = TQStringList::split(';', mkspec_path);
		    for(TQStringList::Iterator it = lst.begin(); it != lst.end(); ++it)
			feature_roots += TQStringList::split(':', (*it));
#else
		    feature_roots += TQStringList::split(':', mkspec_path);
#endif
		}
		if(const char *qmakepath = getenv("QMAKEPATH")) {
#ifdef Q_OS_WIN
		    TQStringList lst = TQStringList::split(';', qmakepath);
		    for(TQStringList::Iterator it = lst.begin(); it != lst.end(); ++it) {
			TQStringList lst2 = TQStringList::split(':', (*it));
			for(TQStringList::Iterator it2 = lst2.begin(); it2 != lst2.end(); ++it2)
			    feature_roots << ((*it2) + concat);
		    }
#else
		    TQStringList lst = TQStringList::split(':', qmakepath);
		    for(TQStringList::Iterator it = lst.begin(); it != lst.end(); ++it)
			feature_roots << ((*it) + concat);
#endif
		}
		feature_roots << Option::mkfile::qmakespec;
		if(const char *qtdir = getenv("TQTDIR"))
		    feature_roots << (qtdir + concat);
#ifdef QT_INSTALL_PREFIX
		feature_roots << (QT_INSTALL_PREFIX + concat);
#endif
#if defined(HAVE_QCONFIG_CPP)
		feature_roots << (tqInstallPath() + concat);
#endif
#ifdef QT_INSTALL_DATA
		feature_roots << (QT_INSTALL_DATA + concat);
#endif
#if defined(HAVE_QCONFIG_CPP)
		feature_roots << (tqInstallPathData() + concat);
#endif
		for(TQStringList::Iterator it = feature_roots.begin(); it != feature_roots.end(); ++it) {
		    TQString prf = (*it) + TQDir::separator() + file;
		    if(TQFile::exists(prf)) {
			found = TRUE;
			file = prf;
			break;
		    }
		}
		if(!found) {
		    printf("Project LOAD(): Feature %s cannot be found.\n", args.first().latin1());
		    exit(3);
		}
	    }
	}
	if(TQDir::isRelativePath(file)) {
	    TQStringList include_roots;
	    include_roots << Option::output_dir;
	    TQString pfilewd = TQFileInfo(parser.file).dirPath();
	    if(!pfilewd.isEmpty() && pfilewd != TQDir::currentDirPath() && pfilewd != ".")
		include_roots << pfilewd;
	    if(Option::output_dir != TQDir::currentDirPath())
		include_roots << TQDir::currentDirPath();
	    for(TQStringList::Iterator it = include_roots.begin(); it != include_roots.end(); ++it) {
		if(TQFile::exists((*it) + TQDir::separator() + file)) {
		    file = (*it) + TQDir::separator() + file;
		    break;
		}
	    }
	}

	if(Option::mkfile::do_preprocess) //nice to see this first..
	    fprintf(stderr, "#switching file %s(%s) - %s:%d\n", func.latin1(), file.latin1(),
		    parser.file.latin1(), parser.line_no);
	debug_msg(1, "Project Parser: %s'ing file %s.", func.latin1(), file.latin1());
	TQString orig_file = file;
	int di = file.findRev(Option::dir_sep);
	TQDir sunworkshop42workaround = TQDir::current();
	TQString oldpwd = sunworkshop42workaround.currentDirPath();
	if(di != -1) {
	    if(!TQDir::setCurrent(file.left(file.findRev(Option::dir_sep)))) {
		fprintf(stderr, "Cannot find directory: %s\n", file.left(di).latin1());
		return FALSE;
	    }
	    file = file.right(file.length() - di - 1);
	}
	parser_info pi = parser;
	int sb = scope_block;
	int sf = scope_flag;
	TestStatus sc = test_status;
	bool r = FALSE;
	if(!seek_var.isNull()) {
	    TQMap<TQString, TQStringList> tmp;
	    if((r = read(file.latin1(), tmp)))
		place[seek_var] += tmp[seek_var];
	} else {
	    r = read(file.latin1(), place);
	}
	if(r)
	    vars["QMAKE_INTERNAL_INCLUDED_FILES"].append(orig_file);
	else
	    warn_msg(WarnParser, "%s:%d: Failure to include file %s.",
		     pi.file.latin1(), pi.line_no, orig_file.latin1());
	parser = pi;
	test_status = sc;
	scope_flag = sf;
	scope_block = sb;
	TQDir::setCurrent(oldpwd);
	return r;
    } else if(func == "error" || func == "message") {
	if(args.count() != 1) {
	    fprintf(stderr, "%s:%d: %s(message) requires one argument.\n", parser.file.latin1(),
		    parser.line_no, func.latin1());
	    return FALSE;
	}
	TQString msg = args.first();
	if((msg.startsWith("\"") || msg.startsWith("'")) && msg.endsWith(msg.left(1)))
	    msg = msg.mid(1, msg.length()-2);
	msg.replace(TQString("${QMAKE_FILE}"), parser.file.latin1());
	msg.replace(TQString("${QMAKE_LINE_NUMBER}"), TQString::number(parser.line_no));
	msg.replace(TQString("${QMAKE_DATE}"), TQDateTime::currentDateTime().toString());
	doVariableReplace(msg, place);
	fixEnvVariables(msg);
	fprintf(stderr, "Project %s: %s\n", func.upper().latin1(), msg.latin1());
	if(func == "message")
	    return TRUE;
	exit(2);
    } else {
	fprintf(stderr, "%s:%d: Unknown test function: %s\n", parser.file.latin1(), parser.line_no,
		func.latin1());
    }
    return FALSE;
}

bool
TQMakeProject::doProjectCheckReqs(const TQStringList &deps, TQMap<TQString, TQStringList> &place)
{
    bool ret = FALSE;
    for(TQStringList::ConstIterator it = deps.begin(); it != deps.end(); ++it) {
	TQString chk = (*it);
	if(chk.isEmpty())
	    continue;
	bool invert_test = (chk.left(1) == "!");
	if(invert_test)
	    chk = chk.right(chk.length() - 1);

	bool test;
	int lparen = chk.find('(');
	if(lparen != -1) { /* if there is an lparen in the chk, it IS a function */
	    int rparen = chk.findRev(')');
	    if(rparen == -1) {
		TQCString error;
		error.sprintf("Function (in REQUIRES) missing right paren: %s", chk.latin1());
		qmake_error_msg(error);
	    } else {
		TQString func = chk.left(lparen);
		test = doProjectTest(func, chk.mid(lparen+1, rparen - lparen - 1), place);
	    }
	} else {
	    test = isActiveConfig(chk, TRUE, &place);
	}
	if(invert_test) {
	    chk.prepend("!");
	    test = !test;
	}
	if(!test) {
	    debug_msg(1, "Project Parser: %s:%d Failed test: REQUIRES = %s",
		      parser.file.latin1(), parser.line_no, chk.latin1());
	    place["QMAKE_FAILED_REQUIREMENTS"].append(chk);
	    ret = FALSE;
	}
    }
    return ret;
}


TQString
TQMakeProject::doVariableReplace(TQString &str, const TQMap<TQString, TQStringList> &place)
{
    for(int var_begin, var_last=0; (var_begin = str.find("$$", var_last)) != -1; var_last = var_begin) {
	if(var_begin >= (int)str.length() + 2) {
	    break;
	} else if(var_begin != 0 && str[var_begin-1] == '\\') {
	    str.replace(var_begin-1, 1, "");
	    var_begin += 1;
	    continue;
	}

	int var_incr = var_begin + 2;
	bool in_braces = FALSE, as_env = FALSE, as_prop = FALSE;
	if(str[var_incr] == '{') {
	    in_braces = TRUE;
	    var_incr++;
	    while(var_incr < (int)str.length() &&
		  (str[var_incr] == ' ' || str[var_incr] == '\t' || str[var_incr] == '\n'))
		var_incr++;
	}
	if(str[var_incr] == '(') {
	    as_env = TRUE;
	    var_incr++;
	} else if(str[var_incr] == '[') {
	    as_prop = TRUE;
	    var_incr++;
	}
	TQString val, args;
	while(var_incr < (int)str.length() &&
	      (str[var_incr].isLetter() || str[var_incr].isNumber() || str[var_incr] == '.' || str[var_incr] == '_'))
	    val += str[var_incr++];
	if(as_env) {
	    if(str[var_incr] != ')') {
		var_incr++;
		warn_msg(WarnParser, "%s:%d: Unterminated env-variable replacement '%s' (%s)",
			 parser.file.latin1(), parser.line_no,
			 str.mid(var_begin, TQMAX(var_incr - var_begin,
						 (int)str.length())).latin1(), str.latin1());
		var_begin += var_incr;
		continue;
	    }
	    var_incr++;
	} else if(as_prop) {
	    if(str[var_incr] != ']') {
		var_incr++;
		warn_msg(WarnParser, "%s:%d: Unterminated prop-variable replacement '%s' (%s)",
			 parser.file.latin1(), parser.line_no,
			 str.mid(var_begin, TQMAX(var_incr - var_begin, int(str.length()))).latin1(), str.latin1());
		var_begin += var_incr;
		continue;
	    }
	    var_incr++;
	} else if(str[var_incr] == '(') { //args
	    for(int parens = 0; var_incr < (int)str.length(); var_incr++) {
		if(str[var_incr] == '(') {
		    parens++;
		    if(parens == 1)
			continue;
		} else if(str[var_incr] == ')') {
		    parens--;
		    if(!parens) {
			var_incr++;
			break;
		    }
		}
		args += str[var_incr];
	    }
	}
	if(var_incr > (int)str.length() || (in_braces && str[var_incr] != '}')) {
	    var_incr++;
	    warn_msg(WarnParser, "%s:%d: Unterminated variable replacement '%s' (%s)",
		     parser.file.latin1(), parser.line_no,
		     str.mid(var_begin, TQMAX(var_incr - var_begin,
					     (int)str.length())).latin1(), str.latin1());
	    var_begin += var_incr;
	    continue;
	} else if(in_braces) {
	    var_incr++;
	}

	TQString replacement;
	if(as_env) {
	    replacement = getenv(val);
	} else if(as_prop) {
	    if(prop)
		replacement = prop->value(val);
	} else if(args.isEmpty()) {
	    if(val.left(1) == ".")
		replacement = "";
	    else if(val == "LITERAL_WHITESPACE")
		replacement = "\t";
	    else if(val == "LITERAL_DOLLAR")
		replacement = "$";
	    else if(val == "LITERAL_HASH")
		replacement = "#";
	    else if(val == "PWD")
		replacement = TQDir::currentDirPath();
	    else if(val == "DIR_SEPARATOR")
		replacement = Option::dir_sep;
	    else
		replacement = place[varMap(val)].join(" ");
	} else {
	    TQStringList arg_list = split_arg_list(doVariableReplace(args, place));
	    debug_msg(1, "Running function: %s( %s )", val.latin1(), arg_list.join("::").latin1());
	    if(val.lower() == "member") {
		if(arg_list.count() < 1 || arg_list.count() > 3) {
		    fprintf(stderr, "%s:%d: member(var, start, end) requires three arguments.\n",
			    parser.file.latin1(), parser.line_no);
		} else {
		    const TQStringList &var = place[varMap(arg_list.first())];
		    int start = 0;
		    if(arg_list.count() >= 2)
			start = arg_list[1].toInt();
		    if(start < 0)
			start += int(var.count());
		    int end = start;
		    if(arg_list.count() == 3)
			end = arg_list[2].toInt();
		    if(end < 0)
			end += int(var.count());
		    if(end < start)
			end = start;
		    for(int i = start; i <= end && (int)var.count() >= i; i++) {
			if(!replacement.isEmpty())
			    replacement += " ";
			replacement += var[i];
		    }
		}
	    } else if(val.lower() == "fromfile") {
		if(arg_list.count() != 2) {
		    fprintf(stderr, "%s:%d: fromfile(file, variable) requires two arguments.\n",
			    parser.file.latin1(), parser.line_no);
		} else {
		    TQString file = arg_list[0];
		    file = Option::fixPathToLocalOS(file);
		    file.replace("\"", "");

		    if(TQDir::isRelativePath(file)) {
			TQStringList include_roots;
			include_roots << Option::output_dir;
			TQString pfilewd = TQFileInfo(parser.file).dirPath();
			if(pfilewd.isEmpty())
			    include_roots << pfilewd;
			if(Option::output_dir != TQDir::currentDirPath())
			    include_roots << TQDir::currentDirPath();
			for(TQStringList::Iterator it = include_roots.begin(); it != include_roots.end(); ++it) {
			    if(TQFile::exists((*it) + TQDir::separator() + file)) {
				file = (*it) + TQDir::separator() + file;
				break;
			    }
			}
		    }
		    TQString orig_file = file;
		    int di = file.findRev(Option::dir_sep);
		    TQDir sunworkshop42workaround = TQDir::current();
		    TQString oldpwd = sunworkshop42workaround.currentDirPath();
		    if(di != -1 && TQDir::setCurrent(file.left(file.findRev(Option::dir_sep))))
			file = file.right(file.length() - di - 1);
		    parser_info pi = parser;
		    int sb = scope_block;
		    int sf = scope_flag;
		    TestStatus sc = test_status;
		    TQMap<TQString, TQStringList> tmp;
		    bool r = read(file.latin1(), tmp);
		    if(r) {
			replacement = tmp[arg_list[1]].join(" ");
			vars["QMAKE_INTERNAL_INCLUDED_FILES"].append(orig_file);
		    } else {
			warn_msg(WarnParser, "%s:%d: Failure to include file %s.",
				 pi.file.latin1(), pi.line_no, orig_file.latin1());
		    }
		    parser = pi;
		    test_status = sc;
		    scope_flag = sf;
		    scope_block = sb;
		    TQDir::setCurrent(oldpwd);
		}
	    } else if(val.lower() == "eval") {
		for(TQStringList::ConstIterator arg_it = arg_list.begin();
		    arg_it != arg_list.end(); ++arg_it) {
		    if(!replacement.isEmpty())
			replacement += " ";
		    replacement += place[(*arg_it)].join(" ");
		}
	    } else if(val.lower() == "list") {
		static int x = 0;
		replacement.sprintf(".QMAKE_INTERNAL_TMP_VAR_%d", x++);
		TQStringList &lst = (*((TQMap<TQString, TQStringList>*)&place))[replacement];
		lst.clear();
		for(TQStringList::ConstIterator arg_it = arg_list.begin();
		    arg_it != arg_list.end(); ++arg_it)
		    lst += split_value_list((*arg_it));
	    } else if(val.lower() == "join") {
		if(arg_list.count() < 1 || arg_list.count() > 4) {
		    fprintf(stderr, "%s:%d: join(var, glue, before, after) requires four"
			    "arguments.\n", parser.file.latin1(), parser.line_no);
		} else {
		    TQString glue, before, after;
		    if(arg_list.count() >= 2)
			glue = arg_list[1].replace("\"", "" );
		    if(arg_list.count() >= 3)
			before = arg_list[2].replace("\"", "" );
		    if(arg_list.count() == 4)
			after = arg_list[3].replace("\"", "" );
		    const TQStringList &var = place[varMap(arg_list.first())];
		    if(!var.isEmpty())
			replacement = before + var.join(glue) + after;
		}
	    } else if(val.lower() == "split") {
		if(arg_list.count() < 2 || arg_list.count() > 3) {
		    fprintf(stderr, "%s:%d split(var, sep, join) requires three arguments\n",
			    parser.file.latin1(), parser.line_no);
		} else {
		    TQString sep = arg_list[1], join = " ";
		    if(arg_list.count() == 3)
			join = arg_list[2];
		    TQStringList var = place[varMap(arg_list.first())];
		    for(TQStringList::Iterator vit = var.begin(); vit != var.end(); ++vit) {
			TQStringList lst = TQStringList::split(sep, (*vit));
			for(TQStringList::Iterator spltit = lst.begin(); spltit != lst.end(); ++spltit) {
			    if(!replacement.isEmpty())
				replacement += join;
			    replacement += (*spltit);
			}
		    }
		}
	    } else if(val.lower() == "find") {
		if(arg_list.count() != 2) {
		    fprintf(stderr, "%s:%d find(var, str) requires two arguments\n",
			    parser.file.latin1(), parser.line_no);
		} else {
		    TQRegExp regx(arg_list[1]);
		    const TQStringList &var = place[varMap(arg_list.first())];
		    for(TQStringList::ConstIterator vit = var.begin();
			vit != var.end(); ++vit) {
			if(regx.search(*vit) != -1) {
			    if(!replacement.isEmpty())
				replacement += " ";
			    replacement += (*vit);
			}
		    }
		}
	    } else if(val.lower() == "system") {
		if(arg_list.count() != 1) {
		    fprintf(stderr, "%s:%d system(execut) requires one argument\n",
			    parser.file.latin1(), parser.line_no);
		} else {
		    char buff[256];
		    FILE *proc = QT_POPEN(arg_list.join(" ").latin1(), "r");
		    while(proc && !feof(proc)) {
			int read_in = int(fread(buff, 1, 255, proc));
			if(!read_in)
			    break;
			for(int i = 0; i < read_in; i++) {
			    if(buff[i] == '\n' || buff[i] == '\t')
				buff[i] = ' ';
			}
			buff[read_in] = '\0';
			replacement += buff;
		    }
		}
	    } else if(val.lower() == "files") {
		if(arg_list.count() != 1) {
		    fprintf(stderr, "%s:%d files(pattern) requires one argument\n",
			    parser.file.latin1(), parser.line_no);
		} else {
		    TQString dir, regex = arg_list[0];
		    regex = Option::fixPathToLocalOS(regex);
		    regex.replace("\"", "");
		    if(regex.findRev(TQDir::separator()) != -1) {
			dir = regex.left(regex.findRev(TQDir::separator()) + 1);
			regex = regex.right(regex.length() - dir.length());
		    }
		    TQDir d(dir, regex);
		    for(int i = 0; i < (int)d.count(); i++) {
			if(!replacement.isEmpty())
			    replacement += " ";
			replacement += dir + d[i];
		    }
		}
	    } else if(val.lower() == "prompt") {
		if(arg_list.count() != 1) {
		    fprintf(stderr, "%s:%d prompt(question) requires one argument\n",
			    parser.file.latin1(), parser.line_no);
		} else if(projectFile() == "-") {
		    fprintf(stderr, "%s:%d prompt(question) cannot be used when '-o -' is used.\n",
			    parser.file.latin1(), parser.line_no);
		} else {
		    TQString msg = arg_list.first();
		    if((msg.startsWith("\"") || msg.startsWith("'")) && msg.endsWith(msg.left(1)))
			msg = msg.mid(1, msg.length()-2);
		    msg.replace(TQString("${QMAKE_FILE}"), parser.file.latin1());
		    msg.replace(TQString("${QMAKE_LINE_NUMBER}"), TQString::number(parser.line_no));
		    msg.replace(TQString("${QMAKE_DATE}"), TQDateTime::currentDateTime().toString());
		    doVariableReplace(msg, place);
		    fixEnvVariables(msg);
		    if(!msg.endsWith("?"))
			msg += "?";
		    fprintf(stderr, "Project %s: %s ", val.upper().latin1(), msg.latin1());

		    TQFile qfile;
		    if(qfile.open(IO_ReadOnly, stdin)) {
			TQTextStream t(&qfile);
			replacement = t.readLine();
		    }
		}
	    } else {
		fprintf(stderr, "%s:%d: Unknown replace function: %s\n",
			parser.file.latin1(), parser.line_no, val.latin1());
	    }
	}
	//actually do replacement now..
	int mlen = var_incr - var_begin;
	debug_msg(2, "Project Parser [var replace]: '%s' :: %s -> %s", str.latin1(),
		  str.mid(var_begin, mlen).latin1(), replacement.latin1());
	str.replace(var_begin, mlen, replacement);
	var_begin += replacement.length();
    }
    return str;
}
