# Agent Doc Sync Contract

This repository uses split-purpose documentation:

- Chinese docs under `docs/zh/` are the human review and edit surface.
- English docs under `docs/` are derived agent context for retrieval, planning,
  implementation, and validation.

The English docs are not literal translations. They should preserve Chinese
intent while optimizing for AI use.

## Human Workflow To Preserve

1. User reads Chinese docs.
2. User reviews project behavior and accumulated AI context.
3. User checks whether docs are inaccurate or stale.
4. User edits Chinese docs.
5. User asks the agent to sync English docs from Chinese intent.

Do not ask the user to write English. The agent owns English normalization.

## Source Of Truth

If Chinese and English docs conflict, prefer the Chinese source and update the
English target.

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

## Agent Sync Rules

- Read the Chinese source before editing the English target.
- Read the current English target to preserve still-valid operational context.
- Translate meaning, not sentence order.
- Prefer concise headings, tables, checklists, path maps, and action rules.
- Keep English docs optimized for search and execution, not narrative reading.
- Preserve commands, file paths, public API names, target names, and script
  names exactly.
- When docs affect behavior, build, test, or structure, update all affected
  paired docs in the same change.

## Expected Sync Summary

When reporting a sync, include:

- Chinese source files reviewed or changed.
- English target files updated.
- Meaning preserved or normalized.
- Validation performed.
