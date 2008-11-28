PROG=tudu
SRCS=tudu.cc interface.cc window.cc data.cc parser.cc text.cc config.cc date.cc
SRCS+= editor.cc screen.cc sched.cc
CXX=g++
CFLAGS=-Wall -O2 -DSHARE_DIR=\"$(SHARE_DIR)\" -DETC_DIR=\"$(ETC_DIR)\"

OBJS=$(SRCS:.cc=.o)

all: $(PROG)

$(PROG): $(OBJS)
	echo $(SRCS)
	$(CXX) $(CFLAGS) -lncurses -o $@ $(OBJS)

.cc.o:
	$(CXX) $(CFLAGS) -c $<

clean:
	@rm -f $(PROG) $(OBJS) core tags

depend:
	@makedepend -Y $(SRCS) 2>/dev/null
	@rm -f Makefile.bak
