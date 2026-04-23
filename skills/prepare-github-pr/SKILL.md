---
name: prepare-github-pr
description: Prepare a review-first GitHub submission draft for Codex-managed work. Use when the user wants Codex to generate a commit summary, commit message, PR title, PR description, staging scope, or GitHub submission plan for review before committing, pushing, or opening a PR.
---

# Purpose

Use this skill to prepare GitHub submission text and scope for user review.
Default to drafting only. Do not stage, commit, push, or open a PR until the user
explicitly confirms the reviewed draft.

# Required context

1. Read `docs/github-workflow.md`.
2. Read `docs/project-structure.md`.
3. Run `git status -sb`.
4. Inspect the relevant diff before summarizing.

# Draft workflow

1. Identify changed, untracked, deleted, and ignored-relevant files.
2. Detect mixed or unrelated worktree changes.
3. Summarize the intended scope.
4. Check validation status. For code changes, prefer `scripts/build.ps1` and
   `scripts/test.ps1` if they have not already run.
5. Produce a review draft.
6. Wait for explicit user confirmation before any write action.

# Draft output

Include:

- Change summary
- Why this change is needed
- Validation performed
- Risks / follow-ups
- Proposed commit message
- Proposed PR title
- Proposed PR description
- Files proposed for staging

# Write safety

- Never stage unrelated changes silently.
- Never commit, push, or create a PR before the user confirms the draft.
- Prefer explicit file paths when staging.
- Use `git add -A` only when the user confirms the whole worktree belongs in
  scope.
- Default to a draft PR unless the user explicitly asks for ready-for-review.
- Stop and report blockers for missing GitHub auth, missing remote, failed
  validation, or unclear scope.

# PR description template

```markdown
## Summary

## Why

## Validation

## Risks / Follow-ups
```
