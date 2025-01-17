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

#include "dbconnectionsimpl.h"
#include <ntqptrlist.h>
#include <ntqgroupbox.h>
#include <ntqlayout.h>
#include "project.h"
#include <ntqlistbox.h>
#include <ntqcombobox.h>
#include <ntqspinbox.h>
#include <ntqlineedit.h>
#include <ntqpushbutton.h>
#include <ntqsqldatabase.h>
#include <ntqmessagebox.h>
#include <ntqapplication.h>
#include "mainwindow.h"
#include "asciivalidator.h"

static bool blockChanges = FALSE;

/*
 *  Constructs a DatabaseConnectionsEditor which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
DatabaseConnectionsEditor::DatabaseConnectionsEditor( Project *pro, TQWidget* parent,  const char* name, bool modal, WFlags fl )
    : DatabaseConnectionBase( parent, name, modal, fl ), project( pro )
{
    connect( buttonHelp, TQ_SIGNAL( clicked() ), MainWindow::self, TQ_SLOT( showDialogHelp() ) );
    connectionWidget = new DatabaseConnectionWidget( grp );
    grpLayout->addWidget( connectionWidget, 0, 0 );
#ifndef TQT_NO_SQL
    TQPtrList<DatabaseConnection> lst = project->databaseConnections();
    for ( DatabaseConnection *conn = lst.first(); conn; conn = lst.next() )
	listConnections->insertItem( conn->name() );
    connectionWidget->comboDriver->insertStringList( TQSqlDatabase::drivers() );
#endif
    connectionWidget->editName->setValidator( new AsciiValidator( connectionWidget->editName ) );
    enableAll( FALSE );
}

DatabaseConnectionsEditor::~DatabaseConnectionsEditor()
{
}

void DatabaseConnectionsEditor::deleteConnection()
{
    if ( listConnections->currentItem() == -1 )
	return;
    project->removeDatabaseConnection( listConnections->currentText() );
    delete listConnections->item( listConnections->currentItem() );
    if ( listConnections->count() ) {
	listConnections->setCurrentItem( 0 );
	currentConnectionChanged( listConnections->currentText() );
    } else {
	enableAll( FALSE );
    }
    project->saveConnections();
}

void DatabaseConnectionsEditor::newConnection()
{
    blockChanges = TRUE;
    enableAll( TRUE );
    TQString n( "(default)" );
    if ( project->databaseConnection( n ) ) {
	n = "connection";
	int i = 2;
	while ( project->databaseConnection( n + TQString::number( i ) ) )
	    ++i;
	n = n + TQString::number( i );
    }
    connectionWidget->editName->setText( n );
    listConnections->clearSelection();
    buttonConnect->setDefault( TRUE );
    connectionWidget->editName->setFocus();
    blockChanges = FALSE;
}

void DatabaseConnectionsEditor::doConnect()
{
#ifndef TQT_NO_SQL
    if ( listConnections->currentItem() == -1 ||
	 !listConnections->item( listConnections->currentItem() )->isSelected() ) { // new connection
	// ### do error checking for duplicated connection names
	DatabaseConnection *conn = new DatabaseConnection( project );
	conn->setName( connectionWidget->editName->text() );
	conn->setDriver( connectionWidget->comboDriver->lineEdit()->text() );
	conn->setDatabase( connectionWidget->editDatabase->text() );
	conn->setUsername( connectionWidget->editUsername->text() );
	conn->setPassword( connectionWidget->editPassword->text() );
	conn->setHostname( connectionWidget->editHostname->text() );
	conn->setPort( connectionWidget->editPort->value() );
	if ( conn->refreshCatalog() ) {
	    project->addDatabaseConnection( conn );
	    listConnections->insertItem( conn->name() );
	    listConnections->setCurrentItem( listConnections->count() - 1 );
	    project->saveConnections();
	} else {
	    TQMessageBox::warning( MainWindow::self, TQApplication::tr( "Connection" ),
				  TQApplication::tr( "Could not connect to the database.\n"
						    "Please ensure that the database server is running "
						    "and that all the connection information is correct.\n"
						    "[ " + conn->lastError() + " ]" ) );
	    delete conn;
	}
    } else { // sync // ### should this do something else? right now it just overwrites all info about the connection...
	DatabaseConnection *conn = project->databaseConnection( listConnections->currentText() );
	conn->setName( connectionWidget->editName->text() );
	conn->setDriver( connectionWidget->comboDriver->lineEdit()->text() );
	conn->setDatabase( connectionWidget->editDatabase->text() );
	conn->setUsername( connectionWidget->editUsername->text() );
	conn->setPassword( connectionWidget->editPassword->text() );
	conn->setHostname( connectionWidget->editHostname->text() );
	conn->setPort( connectionWidget->editPort->value() );
	conn->refreshCatalog();
	project->saveConnections();
    }
#endif
}

void DatabaseConnectionsEditor::currentConnectionChanged( const TQString &s )
{
#ifndef TQT_NO_SQL
    DatabaseConnection *conn = project->databaseConnection( s );
    blockChanges = TRUE;
    enableAll( conn != 0 );
    connectionWidget->editName->setEnabled( FALSE );
    blockChanges = FALSE;
    if ( !conn )
	return;
    blockChanges = TRUE;
    connectionWidget->editName->setText( conn->name() );
    blockChanges = FALSE;
    connectionWidget->comboDriver->lineEdit()->setText( conn->driver() );
    connectionWidget->editDatabase->setText( conn->database() );
    connectionWidget->editUsername->setText( conn->username() );
    connectionWidget->editPassword->setText( conn->password() );
    connectionWidget->editHostname->setText( conn->hostname() );
    connectionWidget->editPort->setValue( conn->port() );
#endif
}

void DatabaseConnectionsEditor::connectionNameChanged( const TQString &s )
{
    if ( listConnections->currentItem() == 0 || blockChanges )
	return;
    listConnections->changeItem( s, listConnections->currentItem() );
}

void DatabaseConnectionsEditor::enableAll( bool b )
{
    connectionWidget->editName->setEnabled( b );
    connectionWidget->editName->setText( "" );
    connectionWidget->comboDriver->setEnabled( b );
    connectionWidget->comboDriver->lineEdit()->setText( "" );
    connectionWidget->editDatabase->setEnabled( b );
    connectionWidget->editDatabase->setText( "" );
    connectionWidget->editUsername->setEnabled( b );
    connectionWidget->editUsername->setText( "" );
    connectionWidget->editPassword->setEnabled( b );
    connectionWidget->editPassword->setText( "" );
    connectionWidget->editHostname->setEnabled( b );
    connectionWidget->editHostname->setText( "" );
    connectionWidget->editPort->setEnabled( b );
    connectionWidget->editPort->setValue( -1 );
    buttonConnect->setEnabled( b );
}
