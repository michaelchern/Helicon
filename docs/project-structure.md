# Agent Context: Project Structure

Purpose: compact map of the current working tree. Human-facing explanations live
in `docs/zh/project-structure.md`; treat that Chinese file as the source of
truth.

## Read Order

1. `docs/architecture.md`
2. `docs/project-structure.md`
3. `docs/code-style.md` when formatting, naming, or comment style matters
4. `docs/build.md` when build assumptions matter
5. `docs/testing.md` when validation assumptions matter
6. `docs/doc-sync.md` when documentation changes

## Repository Tree

```text
Helicon/
|-- docs/
|   `-- zh/
|-- examples/
|   `-- triangle_graph/
|-- include/
|   |-- common/
|   |-- rhi.h
|   |-- rhiHLSL.h
|   |-- utils.h
|   |-- validation.h
|   `-- vulkan.h
|-- scripts/
|-- skills/
|-- src/
|   |-- ast/
|   |-- backends/
|   |   |-- cuda/
|   |   `-- vulkan/
|   |       `-- vulkan_backend.cpp
|   `-- dsl/
|-- tests/
|-- third_party/
|-- AGENTS.md
|-- CMakeLists.txt
|-- README.md
`-- README-zh.md
```

Generated/local directories:

- `build/`: generated output; do not edit by hand
- `out/`: generated IDE/CMake output; do not edit by hand
- `.vs/`: Visual Studio local state
- `.git/`: Git metadata

## Directory Contracts

| Path | Contract |
| --- | --- |
| `AGENTS.md` | Repo-level agent workflow and constraints. |
| `docs/zh/` | Human-editable Chinese source docs. |
| `docs/` | Derived English agent context. |
| `include/` | Current public-header candidate area. |
| `include/common/` | Shared low-level helpers and types under evaluation. |
| `include/utils.h` | Current reference header for banner, bilingual comments, and snake_case helper naming. |
| `include/rhiHLSL.h` | Shared compatibility header. Align comments and layout to repo style, but keep upstream/spec/shader-constrained identifiers stable. |
| `src/` | Current implementation and experiment area. |
| `src/backends/vulkan/` | Current Vulkan-side implementation area. |
| `src/ast/`, `src/dsl/`, `src/backends/cuda/` | Reserved or placeholder directories. Do not assume active functionality. |
| `examples/triangle_graph/` | Example or scratch entry point. |
| `tests/` | Reserved for future stable validation. It may be empty during exploration. |
| `scripts/` | Reusable scripts that still save time. |
| `skills/` | Repo-local AI workflows. |
| `third_party/` | Vendored code only when explicitly justified. |

## Search Routing

| Task | Start Here | Then Check |
| --- | --- | --- |
| Understand current workflow | `docs/architecture.md` | `AGENTS.md` |
| Confirm actual structure | `docs/project-structure.md` | working tree |
| Header/API candidate work | `include/` | `docs/architecture.md` |
| Formatting / naming / comment cleanup | `docs/code-style.md` | `include/utils.h` |
| `utils.h` formatting or cleanup | `include/utils.h` | `docs/code-style.md` |
| `rhiHLSL.h` cleanup | `include/rhiHLSL.h` | `docs/code-style.md` |
| Vulkan-side work | `src/backends/vulkan/` | `include/` |
| Example/scratch behavior | `examples/triangle_graph/` | `README.md` |
| Build assumptions | `docs/build.md` | `scripts/build.ps1`, `CMakeLists.txt` |
| Validation assumptions | `docs/testing.md` | `scripts/test.ps1`, `tests/` |
| Sync docs | `docs/doc-sync.md` | matching files under `docs/zh/` |

## Interpretation Rules

- Prefer actual files plus the latest docs over stale assumptions from older scaffolding.
- A directory that only contains `.gitkeep` is reserved, not implemented.
- If `CMakeLists.txt`, scripts, and the working tree disagree, treat the working tree and user intent as primary until build work is explicitly requested.
- Follow `docs/code-style.md` for large-scale formatting or naming cleanup.
