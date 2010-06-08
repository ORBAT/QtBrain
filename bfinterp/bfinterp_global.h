#ifndef BFINTERP_GLOBAL_H
#define BFINTERP_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(BFINTERP_LIBRARY)
#  define BFINTERPSHARED_EXPORT Q_DECL_EXPORT
#else
#  define BFINTERPSHARED_EXPORT Q_DECL_IMPORT
#endif

// typedefs and enums used globally in the interpreter
namespace QtBrain {

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
}

#endif // BFINTERP_GLOBAL_H
