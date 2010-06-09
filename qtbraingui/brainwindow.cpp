#include "brainwindow.h"
#include "ui_brainwindow.h"
#include "bfinterpreter.h"

using namespace QtBrain;

BrainWindow::BrainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::BrainWindow)
{
    ui->setupUi(this);
}

BrainWindow::~BrainWindow()
{
    delete ui;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// PROTECTED METHODS
//////////////////////

void BrainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void BrainWindow::closeEvent(QCloseEvent *e) {
//    if(maybeSave()) {
//        e->accept();
//    } else {
//        e->ignore();
//    }
}
