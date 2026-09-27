#!/usr/bin/env python3
"""Compile the portable CSX codecs and round-trip real game class records.

Requires a host C++17 compiler. The native test structures intentionally have
64-bit pointers and a larger member-function union than the serialized x86 ABI.
"""
from pathlib import Path
import argparse
import shutil
import struct
import subprocess
from csx_inspect import CSX, Reader, read_class

SOURCE = r'''
#include "legacy_serialization.h"
#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

struct File {
    std::vector<uint8_t> bytes; size_t position = 0;
    size_t Read(void* out, size_t count) {
        count = std::min(count, bytes.size() - position);
        std::memcpy(out, bytes.data() + position, count); position += count; return count;
    }
    size_t Write(const void* data, size_t count) {
        const auto* p = static_cast<const uint8_t*>(data);
        bytes.insert(bytes.end(), p, p + count); return count;
    }
    size_t GetLength() const { return bytes.size(); }
    size_t GetPosition() const { return position; }
    uint32_t Count() {
        uint8_t data[4]; assert(Read(data, 4) == 4);
        return StudySteadyLegacyWire::Read32(data);
    }
};
struct String {
    std::vector<wchar_t> chars; int length = 0;
    wchar_t* GetBuffer(int n) { chars.resize(size_t(n) + 1); return chars.data(); }
    void ReleaseBuffer(int n) { length = n; chars[n] = 0; }
    int GetLength() const { return length; }
    const wchar_t* CharPtr() const { return chars.data(); }
};
struct Cast {
    uintptr_t iNativeParent;
    int iVarOffset, nVarBounds, iFuncOffset;
};
struct Function {
    enum Type { Index, Script, Native, Naked } m_ftType;
    Cast m_castThis;
    uint32_t m_dwAlign;
    union { uintptr_t addrScript; char nativeMemberPointer[16]; } m_varFunc;
    uint32_t m_dwPadding[3];
};
template<class Value, class Read, class Write>
uint32_t Verify(File& input, Read read, Write write) {
    uint32_t count = input.Count();
    for (uint32_t i = 0; i < count; ++i) {
        size_t begin = input.position;
        Value value{}; assert(read(input, value));
        File encoded; assert(write(encoded, value));
        assert(encoded.bytes.size() == input.position - begin);
        assert(std::equal(encoded.bytes.begin(), encoded.bytes.end(), input.bytes.begin() + begin));
    }
    return count;
}
int main(int argc, char** argv) {
    assert(argc == 2);
    std::ifstream source(argv[1], std::ios::binary);
    File file{std::vector<uint8_t>(std::istreambuf_iterator<char>(source), {})};
    using namespace StudySteadyLegacyWire;
    static_assert(sizeof(Cast) > 16, "run this regression on a 64-bit host");
    static_assert(sizeof(Function) > 40, "native method pointer union must be larger");
    auto casts = Verify<Cast>(file, ReadCast<File,Cast>, WriteCast<File,Cast>);
    auto funcs = Verify<Function>(file, ReadFunction<File,Function>, WriteFunction<File,Function>);
    auto strings = Verify<String>(file, ReadWideString<File,String>, WriteWideString<File,String>);
    assert(file.position == file.bytes.size());
    File shortCast{{0, 0, 0}}; Cast cast{}; assert(!ReadCast(shortCast, cast));
    cast.iNativeParent = uintptr_t(1) << 40; File output;
    assert(!WriteCast(output, cast)); assert(output.bytes.empty());
    Function fn{}; fn.m_ftType = Function::Native;
    assert(!WriteFunction(output, fn));
    File invalidString{{0xff,0xff,0xff,0x7f}}; String string;
    assert(!ReadWideString(invalidString, string));
    std::cout << "PASS: " << casts << " casts, " << funcs << " function records, "
              << strings << " UTF-16 strings round-trip across native 64-bit layouts\n";
}
'''


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('csx', nargs='?', type=Path, default=Path('build/game/script.csx'))
    p.add_argument('--build-dir', type=Path, default=Path('build/analysis/wire-test'))
    args = p.parse_args()
    casts, functions, strings = [], [], []

    class AuditReader(Reader):
        def unpack(self, fmt):
            begin = self.pos
            result = super().unpack(fmt)
            if fmt == 'IiiiI': casts.append(self.data[begin:begin + 16])
            elif fmt == 'IIiiiIIIII': functions.append(self.data[begin:self.pos])
            return result

        def string(self):
            begin = self.pos
            result = super().string()
            strings.append(self.data[begin:self.pos])
            return result

    csx = CSX(args.csx)
    reader = AuditReader(csx.records['classinf'])
    names = [reader.string() for _ in range(reader.u32())]
    for _ in names: read_class(reader)
    reader.finish()
    # Exercise supplementary characters, unpaired surrogates and embedded NUL.
    for value in ['', '日本語', 'A\U0001f642B', '\ud800x\udfff', 'a\x00b']:
        encoded = value.encode('utf-16-le', errors='surrogatepass')
        strings.append(struct.pack('<I', len(encoded) // 2) + encoded)
    args.build_dir.mkdir(parents=True, exist_ok=True)
    fixture = args.build_dir / 'classinfo-wire.bin'
    fixture.write_bytes(b''.join(struct.pack('<I', len(items)) + b''.join(items)
                                 for items in (casts, functions, strings)))
    source = args.build_dir / 'wire_test.cpp'
    source.write_text(SOURCE)
    binary = args.build_dir / 'wire_test'
    compiler = shutil.which('clang++') or shutil.which('g++')
    if not compiler: raise SystemExit('C++17 compiler is required')
    subprocess.run([compiler, '-std=c++17', '-Wall', '-Wextra', '-Werror',
                    '-I', str(Path(__file__).resolve().parent), str(source), '-o', str(binary)], check=True)
    subprocess.run([str(binary), str(fixture)], check=True)


if __name__ == '__main__':
    main()
