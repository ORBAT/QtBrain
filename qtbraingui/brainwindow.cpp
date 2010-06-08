#include "brainwindow.h"
#include "ui_brainwindow.h"
#include "bfinterpreter.h"

using namespace QtBrain;

BrainWindow::BrainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::BrainWindow)
{
    ui->setupUi(this);
    BfInterpreter *derp = new BfInterpreter(this);
    derp->start();
    connect(this,SIGNAL(testSig(int)), derp, SLOT(changeDelay(int)));
    emit testSig(100);
    derp->exit();
}

BrainWindow::~BrainWindow()
{
    delete ui;
}

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
