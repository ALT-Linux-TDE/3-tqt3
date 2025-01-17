/****************************************************************************
**
** Definition of TQt extension classes for Netscape Plugin support.
**
** Created : 970601
**
** Copyright (C) 1992-2008 Trolltech ASA.  All rights reserved.
**
** This file is part of the TQt GUI Toolkit.
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

#ifndef TQNP_H
#define TQNP_H

#ifndef QT_H
#include "ntqwidget.h"
#endif // QT_H


struct _NPInstance;
struct _NPStream;
class TQNPInstance;

class TQNPStream {
public:
    ~TQNPStream();

    const char* url() const;
    uint end() const;
    uint lastModified() const;

    const char* type() const;
    bool seekable() const;
    bool okay() const;
    bool complete() const;

    void requestRead(int offset, uint length);
    int write( int len, void* buffer );

    TQNPInstance* instance() { return inst; }
    TQNPStream(TQNPInstance*,const char*,_NPStream*,bool);
    void setOkay(bool);
    void setComplete(bool);

private:
    TQNPInstance* inst;
    _NPStream* stream;
    TQString mtype;
    int seek:1;
    int isokay:1;
    int iscomplete:1;
};

class TQNPWidget : public TQWidget {
    TQ_OBJECT
public:
    TQNPWidget();
    ~TQNPWidget();
    void enterEvent(TQEvent*);
    void leaveEvent(TQEvent*);

    virtual void enterInstance();
    virtual void leaveInstance();

    TQNPInstance* instance();

private:
    _NPInstance* pi;
};

class TQNPInstance : public TQObject {
    TQ_OBJECT
public:
    ~TQNPInstance();

    // Arguments passed to EMBED
    int argc() const;
    const char* argn(int) const;
    const char* argv(int) const;
    enum Reason {
        ReasonDone = 0,
        ReasonBreak = 1,
        ReasonError = 2,
        ReasonUnknown = -1
    };
    const char* arg(const char* name) const;
    enum InstanceMode { Embed=1, Full=2, Background=3 };
    InstanceMode mode() const;

    // The browser's name
    const char* userAgent() const;

    // Your window.
    virtual TQNPWidget* newWindow();
    TQNPWidget* widget();

    // Incoming streams (SRC=... tag).
    // Defaults ignore data.
    enum StreamMode { Normal=1, Seek=2, AsFile=3, AsFileOnly=4 };
    virtual bool newStreamCreated(TQNPStream*, StreamMode& smode);
    virtual int writeReady(TQNPStream*);
    virtual int write(TQNPStream*, int offset, int len, void* buffer);
    virtual void streamDestroyed(TQNPStream*);

    void status(const char* msg);
    void getURLNotify(const char* url, const char* window=0, void*data=0);

    void getURL(const char* url, const char* window=0);
    void postURL(const char* url, const char* window,
	     uint len, const char* buf, bool file);

    TQNPStream* newStream(const char* mimetype, const char* window,
	bool as_file=FALSE);
    virtual void streamAsFile(TQNPStream*, const char* fname);

    void* getJavaPeer() const;

    virtual void notifyURL(const char* url, Reason r, void* notifyData);
    virtual bool printFullPage();
    virtual void print(TQPainter*);

protected:
    TQNPInstance();

private:
    friend class TQNPStream;
    _NPInstance* pi;
};


class TQNPlugin {
public:
    // Write this to return your TQNPlugin derived class.
    static TQNPlugin* create();

    static TQNPlugin* actual();

    virtual ~TQNPlugin();

    void getVersionInfo(int& plugin_major, int& plugin_minor,
	     int& browser_major, int& browser_minor);

    virtual TQNPInstance* newInstance()=0;
    virtual const char* getMIMEDescription() const=0;
    virtual const char* getPluginNameString() const=0;
    virtual const char* getPluginDescriptionString() const=0;

    virtual void* getJavaClass();
    virtual void unuseJavaClass();
    void* getJavaEnv() const;

protected:
    TQNPlugin();
};


#endif  // TQNP_H
