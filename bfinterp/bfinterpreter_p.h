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
#ifndef BFINTERPRETER_P_H
#define BFINTERPRETER_P_H

#include <QObject>
#include <QList>
#include <QQueue>
#include "bfinterp_global.h"
#include "bihash.h"


class QState;
class QStateMachine;
class QTimer;
class QHistoryState;

namespace QtBrain {
    class BfInterpreter;

    class BFINTERPSHARED_EXPORT BfInterpreterPrivate : public QObject  // Is the library macro necessary here?
    {
        Q_OBJECT;
        ////////////////////////////////////////////////////////////////////////////////////////
        //// MEMBER VARIABLES
        /////////////////////
    public:
        explicit BfInterpreterPrivate(BfInterpreter * qq);
        ~BfInterpreterPrivate();

        BfInterpreter * const q_ptr;  // pointer to the public object
        Q_DECLARE_PUBLIC(BfInterpreter);


        DPType             m_DP;            // the Data Pointer. Points to memory

        IPType             m_IP;            /* Instruction Pointer. Points to the
                                            command being executed */

        DPType             m_programSize;   /* the size of the Brainfuck program currently
                                            loaded */





        qint32             m_runDelay;      /* the delay between steps when running a
                                            Brainfuck program. Defaults to 500ms*/

        Memtype            *m_memory;       /* the memory array. Needs to be the same size
                                            as the DP, ie. with a 16 bit DP you can
                                            only have 65536 memory locations.*/


        /////////////////////////////////////////////////////////////////////////////////////
        //////// REPLACE THIS WITH BfCompiler'S FUNCTIONALITY ///////////////////////////////
        /////////////////////////////////////////////////////////////////////////////////////
        //      v JZ IP
        BiHash<IPType,IPType> *m_jmps;      /* contains all JZ/JNZ addresses so
                       ^ JNZ IP             jump instructions don't have to look up
                                            their destination each time. */
        /////////////////////////////////////////////////////////////////////////////////////

        BfOpcode           *m_program;      /* the array that contains the Brainfuck
                                            program */

        QTimer             *m_runTimer;     /* times the delay between steps when
                                            running a Bf program */

        QQueue<Memtype>    *m_inputBuffer;  /* buffer for input characters. The INP
                                               command reads from this, and if it is empty
                                               a needInput() signal is emitted.
                                               Input data can be given with the input()
                                               slot */

        //QList<IPType>      *m_breakpoints;  /* a list of breakpoints */

        bool               m_debugging;     /* if this is true, then the interpreter emits
                                               debugging-related data to the GUI */







        /////////////////////////////////////////////////////////////////////////////////////
        //// STATES
        ///////////
        QStateMachine      *m_stateMachine;
        QState             *m_stateGroup;   /* the group state for all states */
        QState             *m_runGroup;     // for stepping and running states

        QState             *m_emptySt;      // no program is loaded in the VM
        QState             *m_initializedSt;// when a program is loaded, but not running yet
        QState             *m_steppingSt;   // single-step state
        QState             *m_runningSt;    // continuous run
        QState             *m_finishedSt;   /* when a program has finished running. This
                                               state will just wait for the user to
                                               reset or clear the VM */
        QState             *m_waitingForInpSt;
                                            /* if the input buffer is empty, the VM
                                               enters this state. The only way to get
                                               out of this state is to either reset or
                                               clear the VM, or give it input with the
                                               input() slot */
        QState             *m_breakpointSt; /* when a breakpoint is reached */
        QHistoryState      *m_runHistorySt; /* history state used to return to the proper
                                               state in the run group when coming back
                                               from m_waitingForInpSt */

        QState             *m_clearSt;      // clearing the VM (reset all variables etc)


        //////////////////////////////////////////////////////////////////////////////////
        //// INTERNAL METHODS
        /////////////////////

        void listStates() const;

        void memoizeJumps();              /* goes through the program and stores all
                                           origins and destinations of JMPx instructions */

        void doinit(const QList<BfOpcode>&); /* loads a Brainfuck program into memory and
                                                gets the VM into the right state */


        void initializeStateMachine();    // initializes the VM's state machine

        void runInstruction(const BfOpcode&);
                                          // executes Brainfuck in the form of BfOpcodes
        void clearMemory();               // zeroes all memory positions

        bool checkInputBuffer();             /* checks if the input buffer is empty. If not,
                                             returns true. If it is, returns false and
                                             posts an InputBufferEmptyEvent to the state
                                             machine */
        Memtype getInput();               /* dequeues one character from the input buffer.
                                             Also emits a inputConsumed() signal */


        /////////////////////////////////////////////////////////////////////////////////////
        //// "PUBLIC" METHODS
        /////////////////////
        void changeDelay(const int&);   /* changes the delay between steps when running */

        void input(const QString &in);  /* sets the input buffer contents.
                                           Currently RESETS the input buffer contents
                                           instead of appending. FIXME... */

        /////////////////////////////////////////////////////////////////////////////////////
        //// SLOTS FOR INTERNAL USE
        ///////////////////////////
    public slots:
        void step();        /* runs the instruction pointed to by the IP and then increments
                            the IP */

        void go();          /* puts the state machine in the running state.
                               Starts a timer that repeatedly triggers step()*/

        void stop();        /* puts the state machine into the stepping state.
                               Stops any timers started by go();*/

        void reset();       /* resets the state of the VM. Memory is zeroed and the DP and IP
                            are set to 0 and the machine is stopped */

        void clear();       /* clears the VM. Clears program memory and then performs
                            a reset() */

    };
}
#endif // BFINTERPRETER_P_H
