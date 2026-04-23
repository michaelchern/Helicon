# Agent Context: Testing

Human source: `docs/zh/testing.md`. Use this file to understand that validation
is currently conditional, not mandatory.

## Current State

- There is no stable test baseline yet.
- Daily iteration may intentionally skip tests while the framework, kept code,
  and directory boundaries are still in flux.
- Old test assumptions should not be treated as active requirements.

## Reference Command

```powershell
powershell -ExecutionPolicy Bypass -File scripts/test.ps1
```

Usually paired with:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/build.ps1
```

Use these only when the user explicitly wants to restore or inspect the
validation workflow.

## What To Report

- Whether validation was run
- If skipped, why it was skipped
- What needs to stabilize before tests become meaningful again

## Better Validation During Exploration

- Check whether copied-in files still match the current plan
- Check directory ownership and naming consistency
- Record why a module should stay, change, or be removed
