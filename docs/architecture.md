# Agent Context: Architecture

Human source: `docs/zh/architecture.md`. Use this file as compact architecture
context for planning and code changes.

## Project Goal

Maintain a minimal, deterministic, runnable C project shape. Optimize for small,
safe changes with low ambiguity.

## Ownership Boundaries

| Area | Contract |
| --- | --- |
| `include/` | Public headers and stable API boundary. Avoid breaking compatibility unless required. |
| `src/` | Production implementation. Current reserved boundaries: `ast/`, `dsl/`, `backends/`. |
| `tests/` | Executable validation. Update when behavior changes. |
| `scripts/` | Canonical automation entry points for build/test/CI. |
| `docs/zh/` | Human-editable Chinese source docs. Prefer for user intent. |
| `docs/` | Derived English agent context. Keep operational and concise. |
| `examples/` | Usage examples and minimal demos. |
| `third_party/` | Vendored external code only when justified. |

Detailed path map: `docs/project-structure.md`.

## Build/Test Contract

```powershell
powershell -ExecutionPolicy Bypass -File scripts/build.ps1
powershell -ExecutionPolicy Bypass -File scripts/test.ps1
```

- `scripts/build.ps1` configures and builds with CMake into `build/`.
- `scripts/test.ps1` runs CTest from the same `build/` tree.
- Prefer scripts over ad hoc commands.

## API Rule

Public APIs live under `include/`. Prefer extending existing headers and keeping
compatibility. If a public API changes, inspect `src/` and `tests/`.

## Change Rules

- Make the smallest safe change that solves the task.
- Keep implementation, API, tests, scripts, and docs in their assigned areas.
- Update docs when behavior, workflow, or structure changes.
- If Chinese docs change, sync the matching English agent docs.
