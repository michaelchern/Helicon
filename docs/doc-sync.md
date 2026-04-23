# Agent Doc Sync Contract

This repository uses split-purpose documentation:

- Chinese docs under `docs/zh/` are the human source of truth.
- English docs under `docs/` are derived agent context.

English docs are not literal translations. They should preserve intent while
optimizing for search, planning, and execution.

## Core Rules

- Read the Chinese source before editing the English target.
- Do not ask the user to write English.
- If Chinese and English conflict, prefer Chinese and update English.
- Remove stale or redundant English context instead of preserving historical noise.
- When docs change the workflow, structure, build assumptions, or validation
  assumptions, update all affected pairs in the same change.

## Document Pairs

| Chinese source | English target |
| --- | --- |
| `docs/zh/architecture.md` | `docs/architecture.md` |
| `docs/zh/build.md` | `docs/build.md` |
| `docs/zh/testing.md` | `docs/testing.md` |
| `docs/zh/project-structure.md` | `docs/project-structure.md` |
| `docs/zh/github-workflow.md` | `docs/github-workflow.md` |
| `docs/zh/doc-workflow.md` | `docs/doc-sync.md` |
| `README-zh.md` | `README.md` |

## English Doc Style

- Prefer concise headings, tables, checklists, and search-friendly rules.
- Preserve commands, file paths, script names, and directory names exactly.
- Record stable decisions, not brainstorming residue.

## Sync Summary Expectations

When reporting doc sync work, include:

- Chinese source files reviewed or changed
- English target files updated
- What meaning was preserved or normalized
- Validation performed or intentionally skipped
