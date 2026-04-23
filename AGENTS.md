# AGENTS.md

## Project purpose
This repository is developed with Codex as the primary coding agent.
The project is currently in an exploratory assembly phase: the user may copy
candidate code into the repo, then iterate with Codex on planning, cleanup,
integration, and design.
Stable decisions should be captured in agent docs so later work needs less
repeated context.

## How to work in this repo
Before making changes:
1. Read docs/architecture.md
2. Read docs/project-structure.md
3. Read docs/build.md if the change touches build setup, CMake, or build assumptions
4. Read docs/testing.md if the change touches validation strategy, tests, or test assumptions
5. Read docs/doc-sync.md if the change touches documentation or project guidance
6. Read docs/github-workflow.md if preparing commits, pushes, or pull requests
7. Prefer existing scripts in /scripts when they are still relevant; do not recreate deleted scaffolding unless requested

## Documentation language workflow
- Human-editable Chinese source docs live under docs/zh/
- Agent-facing English docs live under docs/
- If a Chinese source doc changes, update the matching English doc in the same change
- Do not ask the user to write English; translate and normalize the English agent docs yourself
- Keep Chinese docs friendly for humans and English docs concise for automation context
- English docs are derived agent context, not literal translations; optimize them for search and action

## Working mode
- Treat copied-in code as candidate material until the user keeps it through review and iteration
- Favor documenting stable decisions, kept modules, directory ownership, and integration constraints
- Remove stale or redundant documentation instead of preserving historical noise
- When the repo structure and docs disagree, update docs to match the user's current intent and working tree

## Coding rules
- Make the smallest safe change that solves the task
- Do not rename public APIs unless necessary
- Do not add dependencies unless justified
- Keep changes local and easy to review
- Update docs when behavior or workflow changes
- When formatting code in a file whose global style is not finalized yet, preserve the file's existing header banner, comment structure, and local layout conventions
- For `include/utils.h`, treat the current file header template and existing comments as the formatting baseline; do not rewrite them unless the user explicitly asks
- Do not perform repo-wide style normalization, indentation unification, or comment-style conversion until the user defines the formatter/style contract

## Validation rules
After code or workflow changes:
1. Decide whether the current repository state is buildable and testable
2. Run scripts/build.ps1 and scripts/test.ps1 only when that workflow is real and the user wants validation
3. If validation is skipped, state the reason clearly
4. Summarize what changed
5. Summarize what was validated or intentionally not validated
6. List risks or follow-up work if any

## Bug fixing rules
When fixing a bug:
1. State the suspected root cause first
2. Identify the smallest code area to change
3. Prefer one fix at a time
4. Explain how the fix was validated or why validation was skipped

## Pull request output
When preparing a PR, include:
- Summary
- Why this change is needed
- Validation performed or why validation was skipped
- Risks / follow-ups

Before committing, pushing, or opening a PR:
- Generate a commit subject, detailed commit body, PR title, and PR description for user review
- List the files proposed for staging
- Wait for explicit user confirmation
- Use Conventional Commits: `<type>(<optional scope>): <subject>`
- Use only these types: `feat`, `fix`, `docs`, `style`, `refactor`, `perf`, `test`, `chore`
- Keep `type` lowercase English and write `subject` in concise Chinese imperative present tense
- Include a commit body with detailed summary, changed files or modules, validation, and risks unless the user asks for subject-only commits
