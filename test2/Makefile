
include ../cdebug/_install/lib/cmake/libcdebug/libcdebug.mk
LDFLAGS += ${LIB_CDEBUG_LDFLAGS} -L ../cdebug/_install/lib/
CFLAGS += ${LIB_CDEBUG_CFLAGS}

all:
	${CC} ./test-trace.c ${CFLAGS} ${LDFLAGS} -o test

