# Agent Context: Testing

Human source: `docs/zh/testing.md`. Use this file for the standard validation
entry point and failure triage.

## Canonical Command

```powershell
powershell -ExecutionPolicy Bypass -File scripts/test.ps1
```

Run `scripts/build.ps1` first when build outputs may be missing or stale.

## Test Contract

- Test entry point: `tests/helicon_test.c`
- Test runner: CTest
- Build directory: `build/`
- Expected local and CI entry point: `scripts/test.ps1`

## Pass Criteria

- `scripts/build.ps1` succeeds.
- `scripts/test.ps1` reports all tests passed.

## Failure Triage

1. Re-run `scripts/build.ps1`.
2. Re-run `scripts/test.ps1`.
3. Inspect the first failing CTest output.
4. Fix the smallest affected code area.
5. Add or update tests when behavior changes.

## Script Failure Routing

| Symptom | Check |
| --- | --- |
| `ctest` not found | Ensure CMake/CTest is installed and on `PATH`. |
| `build/` missing | Run `scripts/build.ps1` first. |
| test executable missing | Check build output and `CMakeLists.txt`. |
| assertion or process failure | Inspect `tests/` and the implementation under `src/`. |
