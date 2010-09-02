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
#ifndef BFHIGHLIGHTER_H
#define BFHIGHLIGHTER_H

#include <QSyntaxHighlighter>
namespace QtBrain {
    class BfHighlighter : public QSyntaxHighlighter
    {
        Q_OBJECT
    public:
        BfHighlighter(QTextDocument *parent = 0);
    protected:
        void highlightBlock(const QString &text);

        /* uses recursion to highlight [] blocks in the source. See the .cpp for
           explanations.
           Basically it paints from the given position index to the next [ or ] character,
           using a color dependent on the depth of the current [] block.
        */
        void recurHighlighter(const QString&, int depth, const int index);


    private:
        // The base color of the [ ] blocks.
        QColor m_blockColor;
        // How much the color is made lighter/darker at each step.
        const int m_colorStep;
/* These are commented out since block highlighting broke all other highlighting functionality,
   despite what the Qt documents say about QTextCharFormats and QSyntaxHighlighter...

        struct HighlightRule {
            QRegExp pattern;
            QTextCharFormat format;
        };

        QList<HighlightRule> m_rules;

        QRegExp m_jmpsExp;

        QTextCharFormat m_addSubFormat;
        QTextCharFormat m_dpFormat;
        QTextCharFormat m_ioFormat;
        QTextCharFormat m_jmpFormat;
*/

        QRegExp m_startBlockExp;
        QRegExp m_endBlockExp;
        QTextCharFormat m_blockFormat;

        // convenience funtion to return a proper QColor depending on QTextBlock state
        QColor getColorByState(int state);

        /* returns the smallest of the two, unless one is negative in which case
           the nonnegative one is returned. If both are negative, -1 is returned */
        int minNotNeg(int a, int b);

    };
}
#endif // BFHIGHLIGHTER_H
