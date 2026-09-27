#!/usr/bin/env python3
"""Record runner-side compiler cache evidence, and reject a bypassed cache."""

import argparse
import json
import os
from pathlib import Path
import re
import subprocess


def summarize(text):
    counters = {}
    for line in text.splitlines():
        if not line.strip():
            continue
        fields = line.split()
        if len(fields) != 2 or not re.fullmatch(r'[a-z0-9_]+', fields[0]) or not fields[1].isdigit():
            raise ValueError(f'Unexpected ccache counter: {line!r}')
        if fields[0] in counters:
            raise ValueError(f'Duplicate ccache counter: {fields[0]}')
        counters[fields[0]] = int(fields[1])
    required = ('direct_cache_hit', 'preprocessed_cache_hit', 'cache_miss')
    if any(name not in counters for name in required):
        raise ValueError('Missing cache hit/miss counters in ccache output')
    hits = counters['direct_cache_hit'] + counters['preprocessed_cache_hit']
    misses = counters['cache_miss']
    return {
        'cacheable_calls': hits + misses,
        'hits': hits,
        'misses': misses,
        'hit_rate': hits / (hits + misses) if hits + misses else 0.0,
        'counters': counters,
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--require-compilations', action='store_true')
    parser.add_argument('--require-hits', action='store_true')
    args = parser.parse_args()
    compiler_cache = os.environ.get('ENTISGLS_COMPILER_CACHE', 'ccache')
    raw = subprocess.check_output([compiler_cache, '--print-stats'], text=True)
    report = summarize(raw)
    report['version'] = subprocess.check_output([compiler_cache, '--version'], text=True).splitlines()[0]
    report['directory'] = os.environ.get('CCACHE_DIR', '')
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2) + '\n')
    message = (f"Compiler cache: {report['cacheable_calls']} cacheable calls, "
               f"{report['hits']} hits, {report['misses']} misses "
               f"({report['hit_rate']:.1%} hit rate)")
    print(message)
    if os.environ.get('GITHUB_STEP_SUMMARY'):
        with open(os.environ['GITHUB_STEP_SUMMARY'], 'a') as summary:
            summary.write(message + '\n\n')
    if args.require_compilations and not report['cacheable_calls']:
        raise SystemExit('No cacheable compiler calls were recorded; compiler cache integration is not working.')
    if args.require_hits and not report['hits']:
        raise SystemExit('The warm-cache run did not reuse any cached compilation.')


if __name__ == '__main__':
    main()
