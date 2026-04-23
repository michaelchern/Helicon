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
- When the repo currently has a meaningful build/test baseline and the change
  needs validation, prefer:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/build.ps1
powershell -ExecutionPolicy Bypass -File scripts/test.ps1
```

- If validation is intentionally skipped because the repo is still exploratory
  or the current build/test setup is in flux, say that explicitly in the draft.

## Draft Output

Before any commit or push, provide:

- Change summary.
- Why this change is needed.
- Validation performed.
- Risks or follow-ups.
- Proposed Conventional Commits subject.
- Proposed detailed commit body.
- Proposed PR title.
- Proposed PR description.
- Files proposed for staging.

## Commit Message Contract

Git commit messages must have:

- Subject: Conventional Commits first line.
- Body: detailed explanation of changes, affected files or modules, validation,
  and risks.

The subject must follow:

```text
<type>(<optional scope>): <subject>
```

Rules:

- `type` must be one of: `feat`, `fix`, `docs`, `style`, `refactor`, `perf`,
  `test`, `chore`.
- `type` must be lowercase English.
- `scope` is optional and should be concise when used.
- `subject` must be Chinese, imperative, present tense, and preferably under 50
  Chinese characters.

Examples:

- `refactor(layout): 重组硬件抽象层目录结构`
- `feat(shader): 增加着色器编译工具目标`
- `docs(architecture): 补充引擎集成说明`

The commit body should include:

```text
Summary:
- What changed.

Details:
- Files or modules changed.
- Important behavior or workflow changes.

Validation:
- Commands or checks run.
- If validation was skipped, explain why.

Risks:
- Remaining risks or follow-ups.
```

If no PR is created, the commit body still needs enough detail for Git history.
PR descriptions may reuse and polish the commit body as GitHub Markdown.

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
- Commit with the approved Conventional Commits subject and detailed body.
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
