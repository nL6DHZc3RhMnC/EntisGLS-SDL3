#!/usr/bin/env python3
"""Pair the port's atomic context status writes with atomic reader operations."""
from pathlib import Path
import re


def patch_source(relative_path: str, text: str) -> str:
    name = Path(relative_path).name
    if name == 'glscs_context.h':
        pattern = r'ExecutionStatus GetStatus\( void \) const\s*\{\s*return\s+m_status ;\s*\}'
        replacement = '''ExecutionStatus GetStatus(void) const
        {
            static_assert(sizeof(m_status) == sizeof(int), "execution status width");
            return static_cast<ExecutionStatus>(__atomic_load_n(
                reinterpret_cast<const int*>(&m_status), __ATOMIC_SEQ_CST));
        }'''
        text, count = re.subn(pattern, lambda _: replacement, text)
        if count != 1: raise ValueError(f'Context status getter match count: {count}')
    elif name == 'glscs_context.cpp':
        pattern = r'ESLError ECSContext::ResumeExecution\( ExecutionStatus xsStatus \)\s*\{.*?\n\}'
        text, count = re.subn(pattern, lambda m: re.sub(r'\bm_status\b', 'GetStatus()', m[0]),
                              text, flags=re.S)
        if count != 1: raise ValueError(f'Context execution-loop match count: {count}')
    return text
