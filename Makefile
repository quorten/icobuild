X = .exe
prefix = /usr/local
exec_prefix = ${prefix}
bindir = ${exec_prefix}/bin
datadir = ${prefix}/share
docdir = ${datadir}/doc

VERSION = 1.0

icobuild_SOURCES = icobuild.c icoinfo.h
icoextract_SOURCES = icoextract.c icoinfo.h
bmptool_SOURCES = bmptool.c icoinfo.h

bin_PROGRAMS = icobuild$(X) icoextract$(X) bmptool$(X)

DISTFILES = icobuild.c icoinfo.h icoextract.c bmptool.c \
	README icons-howto.txt Makefile vga-palette.bmp

all: $(bin_PROGRAMS)

icobuild$(X): icobuild.c icoinfo.h
	gcc -mms-bitfields -o $@ $<
icoextract$(X): icoextract.c icoinfo.h
	gcc -mms-bitfields -o $@ $<
bmptool$(X): bmptool.c icoinfo.h
	gcc -mms-bitfields -o $@ $<

clean:
	rm -f $(bin_PROGRAMS)

install: $(bin_PROGRAMS)
	install $(bin_PROGRAMS) $(bindir)

uninstall:
	rm -f $(bindir)/icobuild$(X) $(bindir)/icoextract$(X) \
	  $(bindir)/bmptool$(X)

dist:
	mkdir icobuild-$(VERSION)
	cp -p $(DISTFILES) icobuild-$(VERSION)
	zip -9rq icobuild-$(VERSION).zip icobuild-$(VERSION)
	rm -rf icobuild-$(VERSION)
