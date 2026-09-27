# Developer tools

Use the categorized scripts directly, for example:

```sh
python3 tools/ci/ci_prepare_dependencies.py
python3 tools/build/build_sdl_android.py
python3 tools/build/build_sdl_desktop.py --arch arm64
python3 tools/build/build_sdl_ios.py
```

| Folder | Purpose |
| --- | --- |
| `build/` | Platform builds, packaging, signing and toolchain selection |
| `ci/` | CI checks, dependency integrity and release assembly |
| `sdk/` | Pinned dependency setup and generated SDK source adaptations |
| `diagnostics/` | Game/resource inspection and manual runtime/device probes |

`_bootstrap.py` locates the checkout and exposes these internal helper modules
to direct script entry points. Scripts do not depend on the caller's working
directory. Repository paths and default build outputs are resolved from that
root; explicit command-line paths retain each command's documented semantics.
Python unit checks are in `tests/unit/python/`, and Android host/provider checks
are in `tests/integration/android/`.

The supported release build entry point is the GitHub Actions **Build and
release** workflow. The commands above document the entry points used by the
workflow and do not imply a requirement to build locally. See
[CI instructions](../docs/development/github-actions.md) and
[source layout](../docs/architecture/source-layout.md).

Project-maintained provenance records update their `verification` command when
an entry point moves. This changes only that command path; upstream versions,
source hashes and recorded file hashes remain unchanged.
