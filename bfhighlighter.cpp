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
#include "bfhighlighter.h"
#include "bihash.h"
#include <QDebug>
#include <algorithm>


namespace QtBrain {


    BfHighlighter::BfHighlighter(QTextDocument *parent) :
            QSyntaxHighlighter(parent),
            m_blockColor(Qt::white),
            m_colorStep(63) // 255/63 gives 4 levels of indentation before the color "rolls over"
    {
        //HighlightRule rule;

        m_startBlockExp.setPattern(tr("\\["));
        m_endBlockExp.setPattern(tr("\\]"));

        // the default background format
        m_blockFormat.setBackground(m_blockColor);


        /* temporarily disabled. See .h for reason
        // how the [ ] commands should look
        m_jmpFormat.setFontWeight(QFont::Bold);
        m_jmpFormat.setForeground(Qt::green);
        m_jmpFormat.setFontPointSize(11);
        rule.pattern = QRegExp(tr("[\\[\\]]"));
        rule.format = m_jmpFormat;
        m_rules.append(rule);

        // < and >
        m_dpFormat.setForeground(Qt::blue);
        m_dpFormat.setFontPointSize(11);
        rule.pattern = QRegExp(tr("[<>]"));
        rule.format = m_dpFormat;
        m_rules.append(rule);

        // , and .
        m_ioFormat.setForeground(Qt::lightGray);
        m_ioFormat.setFontPointSize(11);
        rule.pattern = QRegExp(tr("[,.]"));
        rule.format = m_ioFormat;
        m_rules.append(rule);

        // + and -
        m_addSubFormat.setForeground(Qt::magenta);
        m_addSubFormat.setFontPointSize(11);
        rule.pattern = QRegExp(tr("[+-]"));
        rule.format = m_addSubFormat;
        m_rules.append(rule);
*/
        qDebug() << "BfHighlighter::BfHighlighter()";
    }


    void BfHighlighter::highlightBlock(const QString &text) {
        int currState = previousBlockState();
        setCurrentBlockState(currState);
        qDebug("\n\n****highlightBlock() current state %d, prev %d", currState,
               previousBlockState());

        // temporarily disabled. See .h for reason
        /*
        foreach(const HighlightRule &rule, m_rules) {
            QRegExp expression(rule.pattern);
            int idx = expression.indexIn(text); // returns -1 when not found
            while(idx >= 0) {
                setFormat(idx, 1, rule.format);
                idx = expression.indexIn(text, idx+1);
            }
        }
*/

        /*
         TODO: iterative version (should be easy enough).

         I'm too used to thinking about things recursively...
         */
        recurHighlighter(text, currState, 0);

    }


    int BfHighlighter::minNotNeg(int a, int b) {

        int min = std::min(a,b);
        if(min == -1) {
            qDebug("minNotNeg() a:%d b:%d ret %d",a,b,std::max(a,b));
            return std::max(a,b);
        }
        else {
            qDebug("minNotNeg() a:%d b:%d ret %d",a,b,min);
            return min;
        }
    }

    void BfHighlighter::recurHighlighter(const QString &text, int depth, int index) {
        QTextCharFormat temp(m_blockFormat); //make a copy of the default format
        qDebug() << "\n---- text: " << text;
        if(index >= text.length()) {
            qDebug("---- bailout");
            return;
        }

        int paintIndex;
        paintIndex = index; // start painting from the index
        if(text[index] == '[') {
            depth++;

            qDebug("---- at idx %d (%c), depth %d.", index, text[index].toAscii(),
                   depth);
        } else if(text[index] == ']') {
            depth--;
            qDebug("---- at idx %d (%c), depth %d.", index, text[index].toAscii(),
                   depth);
            /* close braces themselves have to be painted with the
            previous level's color.

            Here's a nifty diagram of what's happening:

         01112222111111110111    Block color number
         v___vvvv________v___    Block color boundaries
         x[xx[xx]xxx][xx]x[x]
         _^^^___^^^^_^^^__^^_    Block depth change
         01112221111011100110    Block depth boundaries

         0111222211111111
         v___vvvv________
         x[xx[xx]xxx][xx]
         _^^^___^^^^_^^^_
         0111222111101110


         */
            if(depth <-1) {
                temp.setBackground(getColorByState(depth));
            } else {
                temp.setBackground(getColorByState(depth+1));
            }
            setFormat(index, 1, temp);
            // with close braces the painting has to start from one step after the index
            paintIndex = index+1;
        }

        setCurrentBlockState(depth);


        int closestStart = text.indexOf(m_startBlockExp, index+1);
        int closestEnd = text.indexOf(m_endBlockExp, index+1);
        int closest = minNotNeg(closestStart, closestEnd);

        if(closest == -1) {
            // no new braces on this line. Paint to the end of the line
            closest = text.length();
        }

        temp.setBackground(getColorByState(depth));
        qDebug("----  Painting %d, closest %d\n", (closest-index), closest);
        qDebug("---- block state %d, depth %d",currentBlockState(), depth);
        qDebug() << "---- color" << getColorByState(depth);
        setFormat(paintIndex,(closest-index), temp);
        recurHighlighter(text,depth,closest);
    }

    inline QColor BfHighlighter::getColorByState(int state) {
        if(state == -1)
            return m_blockColor;
        if(state < -1)
            return QColor(255,150,150);

        int stepAmt = (state+1)*m_colorStep;

        return QColor((quint8)(m_blockColor.red()-stepAmt),
                      230,
                      (quint8)(m_blockColor.blue()-stepAmt));
    }
}


