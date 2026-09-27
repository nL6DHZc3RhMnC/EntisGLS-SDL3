
# Support direct execution from any working directory.
import sys as _sys
from pathlib import Path as _BootstrapPath
_sys.path.insert(0, str(next(parent / "tools" for parent in _BootstrapPath(__file__).resolve().parents
                             if (parent / "tools" / "_bootstrap.py").is_file())))
from _bootstrap import ROOT

import unittest

from ci_compiler_cache_stats import summarize


class CompilerCacheStatsTests(unittest.TestCase):
    def test_cold_cache_is_active_without_claiming_hits(self):
        result = summarize('direct_cache_hit\t0\npreprocessed_cache_hit\t0\ncache_miss\t19\n')
        self.assertEqual(result['cacheable_calls'], 19)
        self.assertEqual(result['hits'], 0)
        self.assertEqual(result['hit_rate'], 0)

    def test_direct_and_preprocessed_hits_are_counted(self):
        result = summarize('direct_cache_hit 7\npreprocessed_cache_hit 2\ncache_miss 1\ncalled_for_link 3\n')
        self.assertEqual(result['cacheable_calls'], 10)
        self.assertEqual(result['hits'], 9)
        self.assertEqual(result['hit_rate'], 0.9)

    def test_empty_or_partial_statistics_cannot_report_success(self):
        for value in ('', 'cache_miss 2\n', 'direct_cache_hit -1\n', 'cache_miss 1\ncache_miss 1\n'):
            with self.subTest(value=value), self.assertRaises(ValueError):
                summarize(value)

    def test_zero_counters_remain_zero(self):
        result = summarize('direct_cache_hit 0\npreprocessed_cache_hit 0\ncache_miss 0\n')
        self.assertEqual(result['cacheable_calls'], 0)


if __name__ == '__main__':
    unittest.main()
