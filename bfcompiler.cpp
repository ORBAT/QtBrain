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
#include "bfcompiler.h"
#include "bihash.h"
#include <QStack>
#include <QDebug>



namespace QtBrain {

    BfCompiler::BfCompiler(QObject *parent) :
            QThread(parent),
            m_error(false)
    {
    }

    BfCompiler::~BfCompiler() {
        qDebug("~BfCompiler()");
    }

    /////////////////////////////////////////////////////////////////////////////////////
    //// SLOTS FOR EXTERNAL USE
    ///////////////////////////
    void BfCompiler::compile(const QString& src) {
        m_error = false;
        BiHash<IPType, quint32> mappings;
        BiHash<IPType,IPType> jmps;

        QList<BfOpcode> bytecode = compileSource(src, mappings);

        if(bytecode.isEmpty()) {
            qDebug("BfCompiler::compile() no valid Bf in source");
            emit error(trUtf8("There were no valid Brainfuck commands in the source"),0);
            return;
        }

        jmps = memoizeJumps(bytecode, mappings);
        if(m_error) {
            qDebug("BfCompiler::compile() encountered an error in the source");
            return;
        }

        emit compiled(bytecode,jmps,mappings);

    }


    /////////////////////////////////////////////////////////////////////////////////////
    //// PROTECTED METHODS
    //////////////////////

    BfOpcode BfCompiler::charToOpcode(const QChar &chr) {
        switch(chr.toAscii()) {
        case '>':
            return DPINC;
        case '<':
            return DPDEC;
        case '+':
            return ADD;
        case '-':
            return SUB;
        case '.':
            return OUT;
        case ',':
            return INP;
        case '[':
            return JZ;
        case ']':
            return JNZ;
        case '%':
            return BRK;
        default:
            return INVALID;
        }
    }

    QList<BfOpcode> BfCompiler::compileSource(const QString &src, BiHash<IPType, quint32> &mappings) {
        QList<BfOpcode> bytecode;

        int srclen = src.size();
        BfOpcode temp;

        for(int i = 0; i < srclen ; ++i) {
            temp = charToOpcode(src[i]);
            if(temp != INVALID) { // Found a valid command at position i
                //               v position in cleaned source
                mappings.insert(bytecode.size(), i);
                //                               ^ position in original
                bytecode.append(temp);
            }
        }
        return bytecode;

    }

    BiHash<IPType,IPType> BfCompiler::memoizeJumps(const QList<BfOpcode> &program,
                                                   BiHash<IPType, quint32> &mappings) {
        QStack<IPType> jzs;
        BiHash<IPType,IPType> jmps;
        DPType programSize = program.size();
        /* scan program, push location of JZs encountered on stack. When a
           JNZ is encountered, pop a location from the stack and add the popped JZ location
           and the IP of the JNZ to the m_jmps BiHash.*/
        for(IPType i = 0; i < programSize; ++i) {
            if(program[i] == JZ) {
                qDebug() << "BfCompiler::memoizeJumps() JZ at"<<i;
                jzs.push(i);
                continue;
            }

            if(program[i] == JNZ) {
                qDebug() << "BfCompiler::memoizeJumps() JNZ at"<<i;
                if(jzs.isEmpty()) {
                    qDebug("BfCompiler::memoizeJumps() brace mismatch at %d",i);
                    emit error(trUtf8("Brace mismatch: too many ]s"), mappings.value(i));
                    m_error = true;
                    return BiHash<IPType,IPType>();
                }

                jmps.insert(jzs.pop(), i);
            }
        }

        if(!jzs.isEmpty()) {
            IPType errPos = jzs.pop();
            qDebug("BfCompiler::memoizeJumps() brace mismatch at %d", errPos);
            emit error(trUtf8("Brace mismatch: too many [s"), mappings.value(errPos));
            m_error = true;
            return BiHash<IPType,IPType>();
        }
        qDebug() << "BfCompiler: (JZ,JNZ)" << jmps.lhash();
        return jmps;
    }


    void BfCompiler::run() {
        qDebug() << "BfCompiler::run() compiler thread running";
    }
}
