#ifndef CDIALOGABOUT_H
#define CDIALOGABOUT_H

#include <QDialog>

namespace Ui {
    class CDialogAbout;
}

class CDialogAbout : public QDialog
{
    Q_OBJECT

public:

    explicit CDialogAbout(QWidget *parent = 0);
    ~CDialogAbout();

    int fire( QWidget * parent );

protected:
    void changeEvent(QEvent *e);

private:
    Ui::CDialogAbout *ui;
};

#endif // CDIALOGABOUT_H
