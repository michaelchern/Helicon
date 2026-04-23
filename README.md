<p align="center">
  <a href="./README.md">English</a> | <a href="./README-zh.md">简体中文</a>
</p>

# Helicon

Helicon is a Codex-first **pure C++20** high-performance rendering core. The
current v0 targets Windows + Vulkan with a C++ RHI facade, a minimal render
graph, and headless offscreen triangle rendering.

Why C++20:

- It covers the current API needs, including `std::span`, RAII, strong enums,
  and modern standard-library facilities.
- Visual Studio, CMake, and mainstream compilers support it well.
- C++23 does not yet simplify the v0 rendering core enough to justify the
  higher toolchain floor.

Current capabilities:

- C++ API for `helicon::Context`, `Device`, `Queue`, `Buffer`, `Image`,
  `ShaderModule`, `GraphicsPipeline`, and `CommandList`.
- Render graph v0 for RGBA8 color attachments, pass declaration, color writes,
  and an embedded builtin triangle draw.
- Vulkan backend for headless offscreen rendering, basic resource creation,
  command submission, and RGBA8 readback.
- `helicon_triangle_graph` example that writes `triangle_graph.ppm`.

## Quickstart (Windows / PowerShell)

Requires CMake, a Visual Studio C++ toolchain, and the Vulkan SDK.

```powershell
powershell -ExecutionPolicy Bypass -File scripts/build.ps1
powershell -ExecutionPolicy Bypass -File scripts/test.ps1
```

## Repository Docs

- `docs/architecture.md`
- `docs/project-structure.md`
- `docs/build.md`
- `docs/testing.md`
- `docs/github-workflow.md`
- `docs/doc-sync.md`

Chinese docs under `docs/zh/` are the human source of truth. English docs under
`docs/` are derived operational context for agents.
