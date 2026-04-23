---
name: sync-docs
description: Sync human-edited Chinese documentation under docs/zh/ into the matching English agent-facing documentation under docs/.
---

# Purpose

Use this skill when the user asks to sync Chinese docs, translate project guidance
for agents, or update English docs based on Chinese source docs. English docs are
derived agent context, not literal translations.

# Source and target

- Chinese source docs live under `docs/zh/` and are the human review/edit surface.
- English agent docs live under `docs/` and are optimized for AI retrieval and action.
- If Chinese and English conflict, prefer the Chinese source and update English.

# Steps

1. Read `docs/doc-sync.md`.
2. Read the relevant Chinese source file under `docs/zh/`.
3. Read the matching English file under `docs/`.
4. Update the English file to preserve Chinese intent in an agent-friendly form.
5. Keep commands, file paths, API names, and script names unchanged.
6. Do not ask the user to write English.
7. Summarize which Chinese source changed and which English target was synced.

# English style

- Prefer concise headings, tables, checklists, path maps, and action rules.
- Optimize for search, planning, implementation, and validation.
- Do not mirror Chinese prose sentence by sentence.

# Output format

- Chinese source
- English target
- What changed
- Validation performed
