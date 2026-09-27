"""Strict command-line parsing for externally supplied PSB probe parameters."""
import argparse
import re


def parse_psb_key(text):
    if not re.fullmatch(r"(?:[0-9]+|0[xX][0-9a-fA-F]+)", text):
        raise argparse.ArgumentTypeError("PSB key must be a uint32 in decimal or 0x hex")
    value = int(text, 16 if text.lower().startswith("0x") else 10)
    if value > 0xFFFFFFFF:
        raise argparse.ArgumentTypeError("PSB key must be a uint32 in decimal or 0x hex")
    return value
