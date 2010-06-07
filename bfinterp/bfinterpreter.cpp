#include "bfinterpreter.h"

namespace QtBrain {
    BfInterpreter::BfInterpreter(QObject *parent) : QThread(parent), d_ptr(new BfInterpreterPrivate(this))
    {}

    BfInterpreter::BfInterpreter(BfInterpreterPrivate &dd, QObject *parent) : QThread(parent), d_ptr(&dd)
    {}

    BfInterpreter::~BfInterpreter() {
        delete d_ptr;
    }

    /////////////////////////////////////////////////////////////////////////////////////
    //// SLOTS FOR EXTERNAL USE
    ///////////////////////////
    void BfInterpreter::changeDelay(int) {
    }

    void BfInterpreter::initialize(const QList<BfOpcode> &src) {
    }

    void BfInterpreter::input(const QString &in){

    }

    void BfInterpreter::setBreakpoint(IPType pos) {

    }

    //////////////////////////////////////////////////////////////////////////////////
    //// PROTECTED METHODS
    //////////////////////

    void BfInterpreter::run() {
    }

}
