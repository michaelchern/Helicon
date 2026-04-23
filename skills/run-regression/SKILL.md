---
name: run-regression
description: Run the standard build and regression checks for this repository, then summarize failures and likely causes.
---

# Purpose
Use this skill whenever code has changed and validation is required.

# Steps
1. Read docs/testing.md
2. Run the standard build script
3. Run the standard test script
4. If a test fails, identify:
   - failing command
   - failing test
   - likely cause
   - smallest next debugging step
5. Summarize results clearly

# Commands
- powershell -ExecutionPolicy Bypass -File scripts/build.ps1
- powershell -ExecutionPolicy Bypass -File scripts/test.ps1

# Output format
- Build status
- Test status
- Failures
- Likely cause
- Next action
