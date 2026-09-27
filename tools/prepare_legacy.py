#!/usr/bin/env python3
"""Validate the traditional runtime already expanded in the official SDK."""
from entis_sdk import LEGACY, validate

if __name__ == '__main__':
    validate()
    print(f'Traditional Cotopha sources: {LEGACY}')
