/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions use TQt Designer which will
** update this file, preserving your code. Create an init() function in place
** of a constructor, and a destroy() function in place of a destructor.
*****************************************************************************/


void FindForm::find()
{
    emit lookfor( findLineEdit->text() );
}

void FindForm::notfound()
{
    findLineEdit->selectAll();
}
