#!/usr/bin/env python3
"""Validate the cartridge header of the built ROM.

Checks: size, Nintendo logo bytes, CGB flag, cartridge type, ROM size code,
header checksum and global checksum. Exits non-zero on any failure.
"""
import sys

# Boot ROM logo, as documented in Pan Docs ("The Cartridge Header", 0104-0133).
LOGO = bytes.fromhex(
    "CEED6666CC0D000B03730083000C000D0008111F8889000EDCCC6EE6DDDDD999"
    "BBBB67636E0EECCCDDDC999FBBB9333E")
MAX_ROM = 8 * 1024 * 1024


def main(path):
    rom = open(path, "rb").read()
    errors = []
    size = len(rom)
    if size < 0x8000 or size > MAX_ROM or size & (size - 1):
        errors.append("ROM size %d is not a power of two between 32 KiB and 8 MiB" % size)
    if rom[0x104:0x134] != LOGO:
        errors.append("Nintendo logo bytes at 0x104 are wrong")
    if rom[0x143] not in (0x80, 0xC0):
        errors.append("CGB flag 0x143 = 0x%02X (expected 0x80 or 0xC0)" % rom[0x143])
    if rom[0x147] != 0x19:
        errors.append("cartridge type 0x147 = 0x%02X (expected 0x19, MBC5)" % rom[0x147])
    if (0x8000 << rom[0x148]) != size:
        errors.append("ROM size code 0x148 = %d does not match file size %d" % (rom[0x148], size))
    hc = 0
    for b in rom[0x134:0x14D]:
        hc = (hc - b - 1) & 0xFF
    if rom[0x14D] != hc:
        errors.append("header checksum 0x%02X, expected 0x%02X" % (rom[0x14D], hc))
    gc = (sum(rom) - rom[0x14E] - rom[0x14F]) & 0xFFFF
    if (rom[0x14E] << 8 | rom[0x14F]) != gc:
        errors.append("global checksum mismatch")
    title = rom[0x134:0x143].split(b"\0")[0].decode("ascii", "replace")
    if errors:
        for e in errors:
            print("check_rom: FAIL: " + e)
        return 1
    print("check_rom: OK  %s  title=%r  size=%d KiB  cgb=0x%02X  type=0x%02X"
          % (path, title, size // 1024, rom[0x143], rom[0x147]))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1]))
