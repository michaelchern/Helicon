# Agent Context: GitHub Submission

Human source: `docs/zh/github-workflow.md`. Use this file when the user wants
Codex to prepare, review, commit, push, or open a GitHub PR.

## Default Mode

Default to review-first preparation:

1. Inspect the working tree.
2. Understand the diff.
3. Draft commit and PR text.
4. Show the draft to the user.
5. Wait for explicit confirmation before staging, committing, pushing, or
   opening a PR.

Do not create GitHub-visible changes before the user confirms the reviewed
summary and description.

## Preparation Checklist

- Run `git status -sb`.
- Inspect relevant diffs before summarizing.
- Identify untracked files and deleted files.
- Detect unrelated or mixed-scope changes.
- Check whether build/test validation has already run.
- For code changes, prefer:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/build.ps1
powershell -ExecutionPolicy Bypass -File scripts/test.ps1
```

## Draft Output

Before any commit or push, provide:

- Change summary.
- Why this change is needed.
- Validation performed.
- Risks or follow-ups.
- Proposed commit message.
- Proposed PR title.
- Proposed PR description.
- Files proposed for staging.

## Approval Gate

Proceed only after explicit user confirmation such as:

- "确认，按这个提交"
- "可以提交"
- "create the PR"
- "commit and push"

If the user asks for edits to the draft, update the draft and ask for
confirmation again.

## Publish Rules

After confirmation:

- Stage only the confirmed files.
- Avoid `git add -A` unless the user confirms the whole worktree belongs in
  scope.
- Commit with the approved message.
- Push to a branch.
- Open a draft PR by default.
- Use a ready-for-review PR only when explicitly requested.

If using the GitHub plugin publish flow, follow its write-safety rules. Stop and
explain blockers for missing auth, missing remote, inaccessible repository, or
failed validation.

## PR Body Contract

Use Markdown with these sections:

```markdown
## Summary

## Why

## Validation

## Risks / Follow-ups
```

Keep the PR body factual and aligned with the inspected diff.
