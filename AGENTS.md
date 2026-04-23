# AGENTS.md

## Project purpose
This repository is developed with Codex as the primary coding agent.
The project should remain easy to build, test, and maintain.

## How to work in this repo
Before making changes:
1. Read docs/architecture.md
2. Read docs/project-structure.md
3. Read docs/build.md if the change touches build configuration or validation
4. Read docs/testing.md if the change touches code, tests, or validation
5. Read docs/doc-sync.md if the change touches documentation or project guidance
6. Read docs/github-workflow.md if preparing commits, pushes, or pull requests
7. Prefer existing scripts in /scripts instead of inventing shell commands

## Documentation language workflow
- Human-editable Chinese source docs live under docs/zh/
- Agent-facing English docs live under docs/
- If a Chinese source doc changes, update the matching English doc in the same change
- Do not ask the user to write English; translate and normalize the English agent docs yourself
- Keep Chinese docs friendly for humans and English docs concise for automation context
- English docs are derived agent context, not literal translations; optimize them for search and action

## Coding rules
- Make the smallest safe change that solves the task
- Do not rename public APIs unless necessary
- Do not add dependencies unless justified
- Keep changes local and easy to review
- Update docs when behavior changes

## Validation rules
After code changes:
1. Run scripts/build.ps1
2. Run scripts/test.ps1
3. Summarize what changed
4. Summarize what was validated
5. List risks or follow-up work if any

## Bug fixing rules
When fixing a bug:
1. State the suspected root cause first
2. Identify the smallest code area to change
3. Prefer one fix at a time
4. Explain how the fix was validated

## Pull request output
When preparing a PR, include:
- Summary
- Why this change is needed
- Validation performed
- Risks / follow-ups

Before committing, pushing, or opening a PR:
- Generate a commit message, PR title, and PR description for user review
- List the files proposed for staging
- Wait for explicit user confirmation
