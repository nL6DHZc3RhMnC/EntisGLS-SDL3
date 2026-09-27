# Source layout and ownership

This repository separates original engine inputs, the launcher we maintain, and
platform application shells. Directory moves do not change the game format,
Android package ID, save rules, or release artifact names.

| Directory | Responsibility |
| --- | --- |
| `EntisGLS/` | Original EntisGLS SDK inputs, including `Cotopha/` and `EntisGLS3/` |
| `vendor/` | Pinned third-party inputs and provenance manifests |
| `native/launcher/` | Game discovery, configuration, PSB settings and runtime session orchestration |
| `native/runtime/cotopha_port/` | Our traditional Cotopha VM, object and native-binding adaptations |
| `native/io/` | Game filesystem abstraction and the game-directory save policy |
| `native/platform/sdl/` | Shared SDL window, input, audio, synchronization and graphics services |
| `native/platform/android/` | Android native storage/JNI adaptation |
| `native/platform/ios/` | iOS native graphics adaptation |
| `native/extensions/emote/` | E-mote, PSB and TJS integration |
| `native/compatibility/` | SDK compatibility headers and explicitly selected game compatibility profiles |
| `apps/launcher/` | Common application entry point |
| `apps/android/` | Active Android Java shell; `legacy/` retains the optional old JNI shell |
| `apps/ios/` | UIKit launcher and iOS bundle resources |
| `apps/macos/` | macOS bundle resources |
| `tests/unit/` | Focused native/Python checks |
| `tests/integration/` | Filesystem and platform integration checks |
| `tests/fixtures/` | Authored fixtures and fixture builders; no commercial game data |
| `tests/probes/` | Optional runtime and graphics diagnostics |
| `tools/build/` | Configure/build/package entry points and toolchain selection |
| `tools/ci/` | Dependency verification, CI checks and release assembly |
| `tools/sdk/` | SDK overlay generators, patch rules and dependency acquisition |
| `tools/diagnostics/` | Resource inspection and manual device/game diagnostics |
| `cmake/` | Build orchestration and SDK/dependency integration |
| `docs/development/` | Build, CI and packaging instructions |
| `docs/research/` | Historical compatibility findings and evidence |
| `assets/` | Redistributable launcher resources and their licenses |

## Engine inputs versus the port

`EntisGLS/Cotopha/` and `native/runtime/cotopha_port/` are not interchangeable
copies. The first contains original SDK source. The second contains our code
that connects the traditional runtime to modern platforms. The port also uses
traditional VM sources from `EntisGLS/EntisGLS3/`.

Adaptations that must transform upstream source produce copies under the CMake
build directory. Do not edit upstream files or commit generated overlays.
Generator scripts and original SDK sources are configuration dependencies so
changing an input regenerates the overlay. Generation remains at configure time
because CMake selects original versus overlaid source paths during configuration.
A future build-time generator must first provide an explicit output manifest.

## Dependency boundaries

Application shells invoke the launcher. The launcher resolves game configuration,
per-game settings and filesystem access, then starts the runtime session. The
session owns environment/font initialization and execution lifecycle. VM and
object adapters live in `cotopha_port`; launcher orchestration belongs in
`native/launcher/runtime_session.cpp`.

The game filesystem abstraction in `native/io/` is shared by native files and
Android document trees. Both implement the same game-relative operations. Saves
use `$(CURRENT)\savedata`; missing directories are created, and old application
private saves are not migrated. Platform UI and permission handling stay in the
application shells or corresponding native platform adapter.

Some historical CMake target names and SDK macros remain to preserve compatibility
with existing build tooling. Moving sources does not imply that all SDK coupling
has been removed. In particular, graphics adapters still use the original
OpenGL/GLES types and the E-mote implementation shares runtime infrastructure.

## Diagnostics and validation

Production builds exclude the traditional runtime probe collection by default.
Use `ENTISGLS_BUILD_DIAGNOSTICS=ON` for a development build that needs those probes.
Tests and fixture builders remain explicit targets and are not bundled as game
resources. Optional diagnostics may require user-provided game data; automated
CI launcher checks use authored, asset-free fixtures.

The `Build and release` workflow is the build verification entry point for this
refactor. It builds Android ARM64, macOS Intel/Apple Silicon and an unsigned iOS
ARM64 IPA, runs the applicable checks (including iOS simulator library and GLES
presentation checks), and only publishes when required jobs
pass. Platform build success does not establish compatibility with every game
or replace device graphics/audio testing.

Tool scripts can be invoked directly from any working directory. Their shared
bootstrap locates the repository and resolves internal Python imports. Prefer
the categorized entry points; historical commands in research notes describe
the state at the time of the experiment.
