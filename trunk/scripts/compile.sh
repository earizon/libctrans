 #!/bin/sh -x

INC="../inc"
SRC="../src"
TST="../tests"
TARGET="../target"

DEBUGOPTS="-ggdb"

PKGCONFIG0=" $(pkg-config --cflags glib-2.0)  $(pkg-config --libs glib-2.0) -I ${INC}"
PKGCONFIG1=" ${PKGCONFIG0} $(pkg-config --cflags gtk+-2.0)  $(pkg-config --libs gtk+-2.0)"


gcc ${DEBUGOPTS}  ${PKGCONFIG0} -c -fPIC ${SRC}/libctrans.c -o ${TARGET}/libctrans.o 
gcc -shared -Wl,-soname,libctrans.so.1 -o ${TARGET}/libctrans.so.1   ${TARGET}/libctrans.o

GCCOPTS="-o dynamically_linked"

gcc ${DEBUGOPTS} ${GCCOPTS} ${PKGCONFIG0} ${TARGET}/libctrans.so.1 ${TST}/test1.c -o ${TARGET}/test1 2>&1 
gcc ${DEBUGOPTS} ${GCCOPTS} ${PKGCONFIG0} ${TARGET}/libctrans.so.1 ${TST}/test2.c -o ${TARGET}/test2 2>&1 
gcc ${DEBUGOPTS} ${GCCOPTS} ${PKGCONFIG1} ${TARGET}/libctrans.so.1 ${TST}/test3.c -o ${TARGET}/test3 2>&1 
