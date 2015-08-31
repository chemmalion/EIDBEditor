#include "cdialogabout.h"
#include "ui_cdialogabout.h"

CDialogAbout::CDialogAbout(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CDialogAbout)
{
    ui->setupUi(this);
}

CDialogAbout::~CDialogAbout()
{
    delete ui;
}

int CDialogAbout::fire( QWidget * parent )
{
    if( parent == NULL ) return exec();
    QRect rect = geometry();
    QRect p_rect = parent->geometry();
    QPoint lefttop = rect.topLeft();
    lefttop.setX( p_rect.left() + ( p_rect.width() - rect.width() ) / 2 );
    lefttop.setY( p_rect.top() + ( p_rect.height() - rect.height() ) / 2 );
    rect.moveTo( lefttop );
    setGeometry( rect );
    return exec();
}

void CDialogAbout::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
