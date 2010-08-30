DESTDIR=/usr/local
BIN_DIR=$(DESTDIR)/bin
ETC_DIR=$(DESTDIR)/etc
SHARE_DIR=$(DESTDIR)/share/tudu
MAN_DIR=$(DESTDIR)/share/man

all:
	$(MAKE) -C src all SHARE_DIR=$(SHARE_DIR) ETC_DIR=$(ETC_DIR)

clean:
	$(MAKE) -C src clean

depend:
	$(MAKE) -C src depend

install: all
	mkdir -p $(BIN_DIR)
	install -m755 src/tudu $(BIN_DIR)
	mkdir -p $(SHARE_DIR)
	install -m644 data/tudu.dtd $(SHARE_DIR)
	install -m644 data/welcome.xml $(SHARE_DIR)
	mkdir -p $(ETC_DIR)
	install -m644 data/tudurc $(ETC_DIR)
	mkdir -p $(MAN_DIR)/man1
	install -m644 data/tudu.1 $(MAN_DIR)/man1

uninstall:
	rm $(BIN_DIR)/tudu
	rm $(SHARE_DIR)/tudu.dtd
	rm $(SHARE_DIR)/welcome.xml
	rm $(MAN_DIR)/man1/tudu.1
	rmdir $(SHARE_DIR)
