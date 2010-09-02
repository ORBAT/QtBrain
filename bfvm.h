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
#ifndef BFVM_H
#define BFVM_H

#include <QThread>
#include <QList>
#include <QQueue>
#include "bihash.h"
#include "customTransitions.h"

class QStateMachine;
class QState;
class QTimer;
class QHistoryState;
namespace QtBrain {

    /**
      The Brainfuck Virtual Machine.

      Contains separate data and program memory, and the Instruction Pointer (IP) and Data
      Pointer (DP) registers.

      The VM is controlled by a QStateMachine. The QStateMachine in turn is controlled
      by either sending signals to the VM from the outside (all *Sig() signals below)
      or by two different QEvents. The signals are meant to affect user-controllable aspects
      of the VM (like whether it's running or single-stepping, resetting the state etc.)
      and the two QEvent subclasses are meant for events that are not meant to be user-
      controllable (like resetting the state when the program ends).

      (Have a look at the state chart if you need to fool around with the internals.)

      Most of the slots are meant for VM internal use since they are used to do various
      things when different states are entered and exited. Using slots meant for VM
      internal use only from outside the VM will probably result in strange
      side-effects.

      All slots meant for external use are marked as such.

      The current signal/slot situation is really confusing and obviously suboptimal.
      */


    typedef quint16 DPType; /* If you change this, remember to check if m_maxAddress
                               needs changing too */
    typedef qint8  Memtype;
    typedef quint32 IPType;


    /* these are the op codes of the BfVM bytecode and their equivalence to "normal" Bf
       INVALID is used internally in the compiler, for example. */
    enum BfOpcode {DPINC, DPDEC, ADD, SUB, OUT, INP, JZ, JNZ, BRK, INVALID};
    //               >      <    +    -    .    ,    [   ]     %

    /* names for the opcodes. The (char*) cast is used to get rid of the annoying
       "warning: deprecated conversion from string constant to ‘char*’ " compiler warning */
    static char* const OPCODENAMES[] = {(char*)"DPINC", (char*)"DPDEC", (char*)"ADD",
                                        (char*)"SUB",(char*)"OUT",(char*)"INP",(char*)"JZ",
                                        (char*)"JNZ", (char*)"BRK", (char*)"INVALID"};



    class BfVM : public QThread
    {
        Q_OBJECT
    public:
        /////////////////////////////////////////////////////////////////////////////////////
        //// PUBLIC METHODS
        ///////////////////
        explicit BfVM(QObject *parent);
        ~BfVM();

        /////////////////////////////////////////////////////////////////////////////////////
        //// PUBLIC MEMBERS
        ///////////////////
        static const DPType MAX_MEM_ADDR = 0-1;
                //(DPType)((1 << sizeof(DPType)*8)-1);
                                           /* the largest address the DP can point to.
                                              0-1 should overflow and give the largest
                                              number DPType can hold _IF_ DPType is
                                              unsigned.

                                              The alternate version that is commented out
                                              should also work as long as bytes are 8 bits
                                              wide.

                                              NOTE: If you change DPType, make sure that
                                              m_maxAddress is calculated properly */



        /////////////////////////////////////////////////////////////////////////////////////
        //// SIGNALS
        ////////////
        /* TODO: reduce the number of different signals. This is getting ridiculous.
           Combine signals by using default arguments, maybe? */
    signals:

        void DPChanged(DPType);              /* emitted when the DP changes */

        void memChanged(DPType, Memtype);    /* emitted when memory changes. */

        void heartBeat(const IPType&);       /* emitted whenever a step() is done. The IP
                                               is sent as an argument */


        void output(const Memtype&);        /* the OUT command emits data with this */

        void needInput();                   /* emitted when the input buffer is empty */


        void running(bool);                 /* emitted when changing running states*/

        void finish();                      /* emitted when the program has finished
                                               executing. note: finishED() is used by
                                               QThread. Don't fool with that one */

        void inited();                      /* emitted when the VM has been initialized,
                                              ie. a program has been loaded. */
        void resetted();                    /* emitted when the VM has been reset */
        void cleared();                     /* emitted when the VM has been cleared */

        void inputConsumed();               /* emitted whenever the VM consumes from the
                                               input buffer */

        void breakpoint(IPType, DPType);    /* emitted when a breakpoint is reached */


        /////////////////////////////////////////////////////////////////////////////////////
        //// STATE CONTROL SIGNALS.
        ///////////////////////////
        /* use these to control the state of the VM. Naturally not all signals are
           meaningful in all states */
        void resetSig();                    /* emitted to cause the VM to reset */
        void stepSig();                     /* emitted to cause the VM to step */
        void toggleRunSig();                /* emitted to toggle the run state on or off */
        void clearSig();                    /* emitted to clear the VM state and start over*/


    protected:

        /////////////////////////////////////////////////////////////////////////////////////
        //// PROTECTED MEMBER VARIABLES
        ///////////////////////////////
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

        QList<IPType>      *m_breakpoints;  /* a list of breakpoints */



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
        //// PROTECTED METHODS
        //////////////////////

        void listStates() const;

        void run();                       // QThread


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
        //// SLOTS FOR INTERNAL USE
        ///////////////////////////
    protected slots:
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

        // TEST. Ignore.
        void breakPtTest();


        /////////////////////////////////////////////////////////////////////////////////////
        //// SLOTS FOR EXTERNAL USE
        ///////////////////////////
        void changeDelay(int);           /* changes the delay between steps when running */


        void initialize(const QList<BfOpcode>&);

        void input(const QString &in);  /* sets the input buffer contents.
                                           Currently RESETS the input buffer contents
                                           instead of appending. FIXME... */

        void setBreakpoint(IPType pos); /* sets a breakpoint at the specified IP. The
                                           breakpoint will be triggered when the IP ==
                                           pos, but before the command at that IP is
                                           executed */



    };
}
#endif // BFVM_H
