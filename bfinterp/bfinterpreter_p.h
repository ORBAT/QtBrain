#ifndef BFINTERPRETER_P_H
#define BFINTERPRETER_P_H
#include "bfinterp_global.h"
namespace QtBrain {
    class BfInterpreter;

    class BFINTERPSHARED_EXPORT BfInterpreterPrivate
    {
    public:
        explicit BfInterpreterPrivate(BfInterpreter * const qq);
        ~BfInterpreterPrivate();

        BfInterpreter * const q_ptr;
        Q_DECLARE_PUBLIC(BfInterpreter);
    };
}
#endif // BFINTERPRETER_P_H
