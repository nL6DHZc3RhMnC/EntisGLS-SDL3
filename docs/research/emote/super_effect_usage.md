# Actual SuperSprite effect use

`tools/diagnostics/motion_audit_super_effects.py` read all 529 `.srcxml` archive entries:
189 from `script.noa`, 340 from `stst_patch_R18.noa`, totaling 37,567,524 decoded
bytes. Counts below are static occurrences across both versions and branches,
not execution frequencies. Every input has a SHA-256 in `super_effect_usage.json`.

The original `ScreenData::SetEffectParam` first checks flags bit 0x100 and uses
`alphaNNN.eri` in that branch. Only otherwise, bit 0x200 selects the effect table
in `ScreenData::GetEffectParam`. Therefore type 7/9/99/201/etc. under the alpha
branch must not be mistaken for unsupported SuperSprite effects.

| Actual effect | Script type / native type | Original / patch occurrences | Earliest original `_R` script | Earliest patch script |
| --- | --- | --- | --- | --- |
| RasterScroll | 6 / 6 | 43 / 69 | common1_11_R, line 16, chgscrn | common1_11, line 16, chgscrn |
| ShadingOff | 9 / 10 | 61 / 87 | common1_7_R, line 136, effect | common1_6, line 216, effect |
| ShadingLight | 10 / 11 | 22 / 101 | common1_11_R, line 242, chgbg | common1_11, line 242, chgbg |

These three are used and exceed the current supported native 0–5 range.
The initial `common1_1` / `common1_1_R` only requests SuperSprite FilterLight 3
(at source lines 57, 555, 608). Across all scripts, the other explicit SuperSprite
requests are FilterWhite 2 (17 / 425) and FilterLight 3 (119 / 154), already
covered by the current implementation. No actual WaveCircle, Shimmer, or
SmashParticle request survived the original flag routing.

The script type table has no MeshWarp entry, whereas the native enum reserves
9 for MeshWarp. Hence script ShadingOff 9 is native 10, and script ShadingLight
10 is native 11. The original native enum is in GLS3 `glsctpsprite.h`.

## SetMeshWarpEffect

There are two genuine native call sites, both the six-argument overload
(class 69, method 132): PhysicalSpringMesh::Initialize at 0x1bde0 and
PhysicalSpringMesh::AdvanceTime at 0x1c092. These occur after inline naked
arithmetic that the linear object-only disassembler cannot decode. The whole-
image typed-call scan located them; their actual decoded native call plus
`free; return 0` precisely reaches each owning function's end.

They are reached through DynamicLayerSet physical-layer loading when a layer
has an image and nonempty `mask`, then through timer updates. The scenario
entry point is the `dynbs` command at CSX 0x646f1 / 0x64833. All 529 scenario XMLs
contain zero `dynbs` commands and zero literal `dynbs`, `LoadLayerSet`,
`PhysicalSpringMesh`, `SetMeshWarpEffect`, or `<layer_set` mentions. No layer-set
resource was found among the game's XML-named archive entries. Thus the compiled
library has the calls, but the supplied scenario data does not establish this
path as a currently required gameplay feature. It is not being reported as a
blanket absent call or made into a priority merely because the SDK supports it.

Read-only evidence includes decoded call sites, 14 explicitly undecoded
naked/data regions, every archive script hash, command parameters, and separate
first occurrences per archive. No mobile execution or screenshot sweep was used.
