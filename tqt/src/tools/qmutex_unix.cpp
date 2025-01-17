/****************************************************************************
**
** TQMutex class for Unix
**
** Created : 20010725
**
** Copyright (C) 1992-2008 Trolltech ASA.  All rights reserved.
**
** This file is part of the tools module of the TQt GUI Toolkit.
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

#if defined(TQT_THREAD_SUPPORT)

#include "qplatformdefs.h"

typedef pthread_mutex_t     Q_MUTEX_T;

#if defined(QT_CHECK_RANGE)
#  define Q_NORMAL_MUTEX_TYPE PTHREAD_MUTEX_ERRORCHECK
#else
#  define Q_NORMAL_MUTEX_TYPE PTHREAD_MUTEX_DEFAULT
#endif
#define Q_RECURSIVE_MUTEX_TYPE PTHREAD_MUTEX_RECURSIVE

#include "ntqmutex.h"
#include "qmutex_p.h"

#include <errno.h>
#include <string.h>

// Private class declarations

class TQRealMutexPrivate : public TQMutexPrivate {
public:
    TQRealMutexPrivate(bool = FALSE);

    void lock();
    void unlock();
    bool locked();
    bool trylock();
    int type() const;
    int level();

    bool recursive;
    int count;
};


// Private class implementation

// base destructor
TQMutexPrivate::~TQMutexPrivate()
{
    int ret = pthread_mutex_destroy(&handle);

#ifdef QT_CHECK_RANGE
    if ( ret )
	tqWarning( "Mutex destroy failure: %s", strerror( ret ) );
#endif
}

// real mutex class
TQRealMutexPrivate::TQRealMutexPrivate(bool recurs)
    : recursive(recurs), count(0)
{
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, recursive ? Q_RECURSIVE_MUTEX_TYPE : Q_NORMAL_MUTEX_TYPE);
    int ret = pthread_mutex_init(&handle, &attr);
    pthread_mutexattr_destroy(&attr);

#ifdef QT_CHECK_RANGE
    if( ret )
	tqWarning( "Mutex init failure: %s", strerror( ret ) );
#endif // QT_CHECK_RANGE
}

void TQRealMutexPrivate::lock()
{
    int ret = pthread_mutex_lock(&handle);

    if (!ret) {
	count++;
    } else {
#ifdef QT_CHECK_RANGE
	tqWarning("Mutex lock failure: %s", strerror(ret));
#endif
    }
}

void TQRealMutexPrivate::unlock()
{
    count--;
    int ret = pthread_mutex_unlock(&handle);

    if (ret) {
	count++;
#ifdef QT_CHECK_RANGE
	tqWarning("Mutex unlock failure: %s", strerror(ret));
#endif
    }
}

bool TQRealMutexPrivate::locked()
{
    return count > 0;
}

bool TQRealMutexPrivate::trylock()
{
    int ret = pthread_mutex_trylock(&handle);

    if (ret == EBUSY) {
	return FALSE;
    } else if (ret) {
#ifdef QT_CHECK_RANGE
	tqWarning("Mutex trylock failure: %s", strerror(ret));
#endif
	return FALSE;
    }

    return TRUE;
}

int TQRealMutexPrivate::type() const
{
    return recursive ? Q_MUTEX_RECURSIVE : Q_MUTEX_NORMAL;
}

int TQRealMutexPrivate::level()
{
    return count;
}


/*!
    \class TQMutex ntqmutex.h
    \threadsafe
    \brief The TQMutex class provides access serialization between threads.

    \ingroup thread
    \ingroup environment

    The purpose of a TQMutex is to protect an object, data structure or
    section of code so that only one thread can access it at a time
    (This is similar to the Java \c synchronized keyword). For
    example, say there is a method which prints a message to the user
    on two lines:

    \code
    int number = 6;

    void method1()
    {
	number *= 5;
	number /= 4;
    }

    void method2()
    {
	number *= 3;
	number /= 2;
    }
    \endcode

    If these two methods are called in succession, the following happens:

    \code
    // method1()
    number *= 5;	// number is now 30
    number /= 4;	// number is now 7

    // method2()
    number *= 3;	// nubmer is now 21
    number /= 2;	// number is now 10
    \endcode

    If these two methods are called simultaneously from two threads then the
    following sequence could result:

    \code
    // Thread 1 calls method1()
    number *= 5;	// number is now 30

    // Thread 2 calls method2().
    //
    // Most likely Thread 1 has been put to sleep by the operating
    // system to allow Thread 2 to run.
    number *= 3;	// number is now 90
    number /= 2;	// number is now 45

    // Thread 1 finishes executing.
    number /= 4;	// number is now 11, instead of 10
    \endcode

    If we add a mutex, we should get the result we want:

    \code
    TQMutex mutex;
    int number = 6;

    void method1()
    {
	mutex.lock();
	number *= 5;
	number /= 4;
	mutex.unlock();
    }

    void method2()
    {
	mutex.lock();
	number *= 3;
	number /= 2;
	mutex.unlock();
    }
    \endcode

    Then only one thread can modify \c number at any given time and
    the result is correct. This is a trivial example, of course, but
    applies to any other case where things need to happen in a
    particular sequence.

    When you call lock() in a thread, other threads that try to call
    lock() in the same place will block until the thread that got the
    lock calls unlock(). A non-blocking alternative to lock() is
    tryLock().
*/

/*!
    Constructs a new mutex. The mutex is created in an unlocked state.
    A recursive mutex is created if \a recursive is TRUE; a normal
    mutex is created if \a recursive is FALSE (the default). With a
    recursive mutex, a thread can lock the same mutex multiple times
    and it will not be unlocked until a corresponding number of
    unlock() calls have been made.
*/
TQMutex::TQMutex(bool recursive)
{
	d = new TQRealMutexPrivate(recursive);
}

/*!
    Destroys the mutex.

    \warning If you destroy a mutex that still holds a lock the
    resultant behavior is undefined.
*/
TQMutex::~TQMutex()
{
    delete d;
}

/*!
    Attempt to lock the mutex. If another thread has locked the mutex
    then this call will \e block until that thread has unlocked it.

    \sa unlock(), locked()
*/
void TQMutex::lock()
{
    d->lock();
}

/*!
    Unlocks the mutex. Attempting to unlock a mutex in a different
    thread to the one that locked it results in an error. Unlocking a
    mutex that is not locked results in undefined behaviour (varies
    between different Operating Systems' thread implementations).

    \sa lock(), locked()
*/
void TQMutex::unlock()
{
    d->unlock();
}

/*!
    Returns TRUE if the mutex is locked by another thread; otherwise
    returns FALSE.

    \warning Due to differing implementations of recursive mutexes on
    various platforms, calling this function from the same thread that
    previously locked the mutex will return undefined results.

    \sa lock(), unlock()
*/
bool TQMutex::locked()
{
    return d->locked();
}

/*!
    Attempt to lock the mutex. If the lock was obtained, this function
    returns TRUE. If another thread has locked the mutex, this
    function returns FALSE, instead of waiting for the mutex to become
    available, i.e. it does not block.

    If the lock was obtained, the mutex must be unlocked with unlock()
    before another thread can successfully lock it.

    \sa lock(), unlock(), locked()
*/
bool TQMutex::tryLock()
{
    return d->trylock();
}

/*!
    Returns the current lock level of the mutex.
    0 means the mutex is unlocked
    This method should only be called when the mutex has already been locked
    by lock(), otherwise the lock level could change before the next line
    of code is executed.

    WARNING: Non-recursive mutexes will never exceed a lock level of 1!

    \sa lock(), unlock(), locked()
*/
int TQMutex::level()
{
    return d->level();
}

/*!
    \class TQMutexLocker ntqmutex.h
    \brief The TQMutexLocker class simplifies locking and unlocking TQMutexes.

    \threadsafe

    \ingroup thread
    \ingroup environment

    The purpose of TQMutexLocker is to simplify TQMutex locking and
    unlocking. Locking and unlocking a TQMutex in complex functions and
    statements or in exception handling code is error prone and
    difficult to debug. TQMutexLocker should be used in such situations
    to ensure that the state of the mutex is well defined and always
    locked and unlocked properly.

    TQMutexLocker should be created within a function where a TQMutex
    needs to be locked. The mutex is locked when TQMutexLocker is
    created, and unlocked when TQMutexLocker is destroyed.

    For example, this complex function locks a TQMutex upon entering
    the function and unlocks the mutex at all the exit points:

    \code
    int complexFunction( int flag )
    {
	mutex.lock();

	int return_value = 0;

	switch ( flag ) {
	case 0:
	case 1:
	    {
		mutex.unlock();
		return moreComplexFunction( flag );
	    }

	case 2:
	    {
		int status = anotherFunction();
		if ( status < 0 ) {
		    mutex.unlock();
		    return -2;
		}
		return_value = status + flag;
		break;
	    }

	default:
	    {
		if ( flag > 10 ) {
		    mutex.unlock();
		    return -1;
		}
		break;
	    }
	}

	mutex.unlock();
	return return_value;
    }
    \endcode

    This example function will get more complicated as it is
    developed, which increases the likelihood that errors will occur.

    Using TQMutexLocker greatly simplifies the code, and makes it more
    readable:

    \code
    int complexFunction( int flag )
    {
	TQMutexLocker locker( &mutex );

	int return_value = 0;

	switch ( flag ) {
	case 0:
	case 1:
	    {
		return moreComplexFunction( flag );
	    }

	case 2:
	    {
		int status = anotherFunction();
		if ( status < 0 )
		    return -2;
		return_value = status + flag;
		break;
	    }

	default:
	    {
		if ( flag > 10 )
		    return -1;
		break;
	    }
	}

	return return_value;
    }
    \endcode

    Now, the mutex will always be unlocked when the TQMutexLocker
    object is destroyed (when the function returns since \c locker is
    an auto variable). Note that the mutex will be unlocked after
    the call to moreComplexFunction() in this example, avoiding
    possible bugs caused by unlocking the mutex too early, as in
    the first example.

    The same principle applies to code that throws and catches
    exceptions. An exception that is not caught in the function that
    has locked the mutex has no way of unlocking the mutex before the
    exception is passed up the stack to the calling function.

    TQMutexLocker also provides a mutex() member function that returns
    the mutex on which the TQMutexLocker is operating. This is useful
    for code that needs access to the mutex, such as
    TQWaitCondition::wait(). For example:

    \code
    class SignalWaiter
    {
    private:
	TQMutexLocker locker;

    public:
	SignalWaiter( TQMutex *mutex )
	    : locker( mutex )
	{
	}

	void waitForSignal()
	{
	    ...
	    ...
	    ...

	    while ( ! signalled )
		waitcondition.wait( locker.mutex() );

	    ...
	    ...
	    ...
	}
    };
    \endcode

    \sa TQMutex, TQWaitCondition
*/

/*!
    \fn TQMutexLocker::TQMutexLocker( TQMutex *mutex )

    Constructs a TQMutexLocker and locks \a mutex. The mutex will be
    unlocked when the TQMutexLocker is destroyed. If \a mutex is zero,
    TQMutexLocker does nothing.

    \sa TQMutex::lock()
*/

/*!
    \fn TQMutexLocker::~TQMutexLocker()

    Destroys the TQMutexLocker and unlocks the mutex which was locked
    in the constructor.

    \sa TQMutexLocker::TQMutexLocker(), TQMutex::unlock()
*/

/*!
    \fn TQMutex *TQMutexLocker::mutex() const

    Returns a pointer to the mutex which was locked in the
    constructor.

    \sa TQMutexLocker::TQMutexLocker()
*/

#endif // TQT_THREAD_SUPPORT
