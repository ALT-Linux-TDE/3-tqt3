void MulticlipForm::addClipping()
{
    TQString text = currentLineEdit->text();
    if ( ! text.isEmpty() ) {
	lengthLCDNumber->display( (int)text.length() );
	int i = 0;
	for ( ; i < (int)clippingsListBox->count(); i++ ) {
	    if ( clippingsListBox->text( i ) == text ) {
		i = -1; // Do not add duplicates
		break;
	    }
	}
	if ( i != -1 )
	    clippingsListBox->insertItem( text, 0 );
    }
}

void MulticlipForm::dataChanged()
{
    TQString text;
    text = cb->text();
    clippingChanged( text );
    if ( autoCheckBox->isChecked() )
	addClipping();
}

void MulticlipForm::deleteClipping()
{
    clippingChanged( "" );
    clippingsListBox->removeItem( clippingsListBox->currentItem() );
}

void MulticlipForm::init()
{
    lengthLCDNumber->setBackgroundColor( darkBlue );
    currentLineEdit->setFocus();

    cb = tqApp->clipboard();
    connect( cb, TQ_SIGNAL( dataChanged() ), TQ_SLOT( dataChanged() ) );
    if ( cb->supportsSelection() )
	connect( cb, TQ_SIGNAL( selectionChanged() ), TQ_SLOT( selectionChanged() ) );

    dataChanged();
}

void MulticlipForm::selectionChanged()
{
    cb->setSelectionMode( TRUE );
    dataChanged();
    cb->setSelectionMode( FALSE );
}

void MulticlipForm::copyPrevious()
{
    if ( clippingsListBox->currentItem() != -1 ) {
	cb->setText( clippingsListBox->currentText() );
	if ( cb->supportsSelection() ) {
	    cb->setSelectionMode( TRUE );
	    cb->setText( clippingsListBox->currentText() );
	    cb->setSelectionMode( FALSE );
	}
    }
}

void MulticlipForm::clippingChanged( const TQString & clipping )
{
    currentLineEdit->setText( clipping );
    lengthLCDNumber->display( (int)clipping.length() );
}

