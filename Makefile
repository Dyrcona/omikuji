# Copyright © 2012 Jason J.A. Stephenson <jason@sigio.com>
#
# This file is part of Omikuji.
#
# Omikuji is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Omikuji is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Omikuji.  If not, see <http://www.gnu.org/licenses/>.

# Variables to define our package and setup a tarball distribution:
package = omikuji
version = 0.0
tarname = $(package)
distdir = $(tarname)-$(version)

# Variables for installation:
prefix = /usr/local
exec_prefix = $(prefix)
bindir = $(exec_prefix)/bin
datarootdir = $(prefix)/share
mandir = $(datarootdir)/man
man1dir = $(mandir)/man1
man6dir = $(mandir)/man6

# Settings needed for our sources:
CFLAGS += -std=c99
CPPFLAGS += -I include

# Needed for Glib-2.0:
CFLAGS += $(shell pkg-config glib-2.0 --cflags)
LDLIBS += $(shell pkg-config glib-2.0 --libs)

# Used for debugging
ifdef DEBUG
CFLAGS += -g
else
.INTERMEDIATE: omifile.o omikuji.o omikujidata.o
endif

VPATH = source include

.PHONY: all clean dist FORCE install uninstall

all: omifile omikuji

omikujidata.o: omikujidata.h omikujidata.c

omifile.o: omikujidata.h omifile.c

omikuji.o: omikujidata.h omikuji.c

omifile: omifile.o omikujidata.o
	$(LINK.c) $^ $(LOADLIBES) -o $@ $(LDLIBS)

omikuji: omikuji.o omikujidata.o
	$(LINK.c) $^ $(LOADLIBES) -o $@ $(LDLIBS)

clean:
	-rm omifile
	-rm omikuji
	-rm *.o

dist: $(distdir).tar.gz

$(distdir).tar.gz: $(distdir)
	tar cho $(distdir) | gzip -9 -c > $@
	rm -rf $(distdir)

$(distdir): FORCE
	mkdir -p $(distdir)/source
	mkdir $(distdir)/include
	mkdir $(distdir)/man
	cp source/* $(distdir)/source
	cp include/* $(distdir)/include
	cp man/* $(distdir)/man
	cp COPYING $(distdir)
	cp Format.rtf $(distdir)
	cp Makefile $(distdir)
	cp README $(distdir)

FORCE:
	-rm -f $(distdir).tar.gz > /dev/null 2>&1
	-rm -rf $(distdir) > /dev/null 2>&1

install: all
	install -d $(DESTDIR)$(bindir)
	install -m 0755 omikuji $(DESTDIR)$(bindir)
	install -m 0755 omifile $(DESTDIR)$(bindir)
	install -d $(DESTDIR)$(man1dir)
	install -m 0644 man/omifile.1 $(DESTDIR)$(man1dir)
	install -d $(DESTDIR)$(man6dir)
	install -m 0644 man/omikuji.6 $(DESTDIR)$(man6dir)

uninstall:
	-rm $(DESTDIR)$(bindir)/omikuji
	-rm $(DESTDIR)$(bindir)/omifile
	-rm $(DESTDIR)$(man1dir)/omifile.1
	-rm $(DESTDIR)$(man6dir)/omikuji.6
