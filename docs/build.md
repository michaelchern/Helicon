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
- A C++20 compiler toolchain available to CMake
- On Windows: Visual Studio Build Tools or Visual Studio with C++ workload
- Vulkan SDK discoverable by CMake as `Vulkan::Vulkan`

## Build Script Contract

`scripts/build.ps1`:

1. Resolves repository root from the script location.
2. Uses `build/` as the CMake build directory.
3. Uses `RelWithDebInfo` as the build type.
4. Verifies `cmake` is available.
5. Runs CMake configure.
6. Runs CMake build.

## Expected Targets

- `helicon`: pure C++20 static library with RHI, render graph, and Vulkan backend.
- `helicon_tests`: C++ smoke test.
- `helicon_render_graph_tests`: C++ Vulkan/render graph regression test.
- `helicon_triangle_graph`: example executable when `HELICON_BUILD_EXAMPLES=ON`.

## Failure Routing

| Symptom | Check |
| --- | --- |
| `cmake` not found | Install CMake and ensure it is on `PATH`. |
| compiler not found | Install a C++20 toolchain. |
| Vulkan not found | Install Vulkan SDK and ensure CMake can find `Vulkan::Vulkan`. |
| target/source missing | Check `CMakeLists.txt` against actual files. |
| generated files stale | Remove `build/` and rerun `scripts/build.ps1`. |
