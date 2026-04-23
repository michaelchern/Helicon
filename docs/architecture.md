# Agent Context: Architecture

Human source: `docs/zh/architecture.md`. Use this file as compact architecture
context for planning and code changes.

## Project Goal

Helicon is a pure C++20 high-performance rendering core. The v0 milestone
targets Windows + Vulkan with a C++ RHI facade and a minimal render graph that
can render a headless offscreen triangle and read back RGBA8 pixels.

C++20 is the project standard because it provides the current API needs
(`std::span`, RAII-friendly types, strong enums, modern library facilities) with
mature Visual Studio/CMake/compiler support. Do not raise to C++23 unless a
specific feature materially simplifies implementation.

Out of scope for v0: windowing, PBR, glTF, ray tracing, async compute, multiple
graphics APIs, CUDA/Metal/DirectX backends, and the shader DSL. Shader DSL work
will build later on `src/ast/`, `src/dsl/`, and backend codegen boundaries.

## Ownership Boundaries

| Area | Contract |
| --- | --- |
| `include/helicon/` | Public C++ API. Current main entry: `helicon/helicon.hpp`. |
| `src/` | Production implementation: RHI facade, render graph, backend glue. |
| `src/backends/vulkan/` | Vulkan v0 backend: instance/device/queue, resources, command submission, offscreen render/readback. |
| `src/shaders/` | Embedded shader data; current builtin triangle SPIR-V constants. |
| `src/ast/`, `src/dsl/` | Reserved for future Helicon Shader DSL work. |
| `tests/` | C++ smoke tests and C++ render graph regression tests. |
| `examples/` | Runnable examples, currently `triangle_graph`. |
| `scripts/` | Canonical build/test/CI entry points. |
| `docs/zh/` | Human-editable Chinese source docs. |
| `docs/` | Derived English agent context. |
| `third_party/` | Vendored code only when explicitly justified. |

## Build/Test Contract

```powershell
powershell -ExecutionPolicy Bypass -File scripts/build.ps1
powershell -ExecutionPolicy Bypass -File scripts/test.ps1
```

- `scripts/build.ps1` configures and builds with CMake into `build/`.
- `scripts/test.ps1` runs CTest from the same `build/` tree.
- Prefer scripts over ad hoc commands.

## API Rules

- Public APIs live under `include/helicon/`.
- Do not expose Vulkan handles in public API; keep backend details in
  `src/backends/vulkan/`.
- Extend the C++ API conservatively and update tests/docs for behavior changes.

## Change Rules

- Make the smallest safe change that solves the task.
- Update docs when behavior, dependencies, workflow, or structure changes.
- If Chinese docs change, sync the matching English agent docs.
