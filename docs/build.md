# Agent Context: Build

Human source: `docs/zh/build.md`. Use this file as guidance for an
in-transition build setup, not as a guarantee that the repo currently builds.

## Current State

- There is no stable build baseline yet.
- `scripts/build.ps1` and `CMakeLists.txt` are retained reference points.
- If the working tree intentionally removed or replaced files, build failure may
  simply mean the build layer has not been updated yet.

## Reference Command

```powershell
powershell -ExecutionPolicy Bypass -File scripts/build.ps1
```

Use it when the user explicitly wants to inspect, repair, or restore the build
workflow.

## Interpretation Rules

- Do not assume the repository must stay buildable during every iteration.
- Prefer the working tree and user intent over stale build scaffolding.
- Convert build details into hard rules only after the build flow is explicitly
  stabilized.

## What A Stable Build Doc Must Eventually Define

- Kept targets and directories
- Required external dependencies
- One canonical build entry point
- First failure checks
