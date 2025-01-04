/****************************************************************************
**
** Definition of movie classes
**
** Created : 970617
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

#ifndef TQMOVIE_H
#define TQMOVIE_H

#ifndef QT_H
#include "ntqpixmap.h" // ### remove or keep for users' convenience?
#endif // QT_H

#ifndef TQT_NO_MOVIE

class TQDataSource;
class TQObject;
class TQMoviePrivate;

class TQ_EXPORT TQMovie {
public:
    TQMovie();
    TQMovie(int bufsize);
    TQMovie(TQDataSource*, int bufsize=1024);
    TQMovie(const TQString &fileName, int bufsize=1024);
    TQMovie(TQByteArray data, int bufsize=1024);
    TQMovie(const TQMovie&);
    ~TQMovie();

    TQMovie& operator=(const TQMovie&);

    int pushSpace() const;
    void pushData(const uchar* data, int length);

    const TQColor& backgroundColor() const;
    void setBackgroundColor(const TQColor&);

    const TQRect& getValidRect() const;
    const TQPixmap& framePixmap() const;
    const TQImage& frameImage() const;

    bool isNull() const;

    int  frameNumber() const;
    int  steps() const;
    bool paused() const;
    bool finished() const;
    bool running() const;

    void unpause();
    void pause();
    void step();
    void step(int);
    void restart();

    int  speed() const;
    void setSpeed(int);

    void connectResize(TQObject* receiver, const char *member);
    void disconnectResize(TQObject* receiver, const char *member=0);

    void connectUpdate(TQObject* receiver, const char *member);
    void disconnectUpdate(TQObject* receiver, const char *member=0);

#ifdef TQ_WS_QWS
    // Temporary hack
    void setDisplayWidget(TQWidget * w);
#endif

    enum Status { SourceEmpty=-2,
		  UnrecognizedFormat=-1,
		  Paused=1,
		  EndOfFrame=2,
		  EndOfLoop=3,
		  EndOfMovie=4,
		  SpeedChanged=5 };
    void connectStatus(TQObject* receiver, const char *member);
    void disconnectStatus(TQObject* receiver, const char *member=0);

private:
    TQMoviePrivate *d;
};

#endif	// TQT_NO_MOVIE

#endif