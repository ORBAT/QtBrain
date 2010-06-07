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
#ifndef BFINTERPRETER_H
#define BFINTERPRETER_H

#include <QThread>
#include "bfinterp_global.h"

class QStateMachine;
class QState;
class QTimer;
class QHistoryState;

namespace QtBrain {


    /////////// TODO: Fix the comments
    /**
      The Brainfuck interpreter (BfI)

      Contains separate data and program memory, and the Instruction Pointer (IP) and Data
      Pointer (DP) registers.

      The interpreter is controlled by a QStateMachine. The QStateMachine in turn is controlled
      by either sending signals to the interpreter from the outside (all *Sig() signals below)
      or by two different QEvents. The signals are meant to affect user-controllable aspects
      of the interpreter (like whether it's running or single-stepping, resetting the state etc.)
      and the two QEvent subclasses are meant for events that are not meant to be user-
      controllable (like resetting the state when the program ends).

      (Have a look at the state chart if you need to fool around with the internals.)

      Most of the slots are meant for interpreter internal use since they are used to do various
      things when different states are entered and exited. Using slots meant for interpreter
      internal use only from outside the interpreter will probably result in strange
      side-effects.

      All slots meant for external use are marked as such.

      The current signal/slot situation is really confusing and obviously suboptimal.
      */

    typedef quint16 DPType; /* If you change this, remember to check if m_maxAddress
                               needs changing too */
    typedef qint8  Memtype;
    typedef quint32 IPType;

    /* these are the op codes of the BfI bytecode and their equivalence to "normal" Bf
       INVALID is used internally in the compiler, for example. */
    enum BfOpcode {DPINC, DPDEC, ADD, SUB, OUT, INP, JZ, JNZ, BRK, INVALID};
    //               >      <    +    -    .    ,    [   ]     %

    /* names for the opcodes. The (char*) cast is used to get rid of the annoying
       "warning: deprecated conversion from string constant to ‘char*’ " compiler warning */
    static char* const OPCODENAMES[] = {(char*)"DPINC", (char*)"DPDEC", (char*)"ADD",
                                        (char*)"SUB",(char*)"OUT",(char*)"INP",(char*)"JZ",
                                        (char*)"JNZ", (char*)"BRK", (char*)"INVALID"};

    class BFINTERPSHARED_EXPORT BfInterpreter : public QThread {
        Q_OBJECT
        /////////////////////////////////////////////////////////////////////////////////////
        //// PUBLIC METHODS
        ///////////////////
    public:
        explicit BfInterpreter(QObject *parent);
        virtual ~BfInterpreter();

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

    };

}
#endif // BFINTERPRETER_H
