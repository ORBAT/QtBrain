#include "brainconsole.h"
#include "bfinterpreter.h"
#include <QDebug>

BrainConsole::BrainConsole(QObject *parent) :
    QObject(parent), bfi(new BfInterpreter()), bfiThread(new QThread())
{
    bfiThread->moveToThread(bfiThread);
    bfiThread->start();
    qDebug() << "Console thread"<<thread()->currentThreadId();



    connect(bfiThread, SIGNAL(started()), this, SLOT(bfiTest()));

    qDebug() << "bfi thread"<< bfi->thread()->currentThreadId();
    bfi->moveToThread(bfiThread);
    qDebug() << "bfi thread now"<< bfi->thread()->currentThreadId();
    connect(bfi, SIGNAL(inited()), this, SLOT(bfiTest()));
    //connect(this, SIGNAL(iniVm(QList<BfOpcode>)), bfi, SLOT(initialize(QList<BfOpcode>)));
    connect(this, SIGNAL(iniVm(QList<BfOpcode> const*)), bfi, SLOT(initialize(QList<BfOpcode> const*)));
    QList<BfOpcode> derp;
    derp << ADD;
    emit iniVm();

}

BrainConsole::~BrainConsole() {
//    delete bfi;
//    delete bfiThread;

}

void BrainConsole::bfiTest() {
    qDebug() << "bfiTest() curr thread" << thread()->currentThreadId();

}
