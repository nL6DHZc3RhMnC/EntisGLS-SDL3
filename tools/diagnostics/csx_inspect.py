#!/usr/bin/env python3
"""Read Cotopha CSX metadata and object bytecode without executing game code.

Format/operands follow GLS3 glscs_execution_image.cpp and
glscs_execution_reverse_assembler.cpp. Serialized ABI is the original Windows
32-bit ABI (UTF-16 wchar_t, 32-bit long/pointers), regardless of this host.
Unknown bytecode is an error; the inspector never guesses its length.
"""

# Support direct execution from any working directory.
import sys as _sys
from pathlib import Path as _BootstrapPath
_sys.path.insert(0, str(next(parent / "tools" for parent in _BootstrapPath(__file__).resolve().parents
                             if (parent / "tools" / "_bootstrap.py").is_file())))
from _bootstrap import ROOT

import argparse
import bisect
import json
import struct
from pathlib import Path

TYPE_NAMES = ['Object', 'Reference', 'Array', 'Hash', 'Integer', 'Real',
              'String', 'Integer64', 'Pointer', 'ClassObject', 'Boolean',
              'Int8', 'Uint8', 'Int16', 'Uint16', 'Int32', 'Uint32',
              'ArrayDimension', 'HashContainer', 'Real32', 'Real64',
              'PointerReference', 'Buffer', 'Function']
MODES = ['imm', 'stack', 'this', 'global', 'static', 'auto']
OPS = ['add', 'sub', 'mul', 'div', 'mod', 'and', 'or', 'xor', 'land', 'lor', 'sra', 'sll']
UNOPS = ['plus', 'negate', 'not', 'lnot', 'inc', 'dec', 'inc', 'dec']
CMPS = ['ne', 'eq', 'lt', 'le', 'gt', 'ge', 'ptr.ne', 'ptr.eq']


class Reader:
    def __init__(self, data, pos=0):
        self.data, self.pos = data, pos

    def take(self, size):
        if size < 0 or size > len(self.data) - self.pos:
            raise ValueError(f'truncated at 0x{self.pos:x}, requested {size}')
        result = self.data[self.pos:self.pos + size]
        self.pos += size
        return result

    def unpack(self, fmt):
        return struct.unpack('<' + fmt, self.take(struct.calcsize('<' + fmt)))

    def u8(self): return self.unpack('B')[0]
    def u32(self): return self.unpack('I')[0]
    def s32(self): return self.unpack('i')[0]
    def s64(self): return self.unpack('q')[0]
    def f64(self): return self.unpack('d')[0]
    def string(self): return self.take(self.u32() * 2).decode('utf-16-le')
    def array(self): return [self.u32() for _ in range(self.u32())]

    def finish(self):
        if self.pos != len(self.data):
            raise ValueError(f'{len(self.data)-self.pos} unparsed bytes at 0x{self.pos:x}')


def read_object(r, typed=True):
    t = r.s32()
    obj = {'type': TYPE_NAMES[t] if 0 <= t < len(TYPE_NAMES) else t}
    if t == -1: return obj
    if t == 0: obj['class'] = r.string()
    elif t == 1:
        if typed: obj['target'] = read_object(r)
    elif t == 2: obj['items'] = [read_object(r, typed) for _ in range(r.u32())]
    elif t == 3: pass
    elif t in (4, 10, 11, 12, 13, 14, 15, 16): obj['value'] = r.s64() if typed else r.s32()
    elif t in (5, 19, 20): obj['value'] = r.f64()
    elif t == 6: obj['value'] = r.string()
    elif t == 7: obj.update(mask=r.s64(), value=r.s64())
    elif t == 8: obj.update(ref_type=r.s32(), read_only=r.u8(), target=read_object(r))
    elif t == 17:
        obj['element_type'] = read_object(r)
        obj['dimensions'] = r.array()
        obj['items'] = [read_object(r) for _ in range(r.u32())]
    elif t == 18: obj['element_type'] = read_object(r)
    elif t == 23:
        obj.update(address=r.s64(), this_class=r.string(), prototype=read_prototype(r))
    else: raise ValueError(f'unknown serialized type {t} at 0x{r.pos-4:x}')
    return obj


def read_type(r): return {'flags': r.u32(), 'object': read_object(r)}


def read_prototype(r):
    return {'flags': r.u32(), 'name': r.string(), 'global_name': r.string(),
            'return_type': read_type(r), 'args': [read_type(r) for _ in range(r.u32())]}


def read_class(r):
    cls = {'flags': r.u32(), 'name': r.string(), 'global_name': r.string()}
    cls['parents'] = [{'flags': r.u32(), 'name': r.string()} for _ in range(r.u32())]
    cls['casts'] = []
    for _ in range(r.u32()):
        cls['casts'].append(dict(zip(['name', 'native_parent', 'var_offset', 'var_bounds', 'func_offset', 'flags'],
                                    [r.string(), *r.unpack('IiiiI')])))
    cls['variables'] = [{'name': r.string(), 'type': read_type(r)} for _ in range(r.u32())]
    cls['methods'] = []
    for _ in range(r.u32()):
        m = read_prototype(r)
        m['class'] = r.string()
        # ECS_FUNCTION_POINTER, packed Windows x86 layout (40 bytes).
        m['pointer'] = dict(zip(['type', 'native_parent', 'var_offset', 'var_bounds',
                                'func_offset', 'align', 'address', 'pad0', 'pad1', 'pad2'],
                               r.unpack('IIiiiIIIII')))
        cls['methods'].append(m)
    cls['extension'] = r.take(r.u32()).hex()
    return cls


class CSX:
    def __init__(self, path):
        self.path = str(path)
        data = Path(path).read_bytes()
        if data[:8] != b'Entis\x1a\0\0' or b'Cotopha Image file' not in data[:64]:
            raise ValueError('not a Cotopha image')
        self.records = {}
        r = Reader(data, 64)
        while r.pos < len(data):
            tag = r.take(8).decode('ascii').strip()
            size = r.unpack('Q')[0]
            self.records[tag] = r.take(size)
        h = Reader(self.records['header'])
        self.header = dict(zip(['version', 'int_base', 'container_flags', 'reserved',
                               'stack_size', 'heap_size', 'entry', 'static_init', 'resume'],
                              h.unpack('9I')))
        h.finish()
        self.image = self.records['image']
        r = Reader(self.records['function'])
        self.prologues, self.epilogues = r.array(), r.array()
        self.functions = [{'address': r.u32(), 'name': r.string()} for _ in range(r.u32())]
        r.finish()
        if 'funcinfo' in self.records:
            r = Reader(self.records['funcinfo'])
            for _ in range(r.u32()):
                flags, address, size, ext_size = r.unpack('4I')
                self.functions.append({'flags': flags, 'address': address, 'size': size,
                                       'name': r.string(), 'extension': r.take(ext_size).hex()})
            r.finish()
        self.function_names = {f['address']: f['name'] for f in self.functions}
        self.function_addresses = sorted(self.function_names)
        self.strings, self.string_references = [], []
        r = Reader(self.records['conststr'])
        for _ in range(r.u32()):
            self.strings.append(r.string())
            self.string_references.append(r.array())
        r.finish()
        r = Reader(self.records['classinf'])
        self.class_names = [r.string() for _ in range(r.u32())]
        self.classes = [read_class(r) for _ in self.class_names]
        r.finish()
        r = Reader(self.records['global'])
        self.globals = [{'name': r.string(), 'object': read_object(r, self.header['int_base'] == 64)}
                        for _ in range(r.u32())]
        r.finish()
        r = Reader(self.records['data'])
        self.constants = []
        for _ in range(r.u32()):
            name, size = r.string(), r.u32()
            if size & 0x80000000:
                value = read_object(r, self.header['int_base'] == 64)
            else:
                value = [{'name': r.string(), 'object': read_object(r, self.header['int_base'] == 64)}
                         for _ in range(size)]
            self.constants.append({'name': name, 'value': value})
        r.finish()
        self.naked_prologues, self.naked_epilogues = [], []
        if 'initnfnc' in self.records:
            r = Reader(self.records['initnfnc'])
            self.naked_prologues, self.naked_epilogues = r.array(), r.array()
            r.finish()
        self.native_functions, self.naked_functions = [], []
        r = Reader(self.records.get('impnativ', b''))
        while r.pos < len(r.data):
            tag = r.take(8).decode('ascii').strip()
            nested = Reader(r.take(r.unpack('Q')[0]))
            names = [nested.string() for _ in range(nested.u32())]
            references = nested.array()
            nested.finish()
            if tag == 'nativfnc': self.native_functions = names
            elif tag == 'nakedfnc': self.naked_functions = names
            else: raise ValueError(f'unknown import record {tag}')

    def function_at(self, address):
        if address in self.prologues and self.image[address] == 4:
            return self.literal(Reader(self.image, address + 1))
        if address in self.naked_prologues:
            return '@naked_initialize'
        i = bisect.bisect_right(self.function_addresses, address) - 1
        if i < 0: return f'0x{address:x}'
        base = self.function_addresses[i]
        f = next(f for f in self.functions if f['address'] == base)
        if 'size' in f and address >= base + f['size']:
            return f'0x{address:x}'
        return self.function_names[base] + (f'+0x{address-base:x}' if address != base else '')

    def literal(self, r):
        size = r.u32()
        return self.strings[r.u32()] if size == 0x80000000 else r.take(size * 2).decode('utf-16-le')

    def type_name(self, r, t):
        if t == 9: return self.class_names[r.u32()]
        if t == 0: return self.literal(r)
        if 0 <= t < len(TYPE_NAMES): return TYPE_NAMES[t]
        raise ValueError(f'unknown operand type {t}')

    def instruction(self, address):
        r = Reader(self.image, address)
        op, args, mnemonic = r.u8(), [], ''
        if op == 0:
            mode, t = r.u8(), r.u8()
            mnemonic, args = 'obj.new.' + MODES[mode], [self.type_name(r, t), self.literal(r)]
        elif op in (1, 5, 11, 23, 25, 27):
            mnemonic = {1: 'obj.free', 5: 'obj.leave', 11: 'obj.element.pop',
                        23: 'obj.buffer.pop', 25: 'obj.pointer.address.pop', 27: 'obj.reference.object'}[op]
        elif op == 2:
            mode, t = r.u8(), r.u8()
            mnemonic = 'obj.load'
            if mode == 0:
                fmt = {4: 'i', 7: 'q', 5: 'd', 10: 'b', 11: 'b', 12: 'B', 13: 'h', 14: 'H', 15: 'i', 16: 'I'}
                if t in fmt: args = [r.unpack(fmt[t])[0]]
                elif t == 6: args = [json.dumps(self.literal(r), ensure_ascii=False)]
                else: args = [self.type_name(r, t)]
            else:
                operand = MODES[mode]
                if t == 4:
                    idx = r.s32()
                    operand += f'[{idx}]'
                    objects = self.globals if mode == 3 else self.constants if mode == 4 else []
                    if 0 <= idx < len(objects): operand += ' (' + objects[idx]['name'] + ')'
                elif t == 6: operand += '.' + self.literal(r)
                args = [operand]
        elif op in (3, 12, 13, 14):
            operator = r.u8()
            if op == 3: mnemonic = 'obj.store' + ('.' + OPS[operator] if operator != 255 else '') + '.pop'
            elif op == 12: mnemonic = 'obj.operate.' + OPS[operator] + '.pop'
            elif op == 13: mnemonic = 'obj.operate.' + UNOPS[operator]
            else: mnemonic = 'obj.compare.' + CMPS[operator] + '.pop'
        elif op == 4:
            mnemonic, args = 'obj.enter', [self.literal(r)]
            count = r.u32()
            if count != 0xffffffff:
                args.append([self.type_name(r, r.u8()) + ' ' + self.literal(r) for _ in range(count)])
            elif r.u8() == 0:
                # ExecuteEnter adds m_ip after consuming this displacement.
                offset = r.s32()
                args.append(f'try 0x{r.pos + offset:x}')
        elif op in (6, 7):
            mnemonic = 'obj.jump'
            if op == 7:
                flag = r.u8()
                mnemonic = 'obj.cjump.' + ('nz' if flag & 1 else 'z') + ('' if flag & 2 else '.pop')
            offset = r.s32()
            args = [f'0x{r.pos + offset:x}']
        elif op == 8:
            mode, argc = r.u8(), r.u32()
            mnemonic, args = 'obj.call.' + MODES[mode], [self.literal(r), argc]
        elif op in (9, 18):
            mnemonic, args = 'obj.return' if op == 9 else 'obj.ex.return', [r.u8()]
        elif op == 10:
            t = r.u8()
            mnemonic, args = 'obj.element', [r.s32() if t == 4 else self.literal(r) if t == 6 else self.type_name(r, t)]
        elif op == 15:
            sub = r.u8()
            mnemonic = ['obj.array.dim', 'obj.hash.container.pop', 'obj.move.ref.pop'][sub]
            if sub == 0: args = [r.array()]
        elif op == 16:
            sub = r.u8()
            mnemonic = ['obj.deselect', 'obj.boolean', 'obj.sizeof', 'obj.typeof', 'obj.static_cast',
                        'obj.dynamic_cast', 'obj.duplicate', 'obj.delete'][sub]
            if sub == 4: args = list(r.unpack('iii'))
            elif sub == 5: args = [self.literal(r)]
        elif op == 17:
            argc, mode, t = r.u32(), r.u8(), r.u8()
            mnemonic = 'obj.ex.call.' + MODES[mode]
            if mode == 0:
                if t == 6: args = [self.literal(r)]
                elif t == 4:
                    target = r.u32()
                    args = [f'0x{target:x} ({self.function_at(target)})']
                else: raise ValueError(f'unknown call target type {t}')
            args.append(argc)
        elif op in (19, 20):
            argc, ci, fi = r.u32(), r.u32(), r.u32()
            method = self.classes[ci]['methods'][fi]
            mnemonic = 'obj.call.member' if op == 19 else 'obj.call.native.member'
            args = [f'{self.class_names[ci]}.{method["name"]} [class={ci}, method={fi}]', argc]
        elif op == 21:
            mnemonic, args = 'obj.swap', [r.u8(), r.u32(), r.u32()]
        elif op in (22, 24, 28):
            mnemonic = {22: 'obj.buffer', 24: 'obj.pointer.offset', 28: 'obj.call.pointer.pop'}[op]
            args = [r.u32()]
        elif op == 26:
            mnemonic, args = 'obj.reference', [TYPE_NAMES[r.u8()]]
        elif op == 29:
            argc, index = r.u32(), r.u32()
            mnemonic, args = 'obj.call.native', [self.native_functions[index] + f' [index={index}]', argc]
        else:
            raise ValueError(f'unsupported naked opcode 0x{op:02x} at image+0x{address:x}')
        return {'address': address, 'size': r.pos-address, 'opcode': op, 'mnemonic': mnemonic,
                'args': args, 'bytes': self.image[address:r.pos].hex()}

    def disassemble(self, address, count=80):
        instructions = []
        for _ in range(count):
            ins = self.instruction(address)
            instructions.append(ins)
            address += ins['size']
        return instructions

    def summary(self):
        return {'path': self.path, 'header': self.header,
                'records': {k: len(v) for k, v in self.records.items()},
                'prologues': self.prologues, 'epilogues': self.epilogues,
                'naked_prologues': self.naked_prologues, 'naked_epilogues': self.naked_epilogues,
                'native_functions': self.native_functions, 'naked_functions': self.naked_functions,
                'functions': self.functions, 'classes': self.classes,
                'globals': self.globals, 'constants': self.constants,
                'strings': self.strings, 'string_references': self.string_references}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('path', nargs='?', default=ROOT / 'build/game/script.csx')
    parser.add_argument('--json', type=Path, help='save all parsed metadata as JSON')
    parser.add_argument('--address', help='image offset (hex) or exact function name')
    parser.add_argument('--count', type=int, default=80)
    parser.add_argument('--stop-at-return', action='store_true', help='stop linear output at the first return')
    args = parser.parse_args()
    csx = CSX(args.path)
    if args.json:
        args.json.parent.mkdir(parents=True, exist_ok=True)
        args.json.write_text(json.dumps(csx.summary(), ensure_ascii=False, indent=2) + '\n')
    print(json.dumps({'header': csx.header, 'functions': len(csx.functions), 'classes': len(csx.classes),
                      'globals': len(csx.globals), 'constants': len(csx.constants),
                      'strings': len(csx.strings), 'prologues': csx.prologues}, indent=2))
    address = csx.header['entry']
    if args.address:
        match = next((f for f in csx.functions if f['name'] == args.address), None)
        address = match['address'] if match else int(args.address, 16)
    print(f'\n{csx.function_at(address)} @ image+0x{address:x}:')
    function = next((f for f in csx.functions if f['address'] == address and 'size' in f), None)
    end = address + function['size'] if function else len(csx.image)
    for _ in range(args.count):
        if address >= end:
            break
        try:
            ins = csx.instruction(address)
        except (ValueError, IndexError) as e:
            raise SystemExit(str(e)) from e
        print(f'{address:08x}: {ins["bytes"][:32]:32} {ins["mnemonic"]:25} ' + ', '.join(map(str, ins['args'])))
        address += ins['size']
        if args.stop_at_return and ins['opcode'] in (9, 18):
            break


if __name__ == '__main__':
    main()
