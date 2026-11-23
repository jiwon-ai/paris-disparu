# PARIS DISPARU - Game Boy Color build (GBDK-2020)
#
#   make            build build/paris-disparu.gbc
#   make assets     regenerate src/assets.c + include/assets.h (needs python3)
#   make check      validate assets and the built ROM header
#   make clean
#
# GBDK_HOME must point at a GBDK-2020 install (tested with 4.3.0).

GBDK_HOME ?= /opt/gbdk/
LCC       = $(GBDK_HOME)/bin/lcc
PYTHON   ?= python3

ROM       = build/paris-disparu.gbc

# -Wm-yC        : CGB-only cartridge (header 0x143 = 0xC0)
# -Wm-yt0x19    : MBC5 cartridge type, ready for ROM banking later
# -Wm-yn        : header title
# -Wm-yk / -yl  : new licensee code "00" and old code 0x33 (no publisher)
LCCFLAGS  = -Iinclude -Wa-l -Wl-m -Wl-j
ROMFLAGS  = -Wm-yC -Wm-yt0x19 -Wm-yn"PARISDISPARU" -Wm-yk00 -Wm-yl0x33

SRC       = $(wildcard src/*.c)
OBJ       = $(patsubst src/%.c,build/obj/%.o,$(SRC))
HDR       = $(wildcard include/*.h)

all: $(ROM)

$(ROM): $(OBJ)
	$(LCC) $(LCCFLAGS) $(ROMFLAGS) -o $@ $(OBJ)

build/obj/%.o: src/%.c $(HDR) | build/obj
	$(LCC) $(LCCFLAGS) -c -o $@ $<

build/obj:
	mkdir -p $@

assets:
	$(PYTHON) tools/gen_assets.py

check: $(ROM)
	$(PYTHON) tools/gen_assets.py --check
	$(PYTHON) tests/check_rom.py $(ROM)

clean:
	rm -rf build

.PHONY: all assets check clean
