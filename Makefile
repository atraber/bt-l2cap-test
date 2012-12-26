CC            = gcc
CXX           = g++
LINK          = g++
CFLAGS        = -Wall -c
CXXFLAGS      = -pipe -g -Wall -W
LDFLAGS       = -lbluetooth

all: l2cap-client.out l2cap-server.out l2cap-server-send.out l2cap-client-recv.out i2c-test.out i2c-pong.out simplescan.out i2c-led.out bti2cget

l2cap-client.o: l2cap-client.c
	${CC} $^ ${CFLAGS} -o $@

l2cap-client-recv.o: l2cap-client-recv.c
	${CC} $^ ${CFLAGS} -o $@

l2cap-server.o: l2cap-server.c
	${CC} $^ ${CFLAGS} -o $@

l2cap-server-send.o: l2cap-server-send.c
	${CC} $^ ${CFLAGS} -o $@

l2cap-client.out: l2cap-client.o
	${CC} $^ ${LDFLAGS} -o $@

l2cap-client-recv.out: l2cap-client-recv.o
	${CC} $^ ${LDFLAGS} -o $@

l2cap-server.out: l2cap-server.o
	${CC} $^ ${LDFLAGS} -o $@

l2cap-server-send.out: l2cap-server-send.o
	${CC} $^ ${LDFLAGS} -o $@

i2c-test.o: i2c-test.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $^

i2c-test.out: i2c-test.o
	$(LINK) $(LDFLAGS) -o $@ $^

i2c-pong.o: i2c-pong.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $^

i2c-pong.out: i2c-pong.o
	$(LINK) $(LDFLAGS) -o $@ $^

simplescan.o: simplescan.c
	${CC} $^ ${CFLAGS} -o $@

simplescan.out: simplescan.o
	${CC} $^ ${LDFLAGS} -o $@

i2c-led.o: i2c-led.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $^

i2c-led.out: i2c-led.o
	$(LINK) $(LDFLAGS) -o $@ $^


bti2cget.o: bti2cget.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $^

bti2cget: bti2cget.o
	$(LINK) $(LDFLAGS) -o $@ $^
