# Actual movie presentation usage — offline audit

All **529** original `.srcxml` entries were decoded and scanned: 189 from
`script.noa` and 340 from `stst_patch_R18.noa`. Their decoded SHA-256 values match
the existing complete scenario inventory. Counts include original and patch
variants; they are static occurrences, not completed play-throughs.

| Movie | Open/play/close groups | Requests direct | Requests ordinary composition |
| --- | ---: | ---: | ---: |
| `opening.mei` | 8 | 8 | 0 |
| `トンネル車窓.mei` | 2 | 0 | 2 |
| `空から雪.mei` | 14 | 0 | 14 |

The `loop` branch is decisive. CSX initializes play flags to 1
(`0x655e2..0x655e9`), sets them to 0 when `layered != 0`
(`0x65618..0x6562c`), then **assigns 2** when `loop != 0`
(`0x6565c..0x65670`). This is assignment, not OR. Consequently the 16 looping
tunnel/snow uses request ordinary composition even though their raw `layered`
attribute is 0. A later platform check may add decode flag `0x400`; it does not
change the direct-presentation bit.

The eight direct script uses are the four route introductions:

| Original script | Patch script | Surrounding source lines |
| --- | --- | --- |
| `haz_01_R.srcxml` | `haz_01.srcxml` | wait 119 → close 122 |
| `mai_01_R.srcxml` | `mai_01.srcxml` | wait 118 → close 121 |
| `nak_01_R.srcxml` | `nak_01.srcxml` | wait 122 → close 125 |
| `yuu_01_R.srcxml` | `yuu_01.srcxml` | wait 123 → close 126 |

Their generated open/play tags have no `@l`; the table identifies them by the
adjacent annotated commands. All use these same raw parameters:

```text
open_movie: src=opening.mei, priority=65535, fullscreen=-1, x=0, y=0, fadein=0
play_movie: layered=0, loop=0, vol_line=0
wait_movie: no_skip_time=2000
close_movie: fadeout=0
```

Each first hides the message, fades sound, changes the background to white, and
waits 500 ms. Between open and close there are only play and wait commands; no
dialogue or visual-layer change occurs in that interval.

The CSX movie gallery is an additional non-XML caller, also only for
`opening.mei`: `UIExtraMode::PlayMovie` opens fullscreen at `(0,0)`, priority
65536, fade-in 0 (`0x3aac3`), requests flags 1 (`0x3aae4`), waits with 2000 ms
unskippable time, and closes. It fades out the gallery and stops its music before
opening, then fades the gallery back in after closing.

No other movie in this inventory requests direct presentation. No actual
non-opening scene requiring bypass of Sprite alpha or priority was identified.
The snow and tunnel scenes often include dialogue, but their effective flags
are already 2, so they are not additional direct-path compatibility risks.

This does **not** establish complete visual equivalence with Windows, hidden
runtime state, or successful execution of the changed Android opening path.
Its live-window regression remains pending because the phone disappeared from
USB/ADB. No production or game files were changed by this audit.

`movie_presentation_usage.json` retains every movie occurrence's original
attributes, source/archive/hash, nearby command parameters, and effective flags.
Supporting disassembly is in `build/analysis/movie-command-branches.txt`,
`opening-openmovie-audit.txt`, `opening-playmovie-audit.txt`, and
`movie-gallery-play-audit.txt`.
