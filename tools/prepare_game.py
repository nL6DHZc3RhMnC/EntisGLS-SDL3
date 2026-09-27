#!/usr/bin/env python3
"""Extract the existing main module/config for local Android compatibility tests."""
from pathlib import Path
import struct
import subprocess
import noa

ROOT = Path(__file__).resolve().parents[1]

def pe_resource(binary, wanted):
    pe = struct.unpack_from('<I', binary, 60)[0]
    optional = pe + 24
    optional_size = struct.unpack_from('<H', binary, pe + 20)[0]
    sections = []
    for i in range(struct.unpack_from('<H', binary, pe + 6)[0]):
        pos = optional + optional_size + i * 40
        size, address, raw_size, offset = struct.unpack_from('<IIII', binary, pos + 8)
        sections.append((address, max(size, raw_size), offset))
    def rva(address):
        for start, size, offset in sections:
            if start <= address < start + size:
                return offset + address - start
        raise ValueError('Invalid PE RVA')
    if struct.unpack_from('<H', binary, optional)[0] != 0x10b:
        raise ValueError('Expected this game\'s PE32 executable')
    base = rva(struct.unpack_from('<I', binary, optional + 112)[0])
    def walk(pos, path):
        named, numeric = struct.unpack_from('<HH', binary, pos + 12)
        for i in range(named + numeric):
            name, entry = struct.unpack_from('<II', binary, pos + 16 + i * 8)
            if name & 0x80000000:
                start = base + (name & 0x7fffffff)
                length = struct.unpack_from('<H', binary, start)[0]
                name = binary[start + 2:start + 2 + length * 2].decode('utf-16le')
            full = path + [name]
            if entry & 0x80000000:
                yield from walk(base + (entry & 0x7fffffff), full)
            elif full[:2] == wanted:
                address, size = struct.unpack_from('<II', binary, base + entry)
                offset = rva(address)
                yield binary[offset:offset + size]
    return next(walk(base, []))

def main():
    output = ROOT / 'build/game'
    output.mkdir(parents=True, exist_ok=True)
    game = ROOT / 'StudySteadyR18'
    noa.extract(game / 'script.noa', 'script.csx', output / 'script.csx')
    packed = output / 'IDR_COTOMI.erisan'
    packed.write_bytes(pe_resource((game / 'ststeady.exe').read_bytes(), [10, 'IDR_COTOMI']))
    subprocess.run([str(ROOT / 'build/host/erisan_decode'), str(packed),
                    str(output / 'original-cotopha.xml')], check=True)

if __name__ == '__main__':
    main()
