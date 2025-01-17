/**********************************************************************
**
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

void MainWindowWizardBase::init()
{
    dIface = 0;
    dfw = 0;
    widget = 0;
    setHelpEnabled( menuToolbarPage, FALSE );
    setHelpEnabled( toolbarsPage, FALSE );
    setHelpEnabled( finishPage, FALSE );
    setFinishEnabled( finishPage, TRUE );
}

void MainWindowWizardBase::destroy()
{
    if ( dIface )
	dIface->release();
}

void MainWindowWizardBase::currentSettingChanged()
{
    bool appr = checkFileToolbar->isChecked() || checkEditToolbar->isChecked() || checkHelpToolbar->isChecked();
    setAppropriate( toolbarsPage, appr );
}

void MainWindowWizardBase::setupToolbarPage()
{
    if ( checkFileToolbar->isChecked() )
	comboToolbar->insertItem( tr( "File" ) );
    if ( checkEditToolbar->isChecked() )
	comboToolbar->insertItem( tr( "Edit" ) );
    if ( checkHelpToolbar->isChecked() )
	comboToolbar->insertItem( tr( "Help" ) );
}

void MainWindowWizardBase::toolbarActionDown()
{
    if ( listToolbar->currentItem() < 0 || listToolbar->currentItem() >= (int)listToolbar->count() - 1 )
	return;

    int next = listToolbar->currentItem() + 1;
    TQString tmp = listToolbar->text( next );
    listToolbar->changeItem( listToolbar->text( next - 1 ), next );
    listToolbar->changeItem( tmp, next - 1 );
    listToolbar->setCurrentItem( next );
}

void MainWindowWizardBase::toolbarActionUp()
{
    if ( listToolbar->currentItem() < 1 || listToolbar->currentItem() > (int)listToolbar->count() - 1 )
	return;

    int prev = listToolbar->currentItem() - 1;
    TQString tmp = listToolbar->text( prev );
    listToolbar->changeItem( listToolbar->text( prev + 1 ), prev );
    listToolbar->changeItem( tmp, prev + 1 );
    listToolbar->setCurrentItem( prev );
}

void MainWindowWizardBase::toolbarAddAction()
{
    if ( listToolbarActions->text( listToolbarActions->currentItem() ).isEmpty() )
	return;
    listToolbar->insertItem( listToolbarActions->text( listToolbarActions->currentItem() ) );
    listToolbar->setCurrentItem( listToolbar->count() - 1 );
    listToolbarActions->setCurrentItem( listToolbarActions->currentItem() + 1 );
}

void MainWindowWizardBase::toolbarRemoveAction()
{
    listToolbar->removeItem( listToolbar->currentItem() );
}

static void devNull( TQtMsgType, const char * )
{
}

void MainWindowWizardBase::accept()
{
    if ( !dfw || !dIface || !widget ) {
	TQWizard::accept();
	return;
    }

    setFinishEnabled( finishPage, FALSE );

    TQPixmap pix;

    TQDict<TQAction> actions;
    TQPtrList<TQAction> usedActions;

    TQtMsgHandler oldMsgHandler = qInstallMsgHandler( devNull );

    getPixmap( "document-new", pix );

    TQAction *fileNewAction = dfw->createAction( tr( "New" ), pix, tr( "&New" ), CTRL + Key_N, 0, "fileNewAction" );
    actions.insert( tr( "New" ), fileNewAction );
    getPixmap( "document-open", pix );
    TQAction *fileOpenAction = dfw->createAction( tr( "Open" ), pix, tr( "&Open..." ), CTRL + Key_O, 0, "fileOpenAction" );
    actions.insert( tr( "Open" ), fileOpenAction );
    getPixmap( "document-save", pix );
    TQAction *fileSaveAction = dfw->createAction( tr( "Save" ), pix, tr( "&Save" ), CTRL + Key_S, 0, "fileSaveAction" );
    actions.insert( tr( "Save" ), fileSaveAction );
    TQAction *fileSaveAsAction = dfw->createAction( tr( "Save As" ), TQIconSet(), tr( "Save &As..." ), 0, 0, "fileSaveAsAction" );
    actions.insert( tr( "Save As" ), fileSaveAsAction );
    getPixmap( "print", pix );
    TQAction *filePrintAction = dfw->createAction( tr( "Print" ), pix, tr( "&Print..." ), CTRL + Key_P, 0, "filePrintAction" );
    actions.insert( tr( "Print" ), filePrintAction );
    TQAction *fileExitAction = dfw->createAction( tr( "Exit" ), TQIconSet(), tr( "E&xit" ), 0, 0, "fileExitAction" );
    actions.insert( tr( "Exit" ), fileExitAction );

    getPixmap( "undo", pix );
    TQAction *editUndoAction = dfw->createAction( tr( "Undo" ), pix, tr( "&Undo" ), CTRL + Key_Z, 0, "editUndoAction" );
    actions.insert( tr( "Undo" ), editUndoAction );
    getPixmap( "redo", pix );
    TQAction *editRedoAction = dfw->createAction( tr( "Redo" ), pix, tr( "&Redo" ), CTRL + Key_Y, 0, "editRedoAction" );
    actions.insert( tr( "Redo" ), editRedoAction );
    getPixmap( "edit-cut", pix );
    TQAction *editCutAction = dfw->createAction( tr( "Cut" ), pix, tr( "Cu&t" ), CTRL + Key_X, 0, "editCutAction" );
    actions.insert( tr( "Cut" ), editCutAction );
    getPixmap( "edit-copy", pix );
    TQAction *editCopyAction = dfw->createAction( tr( "Copy" ), pix, tr( "&Copy" ), CTRL + Key_C, 0, "editCopyAction" );
    actions.insert( tr( "Copy" ), editCopyAction );
    getPixmap( "edit-paste", pix );
    TQAction *editPasteAction = dfw->createAction( tr( "Paste" ), pix, tr( "&Paste" ), CTRL + Key_V, 0, "editPasteAction" );
    actions.insert( tr( "Paste" ), editPasteAction );
    getPixmap( "searchfind", pix );
    TQAction *editFindAction = dfw->createAction( tr( "Find" ), pix, tr( "&Find..." ), CTRL + Key_F, 0, "editFindAction" );
    actions.insert( tr( "Find" ), editFindAction );

    TQAction *helpAboutAction = dfw->createAction( tr( "About" ), TQIconSet(), tr( "&About" ), 0, 0, "helpAboutAction" );
    actions.insert( tr( "About" ), helpAboutAction );
    TQAction *helpContentsAction = dfw->createAction( tr( "Contents" ), TQIconSet(), tr( "&Contents..." ), 0, 0, "helpContentsAction" );
    actions.insert( tr( "Contents" ), helpContentsAction );
    TQAction *helpIndexAction = dfw->createAction( tr( "Index" ), TQIconSet(), tr( "&Index..." ), 0, 0, "helpIndexAction" );
    actions.insert( tr( "Index" ), helpIndexAction );

    if ( checkFileMenu->isChecked() ) {
	dfw->addMenu( "&File", "fileMenu" );
	dfw->addMenuAction( "fileMenu", fileNewAction );
	dfw->addMenuAction( "fileMenu", fileOpenAction );
	dfw->addMenuAction( "fileMenu", fileSaveAction );
	dfw->addMenuAction( "fileMenu", fileSaveAsAction );
	dfw->addMenuSeparator( "fileMenu" );
	dfw->addMenuAction( "fileMenu", filePrintAction );
	dfw->addMenuSeparator( "fileMenu" );
	dfw->addMenuAction( "fileMenu", fileExitAction );
	dfw->addAction( fileNewAction );
	dfw->addAction( fileOpenAction );
	dfw->addAction( fileSaveAction );
	dfw->addAction( fileSaveAsAction );
	dfw->addAction( filePrintAction );
	dfw->addAction( fileExitAction );
	usedActions.append( fileNewAction );
	usedActions.append( fileOpenAction );
	usedActions.append( fileSaveAction );
	usedActions.append( fileSaveAsAction );
	usedActions.append( filePrintAction );
	usedActions.append( fileExitAction );
    }

    if ( checkEditMenu->isChecked() ) {
	dfw->addMenu( "&Edit", "editMenu" );
	dfw->addMenuAction( "editMenu", editUndoAction );
	dfw->addMenuAction( "editMenu", editRedoAction );
	dfw->addMenuSeparator( "editMenu" );
	dfw->addMenuAction( "editMenu", editCutAction );
	dfw->addMenuAction( "editMenu", editCopyAction );
	dfw->addMenuAction( "editMenu", editPasteAction );
	dfw->addMenuSeparator( "editMenu" );
	dfw->addMenuAction( "editMenu", editFindAction );
	dfw->addAction( editUndoAction );
	dfw->addAction( editRedoAction );
	dfw->addAction( editCutAction );
	dfw->addAction( editCopyAction );
	dfw->addAction( editPasteAction );
	dfw->addAction( editFindAction );
	usedActions.append( editUndoAction );
	usedActions.append( editRedoAction );
	usedActions.append( editCutAction );
	usedActions.append( editCopyAction );
	usedActions.append( editPasteAction );
	usedActions.append( editFindAction );
    }

    if ( checkHelpMenu->isChecked() ) {
	dfw->addMenu( "&Help", "helpMenu" );
	dfw->addMenuAction( "helpMenu", helpContentsAction );
	dfw->addMenuAction( "helpMenu", helpIndexAction );
	dfw->addMenuSeparator( "helpMenu" );
	dfw->addMenuAction( "helpMenu", helpAboutAction );
	dfw->addAction( helpContentsAction );
	dfw->addAction( helpIndexAction );
	dfw->addAction( helpAboutAction );
	usedActions.append( helpIndexAction );
	usedActions.append( helpContentsAction );
	usedActions.append( helpAboutAction );
    }

    if ( listToolbar->count() > 0 && appropriate( toolbarsPage ) ) {
	dfw->addToolBar( "Tools", "toolBar" );
	for ( int i = 0; i < (int)listToolbar->count(); ++i ) {
	    if ( listToolbar->text( i ) == tr( "<Separator>" ) ) {
		dfw->addToolBarSeparator( "toolBar" );
		continue;
	    }
	    TQAction *a = actions.find( listToolbar->text( i ) );
	    if ( !a )
		continue;
	    dfw->addToolBarAction( "toolBar", a );
	    dfw->addAction( a );
	    if ( usedActions.findRef( a ) == -1 )
		usedActions.append( a );
	}
    }
    for ( TQAction *ac = usedActions.first(); ac; ac = usedActions.next() ) {
	if ( ( TQString( ac->name() ).find( "file" ) != -1 && checkCreateConnectionsFile->isChecked() ) ||
	     ( TQString( ac->name() ).find( "edit" ) != -1 && checkCreateConnectionsEdit->isChecked() ) ||
	     ( TQString( ac->name() ).find( "help" ) != -1 && checkCreateConnectionsHelp->isChecked() ) ) {
		TQString slot = ac->name();
		slot.remove( slot.length() - 6, 6 );
		slot += "()";
		dfw->addFunction( slot.latin1(), "virtual", "public", "slot", dIface->currentProject()->language(), "void" );
		dfw->addConnection( ac, "activated()", widget, slot );
	}
    }

    qInstallMsgHandler( oldMsgHandler );

    TQWizard::accept();
}

void MainWindowWizardBase::currentToolbarChanged( const TQString & s )
{
    if ( s == tr( "File" ) ) {
	listToolbarActions->clear();
	listToolbarActions->insertItem( tr( "New" ) );
	listToolbarActions->insertItem( tr( "Open" ) );
	listToolbarActions->insertItem( tr( "Save" ) );
	listToolbarActions->insertItem( tr( "Save As" ) );
	listToolbarActions->insertItem( tr( "Print" ) );
	listToolbarActions->insertItem( tr( "Exit" ) );
    } else if ( s == tr( "Edit" ) ) {
	listToolbarActions->clear();
	listToolbarActions->insertItem( tr( "Undo" ) );
	listToolbarActions->insertItem( tr( "Redo" ) );
	listToolbarActions->insertItem( tr( "Cut" ) );
	listToolbarActions->insertItem( tr( "Copy" ) );
	listToolbarActions->insertItem( tr( "Paste" ) );
	listToolbarActions->insertItem( tr( "Find" ) );
    } else if ( s == tr( "Help" ) ) {
	listToolbarActions->clear();
	listToolbarActions->insertItem( tr( "Contents" ) );
	listToolbarActions->insertItem( tr( "Index" ) );
	listToolbarActions->insertItem( tr( "About" ) );
    }
    listToolbarActions->insertItem( "<Separator>" );
    listToolbarActions->setCurrentItem( 0 );
}

void MainWindowWizardBase::getPixmap( const TQString & n, TQPixmap & pix )
{
    TQString name = n + ".png";
    pix = TQPixmap::fromMimeSource( name );
    DesignerProject *pro = dIface->currentProject();
    if ( !pro || pro->projectName() == "<No Project>" )
	return;
    pro->pixmapCollection()->addPixmap( pix, n, FALSE );
}

void MainWindowWizardBase::pageSelected( const TQString & )
{
    if ( currentPage() == toolbarsPage ) {
	comboToolbar->clear();
	setupToolbarPage();
	listToolbarActions->clear();
	currentToolbarChanged( comboToolbar->text( comboToolbar->currentItem() ) );
    }
}

void MainWindowWizardBase::setAppInterface( TQUnknownInterface * iface, DesignerFormWindow * fw, TQWidget * w )
{
    DesignerInterface *d = 0;
    iface->queryInterface(  IID_Designer, (TQUnknownInterface**)&d );
    dIface = d;
    dfw = fw;
    widget = w;
}

