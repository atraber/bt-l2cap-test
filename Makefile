CC            = gcc
CFLAGS        = -Wall -c
LDFLAGS       = -lbluetooth

all: l2cap-client.out l2cap-server.out

l2cap-client.o: l2cap-client.c
	${CC} $^ ${CFLAGS} -o $@

l2cap-server.o: l2cap-server.c
	${CC} $^ ${CFLAGS} -o $@

l2cap-client.out: l2cap-client.o
	${CC} $^ ${LDFLAGS} -o $@

l2cap-server.out: l2cap-server.o
	${CC} $^ ${LDFLAGS} -o $@
