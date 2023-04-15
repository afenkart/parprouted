EXTRA_CFLAGS = 

PREFIX = $(DESTDIR)/usr

#CC = gcc

CFLAGS = -g -O2 -Wall $(EXTRA_CFLAGS)
CXXFLAGS = -g -O2 -std=c++17 -Wall $(EXTRA_CFLAGS)

# For ARM:
# CFLAGS =  -Wall $(EXTRA_CFLAGS)
OBJS = src/parprouted.o src/arp.o

LIBS = -lpthread

all: parprouted

install: all
	install parprouted $(PREFIX)/sbin
	install parprouted.8 $(PREFIX)/share/man/man8

clean:
	rm -f $(OBJS) parprouted core

parprouted:	${OBJS}
	${CXX} -g -o parprouted ${OBJS} ${CXXFLAGS} ${LDFLAGS} ${LIBS}

parprouted.8:	parprouted.pod
	pod2man --section=8 --center="Proxy ARP Bridging Daemon" parprouted.pod --release "parprouted" --date "`date '+%B %Y'`" > parprouted.8

parprouted.o : parprouted.cpp parprouted.h

arp.o : arp.cpp parprouted.h
