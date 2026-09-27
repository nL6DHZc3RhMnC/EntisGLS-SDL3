# Dependencies for the official EntisGLS source build

`EntisGLS/EntisGLS4.07.03/Makes/Android/jni/jni/Android.mk` requires
Loquaty's C++ sources and TinyGLTF. The SDK distribution does not contain them;
`EntisGLS/loquaty_lib_1.02` contains script libraries, documentation and Windows
tools, rather than the Loquaty C++ implementation.

The following unmodified source snapshots were downloaded directly from their
upstream repositories. Their exact commit IDs, archive URLs and SHA-256 hashes
are recorded in [official-entis-dependencies.json](official-entis-dependencies.json).
The source-tree hash is SHA-256 of the UTF-8 concatenation of sorted relative
file paths in `sha256(file)`, two spaces, `relative/path`, newline format.

* `official-loquaty`: [leshade/loquaty](https://github.com/leshade/loquaty), tag
  `Loquaty_1.02`. The SDK's supplied Loquaty library is version 1.02. License:
  [Apache-2.0](official-loquaty/LICENSE).
* `official-tinygltf`: [syoyo/tinygltf](https://github.com/syoyo/tinygltf), tag
  `v2.9.7`. License and bundled dependency notices:
  [LICENSE](official-tinygltf/LICENSE).

The build uses these source directories and the new `EntisGLS/` SDK. It does not
copy headers, sources or Android static libraries from the retired top-level
`EntisGLS4.07.03/` directory. Any needed Android compatibility changes belong in
project-owned generated copies, not in these upstream snapshots.
