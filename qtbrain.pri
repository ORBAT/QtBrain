public_headers.path = $$PWD/inc
public_headers.files = $$PUBLIC_HEADERS
QMAKE_RPATHDIR += ../bfinterp

target.path = $$PWD/bin
INSTALLS += target public_headers
