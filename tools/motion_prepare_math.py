#!/usr/bin/env python3
"""Extract independent numeric MotionPlayer routines without changing algorithms.

No TJS or Layer methods are stubbed. The few aggregate fields read by these
functions are represented by plain records at this adaptation boundary.
"""
from pathlib import Path
import hashlib
import json

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / 'vendor/kirikiroid2/motionplayer'
OUT = ROOT / 'build/generated/motion'


def extract(text, marker):
    start = text.index(marker)
    opening = text.index('{', start)
    count = 1
    end = opening + 1
    while count:
        count += (text[end] == '{') - (text[end] == '}')
        end += 1
    return text[start:end]


def main():
    variable = (SOURCE / 'PlayerVariable.cpp').read_text()
    layers = (SOURCE / 'PlayerUpdateLayersInternal.h').read_text()
    funcs = [
        extract(variable, 'std::int32_t parameterSignedInt32TowardZeroSaturated_guess('),
        extract(variable, 'void normalizeParameterValue_guess('),
        extract(layers, 'inline void applyLocalTransform(\n        Affine2x3 &a,')
    ]
    funcs[1] = funcs[1].replace('detail::MotionParameterEntry', 'Parameter')
    code = '''// Generated from vendor/kirikiroid2/motionplayer; preserve its LICENSE.
#pragma once
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>
namespace studysteady::motion::math {
struct Parameter {
    double rangeBegin = 0, rangeEnd = 0, division = 0, value = 0;
    bool discretization = false;
};
using Affine2x3 = std::array<double, 6>;
'''
    code += '\n\n'.join(funcs) + '\n}\n'
    OUT.mkdir(parents=True, exist_ok=True)
    (OUT / 'motion_math.h').write_text(code)
    provenance = {}
    for name in ('PlayerVariable.cpp', 'PlayerUpdateLayersInternal.h'):
        provenance[name] = hashlib.sha256((SOURCE / name).read_bytes()).hexdigest()
    (OUT / 'provenance.json').write_text(json.dumps(provenance, indent=2) + '\n')


if __name__ == '__main__':
    main()
