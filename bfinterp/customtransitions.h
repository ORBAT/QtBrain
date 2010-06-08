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
#ifndef CUSTOMTRANSITIONS_H
#define CUSTOMTRANSITIONS_H
#include <QEvent>
#include <QAbstractTransition>
#include <QDebug>




/** Some custom events and transitions for the BfVM */




#define INITED_EVENT QEvent::Type(QEvent::User+1)
#define END_EVENT QEvent::Type(QEvent::User+2)
#define INP_BUFFER_FILLED_EVENT QEvent::Type(QEvent::User+3)
#define INP_BUFFER_EMPTY_EVENT QEvent::Type(QEvent::User+4)
#define BREAKPOINT_EVENT QEvent::Type(QEvent::User+5)

namespace QtBrain {

    /**
  Custom Event used to trigger transition between run states and the finished state
  */
    struct EndEvent : public QEvent {
        EndEvent() : QEvent(END_EVENT) {}
    };

    /**
  Custom Event used to trigger transition between empty and initialized states
  */
    struct InitedEvent : public QEvent {
        InitedEvent() : QEvent(INITED_EVENT) {}
    };

    /**
  Custom Event used to trigger transition between run states and the waiting for input
  state
  */
    struct InputBufferEmptyEvent : public QEvent {
        InputBufferEmptyEvent() : QEvent(INP_BUFFER_EMPTY_EVENT) {}
    };

    /**
  Custom Event used to trigger transition between the waiting for input state and the
  previous run state
  */
    struct InputBufferFilledEvent : public QEvent {
        InputBufferFilledEvent() : QEvent(INP_BUFFER_FILLED_EVENT) {}
    };

    /**
    Custom Event used to go to the breakpoint state
    */
      struct BreakpointEvent : public QEvent {
          BreakpointEvent() : QEvent(BREAKPOINT_EVENT) {}
      };

    /**
  Custom transition that responds to the EndEvent
  */
    class EndEventTransition : public QAbstractTransition {
    protected:
        virtual bool eventTest(QEvent *e) {
            if(e->type() == END_EVENT) {
                qDebug("BfVM EndEvent received");
                return true;
            }
            return false;
        }
        virtual void onTransition(QEvent*) {}
    };

    /**
  Custom transition that responds to the InitedEvent
  */
    class InitedEventTransition : public QAbstractTransition {
    protected:
        virtual bool eventTest(QEvent *e) {
            if(e->type() == INITED_EVENT) {
                qDebug("BfVM InitedEvent received");
                return true;
            }
            return false;
        }
        virtual void onTransition(QEvent*) {}
    };

    /**
  Custom transition that responds to the InputBufferEmptyEvent
  */
    class InpBufEmptyTransition : public QAbstractTransition {
    protected:
        virtual bool eventTest(QEvent *e) {
            if(e->type() == INP_BUFFER_EMPTY_EVENT) {
                qDebug("BfVM BufEmptyTransition received");
                return true;
            }
            return false;
        }
        virtual void onTransition(QEvent*) {}
    };

    /**
  Custom transition that responds to the InputBufferFilledEvent
  */
    class InpBufFilledTransition : public QAbstractTransition {
    protected:
        virtual bool eventTest(QEvent *e) {
            if(e->type() == INP_BUFFER_FILLED_EVENT) {
                qDebug("BfVM BufFilledTransition received");
                return true;
            }
            return false;
        }
        virtual void onTransition(QEvent*) {}
    };

    /**
  Custom transition that responds to the BreakpointEvent
  */
    class BreakpointTransition : public QAbstractTransition {
    protected:
        virtual bool eventTest(QEvent *e) {
            if(e->type() == BREAKPOINT_EVENT) {
                qDebug("BfVM BreakpointEvent received");
                return true;
            }
            return false;
        }
        virtual void onTransition(QEvent*) {}
    };


}


#endif // CUSTOMTRANSITIONS_H
