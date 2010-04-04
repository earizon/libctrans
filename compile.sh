 #!/bin/sh

DEBUGOPTS="-ggdb"

PKGCONFIG0=" $(pkg-config --cflags glib-2.0)  $(pkg-config --libs glib-2.0)"
PKGCONFIG1=" ${PKGCONFIG0} $(pkg-config --cflags gtk+-2.0)  $(pkg-config --libs gtk+-2.0)"
gcc ${DEBUGOPTS}  ${PKGCONFIG0} -c -fPIC libctrans.c -o libctrans.o 
gcc -shared -Wl,-soname,libctrans.so.1 -o libctrans.so.1.0.0  libctrans.o

GCCOPTS="-o dynamically_linked"

gcc ${DEBUGOPTS} ${GCCOPTS} ${PKGCONFIG0} ./libctrans.so.1.0.0 test1.c -o test1 2>&1 
gcc ${DEBUGOPTS} ${GCCOPTS} ${PKGCONFIG0} ./libctrans.so.1.0.0 test2.c -o test2 2>&1 

gcc ${DEBUGOPTS} ${GCCOPTS} ${PKGCONFIG1} ./libctrans.so.1.0.0 test3.c -o test3 2>&1 

