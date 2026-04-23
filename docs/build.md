# Agent Context: Build

Human source: `docs/zh/build.md`. Use this file for build prerequisites,
canonical commands, outputs, and failure routing.

## Canonical Command

```powershell
powershell -ExecutionPolicy Bypass -File scripts/build.ps1
```

Do not invent a new build command unless the task explicitly requires bypassing
the standard script.

## Prerequisites

- CMake `>= 3.20`
- A C compiler toolchain available to CMake
- On Windows: Visual Studio Build Tools or Visual Studio with C++ workload

## Build Script Contract

`scripts/build.ps1`:

1. Resolves repository root from the script location.
2. Uses `build/` as the CMake build directory.
3. Uses `RelWithDebInfo` as the build type.
4. Verifies `cmake` is available.
5. Runs CMake configure.
6. Runs CMake build.

## Expected Outputs

- Static library under `build/` in a toolchain-dependent subdirectory.
- Test executable under `build/`.
- CMake target names: `helicon` and `helicon_tests`.

## Failure Routing

| Symptom | Check |
| --- | --- |
| `cmake` not found | Install CMake and ensure it is on `PATH`. |
| compiler not found | Install a supported C toolchain. |
| target/source missing | Check `CMakeLists.txt` against actual files. |
| generated files stale | Remove `build/` and rerun `scripts/build.ps1`. |
