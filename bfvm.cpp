/*
Copyright 2010 Tom Eklof. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are
permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright notice, this list of
      conditions and the following disclaimer.

   2. Redistributions in binary form must reproduce the above copyright notice, this list
      of conditions and the following disclaimer in the documentation and/or other materials
      provided with the distribution.

THIS SOFTWARE IS PROVIDED BY TOM EKLOF ``AS IS'' AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL TOM EKLOF OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "bfvm.h"
#include "customTransitions.h"
#include <stdexcept>
#include <QStateMachine>
#include <QState>
#include <QAbstractTransition>
#include <QTimer>
#include <QDebug>
#include <QStack>
#include <QByteArray>
#include <QHistoryState>

namespace QtBrain {



    BfVM::BfVM(QObject *parent) :
            QThread(parent),
            m_DP(0), m_IP(0), m_programSize(0),
            m_runDelay(200),
            m_memory(new Memtype[MAX_MEM_ADDR+1]),
            m_jmps(new BiHash<IPType,IPType>()),
            m_program(NULL),
            m_runTimer(new QTimer(this)),
            m_inputBuffer(new QQueue<Memtype>()),
            m_breakpoints(new QList<IPType>()),
            m_stateMachine(new QStateMachine(this)), ///// STATE INITIALIZATIONS
            m_stateGroup(new QState()),
            m_runGroup(new QState(m_stateGroup)),
            m_emptySt(new QState(m_stateGroup)),
            m_initializedSt(new QState(m_stateGroup)),
            m_steppingSt(new QState(m_runGroup)),
            m_runningSt(new QState(m_runGroup)),
            m_finishedSt(new QState(m_stateGroup)),
            m_waitingForInpSt(new QState(m_stateGroup)),
            m_breakpointSt(new QState(m_runGroup)),
            m_runHistorySt(new QHistoryState(m_runGroup)),
            m_clearSt(new QState(m_stateGroup))///// END OF STATE INITIALIZATION

    {
        qDebug() << "BfVM::BfVM()\nLargest address:" << MAX_MEM_ADDR;

        // initialize memory to all 0
        clearMemory();

        initializeStateMachine();
        m_stateMachine->start();
        /////////////////////////////////////////////////////////////////////////////////////
        //// TIMER SETUP
        ////////////////

        m_runTimer->setInterval(m_runDelay);
        // running a program is just "single-stepping by the clock."
        connect(m_runTimer, SIGNAL(timeout()), this, SLOT(step()));
    }


    void BfVM::initializeStateMachine() {
        // see the state chart for a clearer picture of what's going on with the states
        /////////////////////////////////////////////////////////////////////////////////////
        //// STATE NAME INITIALIZATIONS
        ///////////////////////////////
        //: Please don't translate the state names in case I forget to remove the tr()s
        m_stateGroup->setProperty("statename", QVariant::fromValue(tr("main state group")));
        m_runGroup->setProperty("statename", QVariant::fromValue(tr("run state group")));
        m_emptySt->setProperty("statename", QVariant::fromValue(tr("empty VM")));
        m_steppingSt->setProperty("statename", QVariant::fromValue(tr("stepping")));
        m_runningSt->setProperty("statename", QVariant::fromValue(tr("running")));
        m_clearSt->setProperty("statename", QVariant::fromValue(tr("clearing")));
        m_initializedSt->setProperty("statename", QVariant::fromValue(tr("initialized")));
        m_finishedSt->setProperty("statename", QVariant::fromValue(tr("finished")));
        m_waitingForInpSt->setProperty("statename", QVariant::fromValue(
                tr("waiting for input")));
        m_breakpointSt->setProperty("statename", QVariant::fromValue(tr("breakpoint")));
        m_runHistorySt->setProperty("statename", QVariant::fromValue(
                tr("run history state")));


        /////////////////////////////////////////////////////////////////////////////////////
        //// EMPTY STATE
        ////////////////
        /*
         Only transition away from the empty state when the VM is initialized, ie. a
         Brainfuck program is loaded succesfully
          */
        InitedEventTransition *iet = new InitedEventTransition();
        iet->setTargetState(m_initializedSt);
        m_emptySt->addTransition(iet);


        /////////////////////////////////////////////////////////////////////////////////////
        //// INITIALIZED STATE
        //////////////////////
        // reset the VM when the initialized state is entered
        connect(m_initializedSt, SIGNAL(entered()), this, SLOT(reset()));

        // emit inited() when entering the initialized state so the GUI knows we're ready
        connect(m_initializedSt, SIGNAL(entered()), this, SIGNAL(inited()));

        m_initializedSt->addTransition(this, SIGNAL(toggleRunSig()), m_runningSt);
        m_initializedSt->addTransition(this, SIGNAL(stepSig()), m_steppingSt);


        /////////////////////////////////////////////////////////////////////////////////////
        //// STEPPING STATE
        ///////////////////
        // run step() whenever this state is entered
        connect(m_steppingSt, SIGNAL(entered()), this, SLOT(step()));
        // just loops back to itself when receiving additional stepSig()s
        m_steppingSt->addTransition(this, SIGNAL(stepSig()), m_steppingSt);
        // Transition to running if toggleRunSig() received
        m_steppingSt->addTransition(this, SIGNAL(toggleRunSig()), m_runningSt);


        /////////////////////////////////////////////////////////////////////////////////////
        //// RUNNING STATE
        //////////////////
        /* Entering or exiting this state controls whether the run timer is running. */
        connect(m_runningSt, SIGNAL(entered()), this, SLOT(go()));
        connect(m_runningSt, SIGNAL(exited()), this, SLOT(stop()));
        m_runningSt->addTransition(this, SIGNAL(stepSig()), m_steppingSt);
        m_runningSt->addTransition(this, SIGNAL(toggleRunSig()), m_steppingSt);


        /////////////////////////////////////////////////////////////////////////////////////
        //// FINISHED STATE
        ///////////////////
        /* the only way to get out of the finished state is to reset or clear the VM.
           finish() is emitted when this state is entered so that the GUI can disable
           actions and whatnot */

        connect(m_finishedSt, SIGNAL(entered()), this, SIGNAL(finish()));
        m_finishedSt->addTransition(this, SIGNAL(resetSig()), m_initializedSt);

        /////////////////////////////////////////////////////////////////////////////////////
        //// WAITING FOR INPUT STATE
        ////////////////////////////

        // signal the GUI that the VM needs input
        connect(m_waitingForInpSt, SIGNAL(entered()), this, SIGNAL(needInput()));

        /* send the same signal when exiting the state, so the GUI can toggle its
           input notification */
        connect(m_waitingForInpSt, SIGNAL(exited()), this, SIGNAL(needInput()));

        /* To get out of the wait state we need either input or a reset/clear.
           Clears are handled by the top state group, but we need to catch the reset
           signal ourselves.
           Transitions back to the correct state of the running state group by using
           m_runGroup's history state */
        InpBufFilledTransition *ibft = new InpBufFilledTransition();
        ibft->setTargetState(m_runHistorySt);
        m_waitingForInpSt->addTransition(ibft);
        m_waitingForInpSt->addTransition(this, SIGNAL(resetted()), m_initializedSt);


        /////////////////////////////////////////////////////////////////////////////////////
        //// BREAKPOINT STATE
        /////////////////////
        connect(m_breakpointSt, SIGNAL(entered()), this, SLOT(breakPtTest()));
        m_breakpointSt->addTransition(this, SIGNAL(stepSig()), m_steppingSt);
        m_breakpointSt->addTransition(this, SIGNAL(toggleRunSig()), m_runningSt);


        /////////////////////////////////////////////////////////////////////////////////////
        //// CLEARING STATE
        ///////////////////
        connect(m_clearSt, SIGNAL(entered()), this, SLOT(clear()));
        m_clearSt->addTransition(m_emptySt);

        /////////////////////////////////////////////////////////////////////////////////////
        //// TOP STATE GROUP
        ////////////////////
        m_stateGroup->setInitialState(m_emptySt);  // start in the empty state
        // if the clearSig() signal is sent when in any state, go to the clear state
        m_stateGroup->addTransition(this, SIGNAL(clearSig()), m_clearSt);


        /////////////////////////////////////////////////////////////////////////////////////
        //// RUN STATE GROUP
        ////////////////////
        m_runGroup->setInitialState(m_steppingSt); /* the run group always starts in the
                                                      stepping state*/

        /* If the VM is reset while running, go back to the initialized state */
        m_runGroup->addTransition(this, SIGNAL(resetSig()), m_initializedSt);

        // Stop the VM when exiting the run group
        connect(m_runGroup, SIGNAL(exited()), this, SLOT(stop()));

        /* handle an EndEvent in any run state by transitioning to the finished state */
        EndEventTransition *eet = new EndEventTransition();
        eet->setTargetState(m_finishedSt);
        m_runGroup->addTransition(eet);

        /* if the input buffer is empty and the VM needs input, go into the waiting
           for input state */
        InpBufEmptyTransition *ibet = new InpBufEmptyTransition();
        ibet->setTargetState(m_waitingForInpSt);
        m_runGroup->addTransition(ibet);

        // go to the breakpoint state if a breakpoint was reached
        BreakpointTransition *bpt = new BreakpointTransition();
        bpt->setTargetState(m_breakpointSt);
        m_runGroup->addTransition(bpt);



        /////////////////////////////////////////////////////////////////////////////////////
        //// STATE MACHINE INITIALIZATION
        /////////////////////////////////

        m_stateMachine->addState(m_stateGroup);
        m_stateMachine->setInitialState(m_stateGroup);
    }


    void BfVM::breakPtTest() {
        qDebug() << "BfVM::breakPtTest()";
    }

    BfVM::~BfVM() {
        qDebug("~BfVM()");
        delete[] m_memory;
        delete m_jmps;
        delete[] m_program;
        delete m_stateGroup;
        delete m_inputBuffer;
        delete m_breakpoints;
    }

    /////////////////////////////////////////////////////////////////////////////////////////
    //// SLOTS
    //////////
    void BfVM::step() {
        qDebug("BfVM::step()");

        //Q_ASSERT_X(m_IP < m_programSize, "BfVM::step()", "IP larger than program size");
        /* If we've reached the last instruction, post an EndEvent and let the state machine
           handle the rest */
        if(m_IP >= m_programSize) {
            qDebug("BfVM::step() program end reached");
            m_stateMachine->postEvent(new EndEvent);
            return;
        }
        // Inform the world of our IP
        emit heartBeat(m_IP);

        // NOTE: the IP is increased by the runInstruction() function
        runInstruction(m_program[m_IP]);

        // emit "heart beat" with current IP and then increase it
    }

    void BfVM::go() {
        qDebug("BfVM::go()");
        // Look ma, no loops!
        m_runTimer->start();
        emit running(true);
#ifndef QT_NO_DEBUG
        listStates();
#endif
    }

    void BfVM::stop() {
        qDebug() << "BfVM::stop()";
        m_runTimer->stop();
        emit running(false);
#ifndef QT_NO_DEBUG
        listStates();
#endif
    }


    void BfVM::reset() {
        qDebug("BfVM::reset()");
        m_DP = 0;
        m_IP = 0;
        clearMemory();
        m_inputBuffer->clear();
        emit resetted();
#ifndef QT_NO_DEBUG
        listStates();
#endif
    }

    void BfVM::clear() {
        qDebug("BfVM::clear()");
        reset();
        //m_program->clear();
        if(m_program != NULL) {
            delete[] m_program;
            m_program = NULL;
        }
        emit cleared();
#ifndef QT_NO_DEBUG
        listStates();
#endif
    }

    void BfVM::changeDelay(int newDelay) {
        qDebug() << "BfVM::changeDelay() new delay" << newDelay;
        m_runDelay = newDelay;
        m_runTimer->setInterval(m_runDelay);
    }


    void BfVM::initialize(const QList<BfOpcode> &bfSource) {
        doinit(bfSource);
    }



    void BfVM::input(const QString &in) {
        qDebug("BfVM::input()");
        // is the buffer currently empty?
        bool wasBufEmpty = m_inputBuffer->isEmpty();

        if(in.size() > 0) {
            m_inputBuffer->clear();
            const QByteArray ba = in.toAscii();
            const char *inp = ba.data();
            while (*inp) {  // QByteArrays are null-terminated, so this should work
                m_inputBuffer->enqueue(*inp);
                ++inp;
            }
        }
        /* if the buffer was empty, post an event notifying that it was filled.
           There is no need to check what state the VM is in since this event is ignored
           by all but the pertinent states */
        if(wasBufEmpty)
            m_stateMachine->postEvent(new InputBufferFilledEvent());
    }

    void BfVM::setBreakpoint(IPType pos) {
        m_breakpoints->append(pos);
    }

    ////////////////////////////////////////////////////////////////////////////////////////
    //// PUBLIC FUNCTIONS
    /////////////////////

    // *crickets chirping*


    ////////////////////////////////////////////////////////////////////////////////////////
    //// PROTECTED FUNCTIONS
    ////////////////////////

    void BfVM::listStates() const{
        qDebug("BfVM::listStates() VM in states:");
        foreach(const QAbstractState* s, m_stateMachine->configuration()) {
            qDebug() << s->property("statename").toString();
        }

    }

    // QThread's run()
    void BfVM::run() {
        qDebug("BfVM::run() VM thread running");
    }

    void BfVM::doinit(const QList<BfOpcode> &opc) {
        emit resetSig();
        m_programSize = opc.size();

        //m_program->clear();
        //m_program->append(opc);
        if(m_program != NULL) {
            delete[] m_program;
            m_program = NULL;
        }

        m_program = new BfOpcode[m_programSize];
        for(int i = 0; i < m_programSize; ++i) {
            m_program[i] = opc[i];
        }

        memoizeJumps();



        // tell the state machine that initialization is done
        m_stateMachine->postEvent(new InitedEvent);
        qDebug() << "BfVM::doinit(QList);";
#ifndef QT_NO_DEBUG
        listStates();
#endif
        // DEBUGGING: input(tr("derp"));
    }

    void BfVM::runInstruction(const BfOpcode &op) {
        qDebug("BfVM::runInstruction()");
        qDebug("IP=%d\t%s\tDP=%d (%d)",m_IP,OPCODENAMES[op],m_DP,m_memory[m_DP]);
        switch(op) {
        case(BRK): // breakpoint, yay
            emit breakpoint(m_IP, m_DP);
            m_stateMachine->postEvent(new BreakpointEvent);
            ++m_IP;
            break;
        case(DPINC):  // ++DP
            /* there's no need to check for overflows here or in SUBDP since it's desireable
               that the DP roll over when reaching either end */
            emit DPChanged(++m_DP);
            ++m_IP;
            break;

        case(DPDEC): // --DP
            emit DPChanged(--m_DP);
            ++m_IP;
            break;

        case(ADD): // ++*DP
            // Again no overflow checking since it's OK to overflow
            emit memChanged(m_DP, ++m_memory[m_DP]);
            ++m_IP;
            break;

        case(SUB): // --*DP
            emit memChanged(m_DP, --m_memory[m_DP]);
            ++m_IP;
            break;

        case(JZ): // Jump to command AFTER matching JNZ if DP points to 0. NOP if *DP!=0
            if(m_memory[m_DP] == 0) {
                qDebug() << "JZ: *DP==0, set IP to"<<m_jmps->value(m_IP)+1;
                m_IP = m_jmps->value(m_IP)+1; // AFTER the matching JNZ!
                break;
            }
            ++m_IP;
            break;

        case(JNZ): // Jump to command AFTER matching JZ if DP points to 0. NOP if *DP==0
            if(m_memory[m_DP] != 0) {
                qDebug() << "JNZ: *DP!=0, set IP to"<<m_jmps->key(m_IP)+1;
                m_IP = m_jmps->key(m_IP)+1; // AFTER the matching JZ!
                break;
            }
            ++m_IP;
            break;
        case(OUT):
            emit output(m_memory[m_DP]);
            qDebug() << "BfVM::runInstruction() output:"<<m_memory[m_DP];
            ++m_IP;
            break;

        case(INP):
            /* this is a tad complicated: if there is no data in the input buffer, the
               function checkInputBuffer() will post an event to the state machine. In this
               case, the IP should NOT be advanced since we want the VM to continue from
               this exact same point after the input buffer has been filled and we can
               *actually* do the INP command.

               In others words, if there's nothing in the input buffer we just leave
               the handling of the situation to the state machine.*/

            if(checkInputBuffer()) {
                m_memory[m_DP] = getInput();
                qDebug("INP read %d",m_memory[m_DP]);
                ++m_IP;
#ifndef QT_NO_DEBUG
        listStates();
#endif
            } else {
                qDebug("INP input buffer empty");
#ifndef QT_NO_DEBUG
        listStates();
#endif
            }
            break;
        default:
            qDebug() << "WEIRD INSTRUCTION FOUND:"<<QString::number(op);
            throw std::runtime_error("VM got a bad instruction");
        }
        qDebug("New IP=%d DP=%d (%d)",m_IP,m_DP,m_memory[m_DP]);
    }

    void BfVM::clearMemory() {
        // note the use of <= to actually clear the memory UP to the last address...
        for(int i = 0; i <= MAX_MEM_ADDR;++i) {
            m_memory[i] = 0;
        }
    }


    Memtype BfVM::getInput() {
        Q_ASSERT_X(!m_inputBuffer->isEmpty(), "BfVM::getInput()", "input buffer empty");
        emit inputConsumed();
        return m_inputBuffer->dequeue();
    }

    bool BfVM::checkInputBuffer() {
        qDebug("BfVM::checkInputBuffer() buffer size %d",m_inputBuffer->size());
        // If the input buffer is empty, post an event to the state machine
        if(m_inputBuffer->isEmpty()) {
            m_stateMachine->postEvent(new InputBufferEmptyEvent);
            return false;
        }

        return true;
    }


    /////////////////////////////////////////////////////////////////////////////////////////
    /////// REMOVE THIS AND REPLACE IT BY USING BfCompiler'S EQUIVALENT FUNCTIONALITY ///////
    /////////////////////////////////////////////////////////////////////////////////////////
    /**
      Memoizes destinations for the JZ and JNZ operators. Assumes that source code
      has been compiled correctly and all JZ/JNZ operators are balanced.
      */
    void BfVM::memoizeJumps() {
        QStack<IPType> jzs;
        /* scan program, push location of JZs encountered on stack. When a
           JNZ is encountered, pop a location from the stack and add the popped JZ location
           and the IP of the JNZ to the m_jmps BiHash.*/
        for(int i = 0; i < m_programSize; ++i) {
            if(m_program[i] == JZ) {
                qDebug() << "BfVM::memoizeJumps() JZ at"<<i;
                jzs.push(i);
                continue;
            }

            if(m_program[i] == JNZ) {
                qDebug() << "BfVM::memoizeJumps() JNZ at"<<i;
                Q_ASSERT_X(!jzs.isEmpty(), "BfVM::memoizeJumps()",
                           "JZ stack empty but found a JNZ");
                m_jmps->insert(jzs.pop(), i);
            }
        }
        qDebug() << "BfVM: (JZ,JNZ)" << m_jmps->lhash();
        Q_ASSERT_X(jzs.isEmpty(), "BfVM::memoizeJumps()",
                   "JZ stack NOT empty after scanning whole source.");
    }



}
