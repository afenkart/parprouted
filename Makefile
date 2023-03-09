EXTRA_CFLAGS = 

PREFIX = $(DESTDIR)/usr

CFLAGS = -g -O2 -Wall -Wextra $(EXTRA_CFLAGS)
CXXFLAGS = -g -O2 -std=c++17 -Wall -Wextra $(EXTRA_CFLAGS)

# For ARM:
# CFLAGS =  -Wall $(EXTRA_CFLAGS)
OBJS = src/parprouted.o src/arp.o src/main.o

LIBS = -lpthread

all: parprouted parprouted.8

install: all
	install parprouted $(PREFIX)/sbin
	install parprouted.8 $(PREFIX)/share/man/man8

clean:
	rm -f $(OBJS) parprouted core parprouted.8

parprouted:	${OBJS}
	${CXX} -g -o parprouted ${OBJS} ${CXXFLAGS} ${LDFLAGS} ${LIBS}

parprouted.8:	parprouted.pod
	pod2man --section=8 --center="Proxy ARP Bridging Daemon" parprouted.pod --release "parprouted" --date "`date '+%B %Y'`" > parprouted.8

%.o : %.cpp parprouted.h
