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

.PHONY: all clean

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
