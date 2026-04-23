Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
$buildDir = Join-Path $repoRoot "build"
$buildType = "RelWithDebInfo"

if (-not (Get-Command ctest -ErrorAction SilentlyContinue)) {
    throw "ctest is required but was not found in PATH."
}

if (-not (Test-Path $buildDir)) {
    throw "Build directory not found. Run scripts/build.ps1 first."
}

Write-Host "Running tests with CTest"
ctest --test-dir $buildDir -C $buildType --output-on-failure

Write-Host "Tests completed."

