# Agent Context: Testing

Human source: `docs/zh/testing.md`. Use this file for the standard validation
entry point and failure triage.

## Canonical Command

```powershell
powershell -ExecutionPolicy Bypass -File scripts/test.ps1
```

Run `scripts/build.ps1` first when build outputs may be missing or stale.

## Test Contract

- `tests/helicon_smoke_test.cpp`: C++ smoke test for version, status strings, and
  backend availability query.
- `tests/render_graph_test.cpp`: C++ RHI/render graph regression. When Vulkan is
  available, it creates a device, buffer, image, command list, clear-only pass,
  builtin triangle pass, and validates RGBA8 readback pixels.
- If no Vulkan device is available, the GPU portion skips and passes. Building
  still requires the Vulkan SDK.
- Test runner: CTest.
- Build directory: `build/`.
- Expected local and CI entry point: `scripts/test.ps1`.

## Pass Criteria

- `scripts/build.ps1` succeeds.
- `scripts/test.ps1` reports all tests passed.

## Failure Triage

1. Re-run `scripts/build.ps1`.
2. Re-run `scripts/test.ps1`.
3. Inspect the first failing CTest output.
4. For Vulkan initialization failures, check Vulkan SDK, drivers, and device
   availability.
5. Fix the smallest affected code area and update tests when behavior changes.
