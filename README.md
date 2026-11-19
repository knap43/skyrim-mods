# Skyrim Mods

A collection of Skyrim Special Edition mods, targeting **1.5.97** with SKSE64.

| Mod | Description |
| --- | --- |
| [StartingPerkPoints](mods/StartingPerkPoints) | Grants a configurable number of perk points once per character — on new games and on existing saves. |

## Layout

```
mods/<ModName>/
  CMakeLists.txt   # present for SKSE plugins; picked up automatically
  src/             # plugin source
  dist/            # files copied as-is into the packaged Data/ folder (INIs, ESPs, scripts…)
  README.md
extern/CommonLibSSE-NG   # git submodule, shared by all SKSE plugins
```

## Building

GitHub Actions builds every mod on each push and uploads the zips as a workflow
artifact. Pushing a tag named `<ModName>-v<version>` (e.g. `StartingPerkPoints-v1.0.0`)
publishes a GitHub release with that mod's zip.

Locally on Windows (Visual Studio 2022, CMake, Ninja, vcpkg with `VCPKG_ROOT` set),
run these from a *x64 Native Tools* prompt:

```
git clone --recursive https://github.com/knap43/skyrim-mods
cmake --preset release
cmake --build --preset release
```

The packaged mods end up in `build/package/<ModName>/`.

## Runtime requirements

- [SKSE64](https://skse.silverlock.org/) for 1.5.97
- [Address Library for SKSE Plugins](https://www.nexusmods.com/skyrimspecialedition/mods/32444)
