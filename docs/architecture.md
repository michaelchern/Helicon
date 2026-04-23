# Agent Context: Architecture

Human source: `docs/zh/architecture.md`. Use this file as the compact contract
for the repository's current exploratory workflow.

## Current Phase

Helicon is in an exploratory assembly phase. The user may copy candidate code
into the repository, then iterate with Codex on design, cleanup, naming,
integration, and selection.

Do not treat the current code snapshot as a finalized architecture. Public docs
should record only stable decisions that survived review and iteration.

## What To Treat As Stable

- The repo is Codex-first and doc-driven.
- Chinese docs under `docs/zh/` are the human source of truth.
- English docs under `docs/` are derived agent context.
- Stable decisions should be written down to reduce repeated context in later work.

## What Not To Assume Yet

- A fixed graphics API or backend strategy
- A fixed public API surface
- A fixed build baseline
- A fixed test matrix
- That all copied-in code is meant to stay

## Ownership Boundaries

| Area | Contract |
| --- | --- |
| `include/` | Current public-header candidate area. Do not treat it as a stable API promise unless the docs say so. |
| `src/` | Current implementation and experiment area. Transitional structure is allowed. |
| `examples/` | Example or scratch entry points. Long-term status may change. |
| `tests/` | Reserved for future stable validation. It may be empty during exploration. |
| `scripts/` | Reusable scripts that still save time. Do not recreate deleted scaffolding unless requested. |
| `docs/zh/` | Human-editable Chinese source docs. |
| `docs/` | Derived English agent context. |
| `.codex/private/` | Local private plans, references, and roadmap notes; ignored by Git. |
| `third_party/` | Vendored code only when explicitly justified. |

## Documentation Rules

- Record stable decisions, kept modules, ownership boundaries, and integration constraints.
- Remove stale or redundant documentation instead of preserving historical noise.
- If the working tree and older docs disagree, update docs to match the user's current intent.

## Validation Rules

- Do not assume every iteration must build or test successfully.
- Build and test become standard requirements only after the relevant workflow is intentionally restored.
- If validation is skipped, explain why.
