#!/usr/bin/env python3
"""Audit real CSX ReleasePlayer call sites without executing game code."""
import argparse
import hashlib
import json
import struct
from pathlib import Path

from csx_inspect import CSX


def audit(path):
    csx = CSX(path)
    slots = []
    for ci, cls in enumerate(csx.classes):
        for fi, method in enumerate(cls['methods']):
            if method['name'] != 'ReleasePlayer':
                continue
            # Member-call instructions are opcode, argc:u32, class:u32,
            # method:u32. Scan the entire image as well as decoded functions,
            # so unsupported naked regions cannot hide this typed instruction.
            pair = struct.pack('<II', ci, fi)
            candidates = []
            position = 0
            while True:
                position = csx.image.find(pair, position)
                if position < 0:
                    break
                if position >= 5 and csx.image[position - 5] in (19, 20):
                    candidates.append(hex(position - 5))
                position += 1
            slots.append({'class': cls['name'], 'class_index': ci,
                          'method_index': fi, 'raw_typed_call_candidates': candidates})
    decoded = {}
    undecoded = []
    emote_calls = []
    release_calls = []
    for function in csx.functions:
        if 'size' not in function:
            continue
        position = function['address']
        end = position + function['size']
        while position < end:
            try:
                instruction = csx.instruction(position)
            except (ValueError, IndexError, KeyError) as error:
                undecoded.append({'function': function['name'], 'address': hex(position),
                                  'error': str(error)})
                break
            decoded[position] = instruction
            if 'call' in instruction['mnemonic']:
                call = {'function': function['name'], **instruction}
                if 'Emote' in str(instruction['args']):
                    emote_calls.append(call)
                if 'ReleasePlayer' in str(instruction['args']):
                    release_calls.append(call)
            position += instruction['size']
    return {
        'path': str(path),
        'sha256': hashlib.sha256(Path(path).read_bytes()).hexdigest(),
        'function_records': len(csx.functions),
        'unique_decoded_instructions': len(decoded),
        'undecoded_regions': undecoded,
        'release_player_declarations': slots,
        'release_player_decoded_calls': release_calls,
        'release_player_constant_strings': [
            {'text': text, 'references': csx.string_references[index]}
            for index, text in enumerate(csx.strings) if 'ReleasePlayer' in text
        ],
        'emote_calls': emote_calls,
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('path', nargs='?', default='build/game/script.csx')
    parser.add_argument('--output', type=Path)
    args = parser.parse_args()
    report = audit(args.path)
    serialized = json.dumps(report, ensure_ascii=False, indent=2) + '\n'
    if args.output:
        args.output.write_text(serialized)
        print(f"{len(report['release_player_decoded_calls'])} decoded ReleasePlayer calls; "
              f"{sum(len(slot['raw_typed_call_candidates']) for slot in report['release_player_declarations'])} "
              f"whole-image typed call candidates; evidence: {args.output}")
    else:
        print(serialized, end='')


if __name__ == '__main__':
    main()
