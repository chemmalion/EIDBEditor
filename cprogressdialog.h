#ifndef CPROGRESSDIALOG_H
#define CPROGRESSDIALOG_H

#include <QDialog>

namespace Ui {
    class CProgressDialog;
}

class CProgressDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CProgressDialog(QWidget *parent = 0);
    ~CProgressDialog();

    void goShow( QWidget * parent );

    void setLabel( QString label );
    void setProgress( int value );
    void setProgressMaximum( int value );

private:
    Ui::CProgressDialog *ui;
};

#endif // CPROGRESSDIALOG_H
