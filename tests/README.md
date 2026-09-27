# Tests and diagnostics

| Directory | Scope |
| --- | --- |
| `unit/` | Focused native and Python checks |
| `integration/` | SDK/filesystem/platform integration, including Android provider tests and iOS presentation smoke |
| `fixtures/` | Shared fixture helpers and the authored CSX fixture generator |
| `probes/` | Opt-in traditional-runtime and E-mote/graphics diagnostics |

The release workflow runs the asset-free checks appropriate for each platform.
`tools/ci/ci_native_checks.py` builds and executes the desktop fixture targets;
its synthetic game verifies configuration discovery, script execution and saves
without reading any commercial game directory. Android host tests exercise a
fake provider and do not replace real-device permission/storage validation.

The traditional runtime probe collection is excluded from the launcher by
default. A diagnostic launcher uses `ENTISGLS_BUILD_DIAGNOSTICS=ON`. Some manual
probe modes require a compatible game supplied by the user. iOS presentation
smoke retains its existing explicit launch mode and does not run during normal
game launch.

Build/test verification for the source-layout refactor runs in GitHub Actions.
Do not include proprietary games or generated binaries as fixtures.
