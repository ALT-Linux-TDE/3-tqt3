/****************************************************************************
**
** Implementation of TQProcess class for Unix
**
** Created : 20000905
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

#include "qplatformdefs.h"

// Solaris redefines connect -> __xnet_connect with _XOPEN_SOURCE_EXTENDED.
#if defined(connect)
#undef connect
#endif

#include "ntqprocess.h"

#ifndef TQT_NO_PROCESS

#include "ntqapplication.h"
#include "ntqptrqueue.h"
#include "ntqptrlist.h"
#include "ntqsocketnotifier.h"
#include "ntqtimer.h"
#include "ntqcleanuphandler.h"
#include "ntqregexp.h"
#include "private/qinternal_p.h"

#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>

#ifdef __MIPSEL__
# ifndef SOCK_DGRAM
#  define SOCK_DGRAM 1
# endif
# ifndef SOCK_STREAM
#  define SOCK_STREAM 2
# endif
#endif

//#define QT_QPROCESS_DEBUG


#ifdef Q_C_CALLBACKS
extern "C" {
#endif // Q_C_CALLBACKS

    QT_SIGNAL_RETTYPE qt_C_sigchldHnd(QT_SIGNAL_ARGS);

#ifdef Q_C_CALLBACKS
}
#endif // Q_C_CALLBACKS


class TQProc;
class TQProcessManager;
class TQProcessPrivate
{
public:
    TQProcessPrivate();
    ~TQProcessPrivate();

    void closeOpenSocketsForChild();
    void newProc( pid_t pid, TQProcess *process );

    TQMembuf bufStdout;
    TQMembuf bufStderr;

    TQPtrQueue<TQByteArray> stdinBuf;

    TQSocketNotifier *notifierStdin;
    TQSocketNotifier *notifierStdout;
    TQSocketNotifier *notifierStderr;

    ssize_t stdinBufRead;
    TQProc *proc;

    bool exitValuesCalculated;
    bool socketReadCalled;

    static TQProcessManager *procManager;
};


/***********************************************************************
 *
 * TQProc
 *
 **********************************************************************/
/*
  The class TQProcess does not necessarily map exactly to the running
  child processes: if the process is finished, the TQProcess class may still be
  there; furthermore a user can use TQProcess to start more than one process.

  The helper-class TQProc has the semantics that one instance of this class maps
  directly to a running child process.
*/
class TQProc
{
public:
    TQProc( pid_t p, TQProcess *proc=0 ) : pid(p), process(proc)
    {
#if defined(QT_QPROCESS_DEBUG)
	tqDebug( "TQProc: Constructor for pid %d and TQProcess %p", pid, process );
#endif
	socketStdin = 0;
	socketStdout = 0;
	socketStderr = 0;
    }
    ~TQProc()
    {
#if defined(QT_QPROCESS_DEBUG)
	tqDebug( "TQProc: Destructor for pid %d and TQProcess %p", pid, process );
#endif
	if ( process ) {
	    if ( process->d->notifierStdin )
		process->d->notifierStdin->setEnabled( FALSE );
	    if ( process->d->notifierStdout )
		process->d->notifierStdout->setEnabled( FALSE );
	    if ( process->d->notifierStderr )
		process->d->notifierStderr->setEnabled( FALSE );
	    process->d->proc = 0;
	}
	if( socketStdin )
	    ::close( socketStdin );
	if( socketStdout )
	    ::close( socketStdout );
	if( socketStderr )
	    ::close( socketStderr );
    }

    pid_t pid;
    int socketStdin;
    int socketStdout;
    int socketStderr;
    TQProcess *process;
};

/***********************************************************************
 *
 * TQProcessManager
 *
 **********************************************************************/
class TQProcessManager : public TQObject
{
    TQ_OBJECT

public:
    TQProcessManager();
    ~TQProcessManager();

    void append( TQProc *p );
    void remove( TQProc *p );

    void cleanup();

public slots:
    void removeMe();
    void sigchldHnd( int );

public:
    struct sigaction oldactChld;
    struct sigaction oldactPipe;
    TQPtrList<TQProc> *procList;
    int sigchldFd[2];

private:
    TQSocketNotifier *sn;
};

static void qprocess_cleanup()
{
    delete TQProcessPrivate::procManager;
    TQProcessPrivate::procManager = 0;
}

#ifdef Q_OS_QNX6
#define BAILOUT close(tmpSocket);close(socketFD[1]);return -1;
int qnx6SocketPairReplacement (int socketFD[2]) {
    int tmpSocket;
    tmpSocket = socket (AF_INET, SOCK_STREAM, 0);
    if (tmpSocket == -1)
	return -1;
    socketFD[1] = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFD[1] == -1) { BAILOUT };

    sockaddr_in ipAddr;
    memset(&ipAddr, 0, sizeof(ipAddr));
    ipAddr.sin_family = AF_INET;
    ipAddr.sin_addr.s_addr = INADDR_ANY;

    int socketOptions = 1;
    setsockopt(tmpSocket, SOL_SOCKET, SO_REUSEADDR, &socketOptions, sizeof(int));

    bool found = FALSE;
    for (int socketIP = 2000; (socketIP < 2500) && !(found); socketIP++) {
	ipAddr.sin_port = htons(socketIP);
	if (bind(tmpSocket, (struct sockaddr *)&ipAddr, sizeof(ipAddr)))
	    found = TRUE;
    }

    if (listen(tmpSocket, 5)) { BAILOUT };

    // Select non-blocking mode
    int originalFlags = fcntl(socketFD[1], F_GETFL, 0);
    fcntl(socketFD[1], F_SETFL, originalFlags | O_NONBLOCK);

    // Request connection
    if (connect(socketFD[1], (struct sockaddr*)&ipAddr, sizeof(ipAddr)))
	if (errno != EINPROGRESS) { BAILOUT };

    // Accept connection
    socketFD[0] = accept(tmpSocket, (struct sockaddr *)NULL, (size_t *)NULL);
    if(socketFD[0] == -1) { BAILOUT };

    // We're done
    close(tmpSocket);

    // Restore original flags , ie return to blocking
    fcntl(socketFD[1], F_SETFL, originalFlags);
    return 0;
}
#undef BAILOUT
#endif

TQProcessManager::TQProcessManager() : sn(0)
{
    procList = new TQPtrList<TQProc>;
    procList->setAutoDelete( TRUE );

    // The SIGCHLD handler writes to a socket to tell the manager that
    // something happened. This is done to get the processing in sync with the
    // event reporting.
#ifndef Q_OS_QNX6
    if ( ::socketpair( AF_UNIX, SOCK_STREAM, 0, sigchldFd ) ) {
#else
    if ( qnx6SocketPairReplacement (sigchldFd) ) {
#endif
	sigchldFd[0] = 0;
	sigchldFd[1] = 0;
    } else {
#if defined(QT_QPROCESS_DEBUG)
	tqDebug( "TQProcessManager: install socket notifier (%d)", sigchldFd[1] );
#endif
	sn = new TQSocketNotifier( sigchldFd[1],
		TQSocketNotifier::Read, this );
	connect( sn, TQ_SIGNAL(activated(int)),
		this, TQ_SLOT(sigchldHnd(int)) );
	sn->setEnabled( TRUE );
    }

    // install a SIGCHLD handler and ignore SIGPIPE
    struct sigaction act;

#if defined(QT_QPROCESS_DEBUG)
    tqDebug( "TQProcessManager: install a SIGCHLD handler" );
#endif
    act.sa_handler = qt_C_sigchldHnd;
    sigemptyset( &(act.sa_mask) );
    sigaddset( &(act.sa_mask), SIGCHLD );
    act.sa_flags = SA_NOCLDSTOP;
#if defined(SA_RESTART)
    act.sa_flags |= SA_RESTART;
#endif
    if ( sigaction( SIGCHLD, &act, &oldactChld ) != 0 )
	tqWarning( "Error installing SIGCHLD handler" );

#if defined(QT_QPROCESS_DEBUG)
    tqDebug( "TQProcessManager: install a SIGPIPE handler (SIG_IGN)" );
#endif
    act.sa_handler = QT_SIGNAL_IGNORE;
    sigemptyset( &(act.sa_mask) );
    sigaddset( &(act.sa_mask), SIGPIPE );
    act.sa_flags = 0;
    if ( sigaction( SIGPIPE, &act, &oldactPipe ) != 0 )
	tqWarning( "Error installing SIGPIPE handler" );
}

TQProcessManager::~TQProcessManager()
{
    delete procList;

    if ( sigchldFd[0] != 0 )
	::close( sigchldFd[0] );
    if ( sigchldFd[1] != 0 )
	::close( sigchldFd[1] );

    // restore SIGCHLD handler
#if defined(QT_QPROCESS_DEBUG)
    tqDebug( "TQProcessManager: restore old sigchild handler" );
#endif
    if ( sigaction( SIGCHLD, &oldactChld, 0 ) != 0 )
	tqWarning( "Error restoring SIGCHLD handler" );

#if defined(QT_QPROCESS_DEBUG)
    tqDebug( "TQProcessManager: restore old sigpipe handler" );
#endif
    if ( sigaction( SIGPIPE, &oldactPipe, 0 ) != 0 )
	tqWarning( "Error restoring SIGPIPE handler" );
}

void TQProcessManager::append( TQProc *p )
{
    procList->append( p );
#if defined(QT_QPROCESS_DEBUG)
    tqDebug( "TQProcessManager: append process (procList.count(): %d)", procList->count() );
#endif
}

void TQProcessManager::remove( TQProc *p )
{
    procList->remove( p );
#if defined(QT_QPROCESS_DEBUG)
    tqDebug( "TQProcessManager: remove process (procList.count(): %d)", procList->count() );
#endif
    cleanup();
}

void TQProcessManager::cleanup()
{
    if ( procList->count() == 0 ) {
	TQTimer::singleShot( 0, this, TQ_SLOT(removeMe()) );
    }
}

void TQProcessManager::removeMe()
{
    if ( procList->count() == 0 ) {
	tqRemovePostRoutine(qprocess_cleanup);
	TQProcessPrivate::procManager = 0;
	delete this;
    }
}

void TQProcessManager::sigchldHnd( int fd )
{
    // Disable the socket notifier to make sure that this function is not
    // called recursively -- this can happen, if you enter the event loop in
    // the slot connected to the processExited() signal (e.g. by showing a
    // modal dialog) and there are more than one process which exited in the
    // meantime.
    if ( sn ) {
	if ( !sn->isEnabled() )
	    return;
	sn->setEnabled( FALSE );
    }

    char tmp;
    if (::read( fd, &tmp, sizeof(tmp) ) < 0) {
	tqWarning( "Could not read from file descriptor" );
    }
#if defined(QT_QPROCESS_DEBUG)
    tqDebug( "TQProcessManager::sigchldHnd()" );
#endif
    TQProc *proc;
    TQProcess *process;
    bool removeProc;
    proc = procList->first();
    while ( proc != 0 ) {
	removeProc = FALSE;
	process = proc->process;
	if ( process != 0 ) {
	    if ( !process->isRunning() ) {
#if defined(QT_QPROCESS_DEBUG)
		tqDebug( "TQProcessManager::sigchldHnd() (PID: %d): process exited (TQProcess available)", proc->pid );
#endif
		/*
		  Apparently, there is not consistency among different
		  operating systems on how to use FIONREAD.

		  FreeBSD, Linux and Solaris all expect the 3rd
		  argument to ioctl() to be an int, which is normally
		  32-bit even on 64-bit machines.

		  IRIX, on the other hand, expects a size_t, which is
		  64-bit on 64-bit machines.

		  So, the solution is to use size_t initialized to
		  zero to make sure all bits are set to zero,
		  preventing underflow with the FreeBSD/Linux/Solaris
		  ioctls.
		*/
		size_t nbytes = 0;
		// read pending data
		if ( proc->socketStdout && ::ioctl(proc->socketStdout, FIONREAD, (char*)&nbytes)==0 && nbytes>0 ) {
#if defined(QT_QPROCESS_DEBUG)
		    tqDebug( "TQProcessManager::sigchldHnd() (PID: %d): reading %d bytes of pending data on stdout", proc->pid, nbytes );
#endif
		    process->socketRead( proc->socketStdout );
		}
		nbytes = 0;
		if ( proc->socketStderr && ::ioctl(proc->socketStderr, FIONREAD, (char*)&nbytes)==0 && nbytes>0 ) {
#if defined(QT_QPROCESS_DEBUG)
		    tqDebug( "TQProcessManager::sigchldHnd() (PID: %d): reading %d bytes of pending data on stderr", proc->pid, nbytes );
#endif
		    process->socketRead( proc->socketStderr );
		}
		// close filedescriptors if open, and disable the
		// socket notifiers
		if ( proc->socketStdout ) {
		    ::close( proc->socketStdout );
		    proc->socketStdout = 0;
		    if (process->d->notifierStdout)
			process->d->notifierStdout->setEnabled(FALSE);
		}
		if ( proc->socketStderr ) {
		    ::close( proc->socketStderr );
		    proc->socketStderr = 0;
		    if (process->d->notifierStderr)
			process->d->notifierStderr->setEnabled(FALSE);
		}

		if ( process->notifyOnExit )
		    emit process->processExited();

		removeProc = TRUE;
	    }
	} else {
	    int status;
	    if ( ::waitpid( proc->pid, &status, WNOHANG ) == proc->pid ) {
#if defined(QT_QPROCESS_DEBUG)
		tqDebug( "TQProcessManager::sigchldHnd() (PID: %d): process exited (TQProcess not available)", proc->pid );
#endif
		removeProc = TRUE;
	    }
	}
	if ( removeProc ) {
	    TQProc *oldproc = proc;
	    proc = procList->next();
	    remove( oldproc );
	} else {
	    proc = procList->next();
	}
    }
    if ( sn )
	sn->setEnabled( TRUE );
}

#include "qprocess_unix.moc"


/***********************************************************************
 *
 * TQProcessPrivate
 *
 **********************************************************************/
TQProcessManager *TQProcessPrivate::procManager = 0;

TQProcessPrivate::TQProcessPrivate()
{
#if defined(QT_QPROCESS_DEBUG)
    tqDebug( "TQProcessPrivate: Constructor" );
#endif
    stdinBufRead = 0;

    notifierStdin = 0;
    notifierStdout = 0;
    notifierStderr = 0;

    exitValuesCalculated = FALSE;
    socketReadCalled = FALSE;

    proc = 0;
}

TQProcessPrivate::~TQProcessPrivate()
{
#if defined(QT_QPROCESS_DEBUG)
    tqDebug( "TQProcessPrivate: Destructor" );
#endif

    if ( proc != 0 ) {
	if ( proc->socketStdin != 0 ) {
	    ::close( proc->socketStdin );
	    proc->socketStdin = 0;
	}
	proc->process = 0;
    }

    while ( !stdinBuf.isEmpty() ) {
	delete stdinBuf.dequeue();
    }
    delete notifierStdin;
    delete notifierStdout;
    delete notifierStderr;
}

/*
  Closes all open sockets in the child process that are not needed by the child
  process. Otherwise one child may have an open socket on standard input, etc.
  of another child.
*/
void TQProcessPrivate::closeOpenSocketsForChild()
{
    if ( procManager != 0 ) {
	if ( procManager->sigchldFd[0] != 0 )
	    ::close( procManager->sigchldFd[0] );
	if ( procManager->sigchldFd[1] != 0 )
	    ::close( procManager->sigchldFd[1] );

	// close also the sockets from other TQProcess instances
	for ( TQProc *p=procManager->procList->first(); p!=0; p=procManager->procList->next() ) {
	    ::close( p->socketStdin );
	    ::close( p->socketStdout );
	    ::close( p->socketStderr );
	}
    }
}

void TQProcessPrivate::newProc( pid_t pid, TQProcess *process )
{
    proc = new TQProc( pid, process );
    if ( procManager == 0 ) {
	procManager = new TQProcessManager;
	tqAddPostRoutine(qprocess_cleanup);
    }
    // the TQProcessManager takes care of deleting the TQProc instances
    procManager->append( proc );
}

/***********************************************************************
 *
 * sigchld handler callback
 *
 **********************************************************************/
QT_SIGNAL_RETTYPE qt_C_sigchldHnd( QT_SIGNAL_ARGS )
{
    if ( TQProcessPrivate::procManager == 0 )
	return;
    if ( TQProcessPrivate::procManager->sigchldFd[0] == 0 )
	return;

    char a = 1;
    if (::write( TQProcessPrivate::procManager->sigchldFd[0], &a, sizeof(a) ) < 0) {
	tqWarning( "Could not write to file descriptor" );
    }
}


/***********************************************************************
 *
 * TQProcess
 *
 **********************************************************************/
/*
  This private class does basic initialization.
*/
void TQProcess::init()
{
    d = new TQProcessPrivate();
    exitStat = 0;
    exitNormal = FALSE;
}

/*
  This private class resets the process variables, etc. so that it can be used
  for another process to start.
*/
void TQProcess::reset()
{
    delete d;
    d = new TQProcessPrivate();
    exitStat = 0;
    exitNormal = FALSE;
    d->bufStdout.clear();
    d->bufStderr.clear();
}

TQMembuf* TQProcess::membufStdout()
{
    if ( d->proc && d->proc->socketStdout ) {
	/*
	  Apparently, there is not consistency among different
	  operating systems on how to use FIONREAD.

	  FreeBSD, Linux and Solaris all expect the 3rd argument to
	  ioctl() to be an int, which is normally 32-bit even on
	  64-bit machines.

	  IRIX, on the other hand, expects a size_t, which is 64-bit
	  on 64-bit machines.

	  So, the solution is to use size_t initialized to zero to
	  make sure all bits are set to zero, preventing underflow
	  with the FreeBSD/Linux/Solaris ioctls.
	*/
	size_t nbytes = 0;
	if ( ::ioctl(d->proc->socketStdout, FIONREAD, (char*)&nbytes)==0 && nbytes>0 )
	    socketRead( d->proc->socketStdout );
    }
    return &d->bufStdout;
}

TQMembuf* TQProcess::membufStderr()
{
    if ( d->proc && d->proc->socketStderr ) {
	/*
	  Apparently, there is not consistency among different
	  operating systems on how to use FIONREAD.

	  FreeBSD, Linux and Solaris all expect the 3rd argument to
	  ioctl() to be an int, which is normally 32-bit even on
	  64-bit machines.

	  IRIX, on the other hand, expects a size_t, which is 64-bit
	  on 64-bit machines.

	  So, the solution is to use size_t initialized to zero to
	  make sure all bits are set to zero, preventing underflow
	  with the FreeBSD/Linux/Solaris ioctls.
	*/
	size_t nbytes = 0;
	if ( ::ioctl(d->proc->socketStderr, FIONREAD, (char*)&nbytes)==0 && nbytes>0 )
	    socketRead( d->proc->socketStderr );
    }
    return &d->bufStderr;
}

/*!
    Destroys the instance.

    If the process is running, it is <b>not</b> terminated! The
    standard input, standard output and standard error of the process
    are closed.

    You can connect the destroyed() signal to the kill() slot, if you
    want the process to be terminated automatically when the instance
    is destroyed.

    \sa tryTerminate() kill()
*/
TQProcess::~TQProcess()
{
    delete d;
}

/*!
    Tries to run a process for the command and arguments that were
    specified with setArguments(), addArgument() or that were
    specified in the constructor. The command is searched for in the
    path for executable programs; you can also use an absolute path in
    the command itself.

    If \a env is null, then the process is started with the same
    environment as the starting process. If \a env is non-null, then
    the values in the stringlist are interpreted as environment
    setttings of the form \c {key=value} and the process is started in
    these environment settings. For convenience, there is a small
    exception to this rule: under Unix, if \a env does not contain any
    settings for the environment variable \c LD_LIBRARY_PATH, then
    this variable is inherited from the starting process; under
    Windows the same applies for the environment variable \c PATH.

    Returns TRUE if the process could be started; otherwise returns
    FALSE.

    You can write data to the process's standard input with
    writeToStdin(). You can close standard input with closeStdin() and
    you can terminate the process with tryTerminate(), or with kill().

    You can call this function even if you've used this instance to
    create a another process which is still running. In such cases,
    TQProcess closes the old process's standard input and deletes
    pending data, i.e., you lose all control over the old process, but
    the old process is not terminated. This applies also if the
    process could not be started. (On operating systems that have
    zombie processes, TQt will also wait() on the old process.)

    \sa launch() closeStdin()
*/
bool TQProcess::start( TQStringList *env )
{
#if defined(QT_QPROCESS_DEBUG)
    tqDebug( "TQProcess::start()" );
#endif
    reset();

    int sStdin[2];
    int sStdout[2];
    int sStderr[2];

    // open sockets for piping
#ifndef Q_OS_QNX6
    if ( (comms & Stdin) && ::socketpair( AF_UNIX, SOCK_STREAM, 0, sStdin ) == -1 ) {
#else
    if ( (comms & Stdin) && qnx6SocketPairReplacement(sStdin) == -1 ) {
#endif
	return FALSE;
    }
#ifndef Q_OS_QNX6
    if ( (comms & Stderr) && ::socketpair( AF_UNIX, SOCK_STREAM, 0, sStderr ) == -1 ) {
#else
    if ( (comms & Stderr) && qnx6SocketPairReplacement(sStderr) == -1 ) {
#endif
	if ( comms & Stdin ) {
	    ::close( sStdin[0] );
	    ::close( sStdin[1] );
	}
	return FALSE;
    }
#ifndef Q_OS_QNX6
    if ( (comms & Stdout) && ::socketpair( AF_UNIX, SOCK_STREAM, 0, sStdout ) == -1 ) {
#else
    if ( (comms & Stdout) && qnx6SocketPairReplacement(sStdout) == -1 ) {
#endif
	if ( comms & Stdin ) {
	    ::close( sStdin[0] );
	    ::close( sStdin[1] );
	}
	if ( comms & Stderr ) {
	    ::close( sStderr[0] );
	    ::close( sStderr[1] );
	}
	return FALSE;
    }

    // the following pipe is only used to determine if the process could be
    // started
    int fd[2];
    if ( pipe( fd ) < 0 ) {
	// non critical error, go on
	fd[0] = 0;
	fd[1] = 0;
    }

    // construct the arguments for exec
    TQCString *arglistQ = new TQCString[ _arguments.count() + 1 ];
    const char** arglist = new const char*[ _arguments.count() + 1 ];
    int i = 0;
    for ( TQStringList::Iterator it = _arguments.begin(); it != _arguments.end(); ++it ) {
	arglistQ[i] = (*it).local8Bit();
	arglist[i] = arglistQ[i];
#if defined(QT_QPROCESS_DEBUG)
	tqDebug( "TQProcess::start(): arg %d = %s", i, arglist[i] );
#endif
	i++;
    }
#ifdef Q_OS_MACX
    if(i) {
	TQCString arg_bundle = arglistQ[0];
	TQFileInfo fi(arg_bundle);
	if(fi.exists() && fi.isDir() && arg_bundle.right(4) == ".app") {
	    TQCString exe = arg_bundle;
	    int lslash = exe.findRev('/');
	    if(lslash != -1)
		exe = exe.mid(lslash+1);
	    exe = TQCString(arg_bundle + "/Contents/MacOS/" + exe);
	    exe = exe.left(exe.length() - 4); //chop off the .app
	    if(TQFile::exists(exe)) {
		arglistQ[0] = exe;
		arglist[0] = arglistQ[0];
	    }
	}
    }
#endif
    arglist[i] = 0;

    // Must make sure signal handlers are installed before exec'ing
    // in case the process exits quickly.
    if ( d->procManager == 0 ) {
	d->procManager = new TQProcessManager;
	tqAddPostRoutine(qprocess_cleanup);
    }

    // fork and exec
    TQApplication::flushX();
    pid_t pid = fork();
    if ( pid == 0 ) {
	// child
	d->closeOpenSocketsForChild();
	if ( comms & Stdin ) {
	    ::close( sStdin[1] );
	    ::dup2( sStdin[0], STDIN_FILENO );
	}
	if ( comms & Stdout ) {
	    ::close( sStdout[0] );
	    ::dup2( sStdout[1], STDOUT_FILENO );
	}
	if ( comms & Stderr ) {
	    ::close( sStderr[0] );
	    ::dup2( sStderr[1], STDERR_FILENO );
	}
	if ( comms & DupStderr ) {
	    ::dup2( STDOUT_FILENO, STDERR_FILENO );
	}
#ifndef TQT_NO_DIR
	if (::chdir( workingDir.absPath().latin1() ) < 0) {
		tqWarning( "Could not chdir" );
	}
#endif
	if ( fd[0] )
	    ::close( fd[0] );
	if ( fd[1] )
	    ::fcntl( fd[1], F_SETFD, FD_CLOEXEC ); // close on exec shows sucess

	if ( env == 0 ) { // inherit environment and start process
#ifndef Q_OS_QNX4
	    ::execvp( arglist[0], (char*const*)arglist ); // ### cast not nice
#else
	    ::execvp( arglist[0], (char const*const*)arglist ); // ### cast not nice
#endif
	} else { // start process with environment settins as specified in env
	    // construct the environment for exec
	    int numEntries = env->count();
#if defined(Q_OS_MACX)
	    TQString ld_library_path("DYLD_LIBRARY_PATH");
#else
	    TQString ld_library_path("LD_LIBRARY_PATH");
#endif
	    bool setLibraryPath =
		env->grep( TQRegExp( "^" + ld_library_path + "=" ) ).empty() &&
		getenv( ld_library_path ) != 0;
	    if ( setLibraryPath )
		numEntries++;
	    TQCString *envlistQ = new TQCString[ numEntries + 1 ];
	    const char** envlist = new const char*[ numEntries + 1 ];
	    int i = 0;
	    if ( setLibraryPath ) {
		envlistQ[i] = TQString( ld_library_path + "=%1" ).arg( getenv( ld_library_path ) ).local8Bit();
		envlist[i] = envlistQ[i];
		i++;
	    }
	    for ( TQStringList::Iterator it = env->begin(); it != env->end(); ++it ) {
		envlistQ[i] = (*it).local8Bit();
		envlist[i] = envlistQ[i];
		i++;
	    }
	    envlist[i] = 0;

	    // look for the executable in the search path
	    if ( _arguments.count()>0 && getenv("PATH")!=0 ) {
		TQString command = _arguments[0];
		if ( !command.contains( '/' ) ) {
		    TQStringList pathList = TQStringList::split( ':', getenv( "PATH" ) );
		    for (TQStringList::Iterator it = pathList.begin(); it != pathList.end(); ++it ) {
			TQString dir = *it;
#if defined(Q_OS_MACX) //look in a bundle
			if(!TQFile::exists(dir + "/" + command) && TQFile::exists(dir + "/" + command + ".app"))
			    dir += "/" + command + ".app/Contents/MacOS";
#endif
#ifndef TQT_NO_DIR
			TQFileInfo fileInfo( dir, command );
#else
			TQFileInfo fileInfo( dir + "/" + command );
#endif
			if ( fileInfo.isExecutable() ) {
#if defined(Q_OS_MACX)
			    arglistQ[0] = fileInfo.absFilePath().local8Bit();
#else
			    arglistQ[0] = fileInfo.filePath().local8Bit();
#endif
			    arglist[0] = arglistQ[0];
			    break;
			}
		    }
		}
	    }
#ifndef Q_OS_QNX4
	    ::execve( arglist[0], (char*const*)arglist, (char*const*)envlist ); // ### casts not nice
#else
	    ::execve( arglist[0], (char const*const*)arglist,(char const*const*)envlist ); // ### casts not nice
#endif
	}
	if ( fd[1] ) {
	    char buf = 0;
	    if (::write( fd[1], &buf, 1 ) < 0) {
		tqWarning( "Could not write to file descriptor" );
	    }
	    ::close( fd[1] );
	}
	::_exit( -1 );
    } else if ( pid == -1 ) {
	// error forking
	goto error;
    }

    // test if exec was successful
    if ( fd[1] )
	::close( fd[1] );
    if ( fd[0] ) {
	char buf;
	for ( ;; ) {
	    int n = ::read( fd[0], &buf, 1 );
	    if ( n==1 ) {
		// socket was not closed => error
		if ( ::waitpid( pid, 0, WNOHANG ) != pid ) {
		    // The wait did not succeed yet, so try again when we get
		    // the sigchild (to avoid zombies).
		    d->newProc( pid, 0 );
		}
		d->proc = 0;
		goto error;
	    } else if ( n==-1 ) {
		if ( errno==EAGAIN || errno==EINTR )
		    // try it again
		    continue;
	    }
	    break;
	}
	::close( fd[0] );
    }

    d->newProc( pid, this );

    if ( comms & Stdin ) {
	::close( sStdin[0] );
	d->proc->socketStdin = sStdin[1];

	// Select non-blocking mode
	int originalFlags = fcntl(d->proc->socketStdin, F_GETFL, 0);
	fcntl(d->proc->socketStdin, F_SETFL, originalFlags | O_NONBLOCK);

	d->notifierStdin = new TQSocketNotifier( sStdin[1], TQSocketNotifier::Write );
	connect( d->notifierStdin, TQ_SIGNAL(activated(int)),
		this, TQ_SLOT(socketWrite(int)) );
	// setup notifiers for the sockets
	if ( !d->stdinBuf.isEmpty() ) {
	    d->notifierStdin->setEnabled( TRUE );
	}
    }
    if ( comms & Stdout ) {
	::close( sStdout[1] );
	d->proc->socketStdout = sStdout[0];
	d->notifierStdout = new TQSocketNotifier( sStdout[0], TQSocketNotifier::Read );
	connect( d->notifierStdout, TQ_SIGNAL(activated(int)),
		this, TQ_SLOT(socketRead(int)) );
	if ( ioRedirection )
	    d->notifierStdout->setEnabled( TRUE );
    }
    if ( comms & Stderr ) {
	::close( sStderr[1] );
	d->proc->socketStderr = sStderr[0];
	d->notifierStderr = new TQSocketNotifier( sStderr[0], TQSocketNotifier::Read );
	connect( d->notifierStderr, TQ_SIGNAL(activated(int)),
		this, TQ_SLOT(socketRead(int)) );
	if ( ioRedirection )
	    d->notifierStderr->setEnabled( TRUE );
    }

    // cleanup and return
    delete[] arglistQ;
    delete[] arglist;
    return TRUE;

error:
#if defined(QT_QPROCESS_DEBUG)
    tqDebug( "TQProcess::start(): error starting process" );
#endif
    if ( d->procManager )
	d->procManager->cleanup();
    if ( comms & Stdin ) {
	::close( sStdin[1] );
	::close( sStdin[0] );
    }
    if ( comms & Stdout ) {
	::close( sStdout[0] );
	::close( sStdout[1] );
    }
    if ( comms & Stderr ) {
	::close( sStderr[0] );
	::close( sStderr[1] );
    }
    ::close( fd[0] );
    ::close( fd[1] );
    delete[] arglistQ;
    delete[] arglist;
    return FALSE;
}


/*!
    Asks the process to terminate. Processes can ignore this if they
    wish. If you want to be certain that the process really
    terminates, you can use kill() instead.

    The slot returns immediately: it does not wait until the process
    has finished. When the process terminates, the processExited()
    signal is emitted.

    \sa kill() processExited()
*/
void TQProcess::tryTerminate() const
{
    if ( d->proc != 0 )
	::kill( d->proc->pid, SIGTERM );
}

/*!
    Terminates the process. This is not a safe way to end a process
    since the process will not be able to do any cleanup.
    tryTerminate() is safer, but processes can ignore a
    tryTerminate().

    The nice way to end a process and to be sure that it is finished,
    is to do something like this:
    \code
	process->tryTerminate();
	TQTimer::singleShot( 5000, process, TQ_SLOT( kill() ) );
    \endcode

    This tries to terminate the process the nice way. If the process
    is still running after 5 seconds, it terminates the process the
    hard way. The timeout should be chosen depending on the time the
    process needs to do all its cleanup: use a higher value if the
    process is likely to do a lot of computation or I/O on cleanup.

    The slot returns immediately: it does not wait until the process
    has finished. When the process terminates, the processExited()
    signal is emitted.

    \sa tryTerminate() processExited()
*/
void TQProcess::kill() const
{
    if ( d->proc != 0 )
	::kill( d->proc->pid, SIGKILL );
}

/*!
    Returns TRUE if the process is running; otherwise returns FALSE.

    \sa normalExit() exitStatus() processExited()
*/
bool TQProcess::isRunning() const
{
    if ( d->exitValuesCalculated ) {
#if defined(QT_QPROCESS_DEBUG)
	tqDebug( "TQProcess::isRunning(): FALSE (already computed)" );
#endif
	return FALSE;
    }
    if ( d->proc == 0 )
	return FALSE;
    int status;
    if ( ::waitpid( d->proc->pid, &status, WNOHANG ) == d->proc->pid ) {
	// compute the exit values
	TQProcess *that = (TQProcess*)this; // mutable
	that->exitNormal = WIFEXITED( status ) != 0;
	if ( exitNormal ) {
	    that->exitStat = (char)WEXITSTATUS( status );
	}
	d->exitValuesCalculated = TRUE;

	// On heavy processing, the socket notifier for the sigchild might not
	// have found time to fire yet.
	if ( d->procManager && d->procManager->sigchldFd[1] < FD_SETSIZE ) {
	    fd_set fds;
	    struct timeval tv;
	    FD_ZERO( &fds );
	    FD_SET( d->procManager->sigchldFd[1], &fds );
	    tv.tv_sec = 0;
	    tv.tv_usec = 0;
	    if ( ::select( d->procManager->sigchldFd[1]+1, &fds, 0, 0, &tv ) > 0 )
		d->procManager->sigchldHnd( d->procManager->sigchldFd[1] );
	}

#if defined(QT_QPROCESS_DEBUG)
	tqDebug( "TQProcess::isRunning() (PID: %d): FALSE", d->proc->pid );
#endif
	return FALSE;
    }
#if defined(QT_QPROCESS_DEBUG)
    tqDebug( "TQProcess::isRunning() (PID: %d): TRUE", d->proc->pid );
#endif
    return TRUE;
}

/*!
    Returns TRUE if it's possible to read an entire line of text from
    standard output at this time; otherwise returns FALSE.

    \sa readLineStdout() canReadLineStderr()
*/
bool TQProcess::canReadLineStdout() const
{
    if ( !d->proc || !d->proc->socketStdout )
	return d->bufStdout.size() != 0;

    TQProcess *that = (TQProcess*)this;
    return that->membufStdout()->scanNewline( 0 );
}

/*!
    Returns TRUE if it's possible to read an entire line of text from
    standard error at this time; otherwise returns FALSE.

    \sa readLineStderr() canReadLineStdout()
*/
bool TQProcess::canReadLineStderr() const
{
    if ( !d->proc || !d->proc->socketStderr )
	return d->bufStderr.size() != 0;

    TQProcess *that = (TQProcess*)this;
    return that->membufStderr()->scanNewline( 0 );
}

/*!
    Writes the data \a buf to the process's standard input. The
    process may or may not read this data.

    This function always returns immediately. The data you
    pass to writeToStdin() is copied into an internal memory buffer in
    TQProcess, and when control goes back to the event loop, TQProcess will
    starting transferring data from this buffer to the running process.  
    Sometimes the data will be transferred in several payloads, depending on
    how much data is read at a time by the process itself. When TQProcess has
    transferred all the data from its memory buffer to the running process, it
    emits wroteToStdin().
    
    Note that some operating systems use a buffer to transfer
    the data. As a result, wroteToStdin() may be emitted before the
    running process has actually read all the data.

    \sa wroteToStdin() closeStdin() readStdout() readStderr()
*/
void TQProcess::writeToStdin( const TQByteArray& buf )
{
#if defined(QT_QPROCESS_DEBUG)
//    tqDebug( "TQProcess::writeToStdin(): write to stdin (%d)", d->socketStdin );
#endif
    d->stdinBuf.enqueue( new TQByteArray(buf) );
    if ( d->notifierStdin != 0 )
	d->notifierStdin->setEnabled( TRUE );
}


/*!
    Closes the process's standard input.

    This function also deletes any pending data that has not been
    written to standard input.

    \sa wroteToStdin()
*/
void TQProcess::closeStdin()
{
    if ( d->proc == 0 )
	return;
    if ( d->proc->socketStdin !=0 ) {
	while ( !d->stdinBuf.isEmpty() ) {
	    delete d->stdinBuf.dequeue();
	}
	delete d->notifierStdin;
	d->notifierStdin = 0;
	if ( ::close( d->proc->socketStdin ) != 0 ) {
	    tqWarning( "Could not close stdin of child process" );
	}
#if defined(QT_QPROCESS_DEBUG)
	tqDebug( "TQProcess::closeStdin(): stdin (%d) closed", d->proc->socketStdin );
#endif
	d->proc->socketStdin = 0;
    }
}


/*
  This private slot is called when the process has outputted data to either
  standard output or standard error.
*/
void TQProcess::socketRead( int fd )
{
    if ( d->socketReadCalled ) {
	// the slots that are connected to the readyRead...() signals might
	// trigger a recursive call of socketRead(). Avoid this since you get a
	// blocking read otherwise.
	return;
    }

#if defined(QT_QPROCESS_DEBUG)
    tqDebug( "TQProcess::socketRead(): %d", fd );
#endif
    if ( fd == 0 )
	return;
    if ( !d->proc )
	return;
    TQMembuf *buffer = 0;
    int n;
    if ( fd == d->proc->socketStdout ) {
	buffer = &d->bufStdout;
    } else if ( fd == d->proc->socketStderr ) {
	buffer = &d->bufStderr;
    } else {
	// this case should never happen, but just to be safe
	return;
    }
#if defined(QT_QPROCESS_DEBUG)
    uint oldSize = buffer->size();
#endif

    // try to read data first (if it fails, the filedescriptor was closed)
    const int basize = 4096;
    TQByteArray *ba = new TQByteArray( basize );
    n = ::read( fd, ba->data(), basize );
    if ( n > 0 ) {
	ba->resize( n );
	buffer->append( ba );
	ba = 0;
    } else {
	delete ba;
	ba = 0;
    }
    // eof or error?
    if ( n == 0 || n == -1 ) {
	if ( fd == d->proc->socketStdout ) {
#if defined(QT_QPROCESS_DEBUG)
	    tqDebug( "TQProcess::socketRead(): stdout (%d) closed", fd );
#endif
	    d->notifierStdout->setEnabled( FALSE );
	    delete d->notifierStdout;
	    d->notifierStdout = 0;
	    ::close( d->proc->socketStdout );
	    d->proc->socketStdout = 0;
	    return;
	} else if ( fd == d->proc->socketStderr ) {
#if defined(QT_QPROCESS_DEBUG)
	    tqDebug( "TQProcess::socketRead(): stderr (%d) closed", fd );
#endif
	    d->notifierStderr->setEnabled( FALSE );
	    delete d->notifierStderr;
	    d->notifierStderr = 0;
	    ::close( d->proc->socketStderr );
	    d->proc->socketStderr = 0;
	    return;
	}
    }

    if ( fd < FD_SETSIZE ) {
	fd_set fds;
	struct timeval tv;
	FD_ZERO( &fds );
	FD_SET( fd, &fds );
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	while ( ::select( fd+1, &fds, 0, 0, &tv ) > 0 ) {
	    // prepare for the next round
	    FD_ZERO( &fds );
	    FD_SET( fd, &fds );
	    // read data
	    ba = new TQByteArray( basize );
	    n = ::read( fd, ba->data(), basize );
	    if ( n > 0 ) {
		ba->resize( n );
		buffer->append( ba );
		ba = 0;
	    } else {
		delete ba;
		ba = 0;
		break;
	    }
	}
    }

    d->socketReadCalled = TRUE;
    if ( fd == d->proc->socketStdout ) {
#if defined(QT_QPROCESS_DEBUG)
	tqDebug( "TQProcess::socketRead(): %d bytes read from stdout (%d)",
		buffer->size()-oldSize, fd );
#endif
	emit readyReadStdout();
    } else if ( fd == d->proc->socketStderr ) {
#if defined(QT_QPROCESS_DEBUG)
	tqDebug( "TQProcess::socketRead(): %d bytes read from stderr (%d)",
		buffer->size()-oldSize, fd );
#endif
	emit readyReadStderr();
    }
    d->socketReadCalled = FALSE;
}


/*
  This private slot is called when the process tries to read data from standard
  input.
*/
void TQProcess::socketWrite( int fd )
{
    while ( fd == d->proc->socketStdin && d->proc->socketStdin != 0 ) {
	if ( d->stdinBuf.isEmpty() ) {
	    d->notifierStdin->setEnabled( FALSE );
	    return;
	}
	ssize_t ret = ::write( fd,
		d->stdinBuf.head()->data() + d->stdinBufRead,
		d->stdinBuf.head()->size() - d->stdinBufRead );
#if defined(QT_QPROCESS_DEBUG)
	tqDebug( "TQProcess::socketWrite(): wrote %d bytes to stdin (%d)", ret, fd );
#endif
	if ( ret == -1 )
	    return;
	d->stdinBufRead += ret;
	if ( d->stdinBufRead == (ssize_t)d->stdinBuf.head()->size() ) {
	    d->stdinBufRead = 0;
	    delete d->stdinBuf.dequeue();
	    if ( wroteToStdinConnected && d->stdinBuf.isEmpty() )
		emit wroteToStdin();
	}
    }
}

/*!
  \internal
  Flushes standard input. This is useful if you want to use TQProcess in a
  synchronous manner.

  This function should probably go into the public API.
*/
void TQProcess::flushStdin()
{
    if (d->proc)
        socketWrite(d->proc->socketStdin);
}

/*
  This private slot is only used under Windows (but moc does not know about #if
  defined()).
*/
void TQProcess::timeout()
{
}


/*
  This private function is used by connectNotify() and disconnectNotify() to
  change the value of ioRedirection (and related behaviour)
*/
void TQProcess::setIoRedirection( bool value )
{
    ioRedirection = value;
    if ( ioRedirection ) {
	if ( d->notifierStdout )
	    d->notifierStdout->setEnabled( TRUE );
	if ( d->notifierStderr )
	    d->notifierStderr->setEnabled( TRUE );
    } else {
	if ( d->notifierStdout )
	    d->notifierStdout->setEnabled( FALSE );
	if ( d->notifierStderr )
	    d->notifierStderr->setEnabled( FALSE );
    }
}

/*
  This private function is used by connectNotify() and
  disconnectNotify() to change the value of notifyOnExit (and related
  behaviour)
*/
void TQProcess::setNotifyOnExit( bool value )
{
    notifyOnExit = value;
}

/*
  This private function is used by connectNotify() and disconnectNotify() to
  change the value of wroteToStdinConnected (and related behaviour)
*/
void TQProcess::setWroteStdinConnected( bool value )
{
    wroteToStdinConnected = value;
}

/*! \enum TQProcess::PID
  \internal
*/
/*!
    Returns platform dependent information about the process. This can
    be used together with platform specific system calls.

    Under Unix the return value is the PID of the process, or -1 if no
    process belongs to this object.

    Under Windows it is a pointer to the \c PROCESS_INFORMATION
    struct, or 0 if no process is belongs to this object.

    Use of this function's return value is likely to be non-portable.
*/
TQProcess::PID TQProcess::processIdentifier()
{
    if ( d->proc == 0 )
	return -1;
    return d->proc->pid;
}

#endif // TQT_NO_PROCESS
