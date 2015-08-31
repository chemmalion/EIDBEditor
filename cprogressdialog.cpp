#include "cprogressdialog.h"
#include "ui_cprogressdialog.h"

CProgressDialog::CProgressDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CProgressDialog)
{
    ui->setupUi(this);
}

CProgressDialog::~CProgressDialog()
{
    delete ui;
}

void CProgressDialog::goShow( QWidget * parent )
{
    if( parent == NULL ) return show();
    QRect rect = geometry();
    QRect p_rect = parent->geometry();
    QPoint lefttop = rect.topLeft();
    lefttop.setX( p_rect.left() + ( p_rect.width() - rect.width() ) / 2 );
    lefttop.setY( p_rect.top() + ( p_rect.height() - rect.height() ) / 2 );
    rect.moveTo( lefttop );
    setGeometry( rect );
    return show();
}

void CProgressDialog::setLabel( QString label )
{
    ui->label->setText( label );
}

void CProgressDialog::setProgress( int value )
{
    ui->progressBar->setValue( value );
}

void CProgressDialog::setProgressMaximum( int value )
{
    ui->progressBar->setMaximum( value );
}
