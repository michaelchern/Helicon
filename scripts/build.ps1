Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
$buildDir = Join-Path $repoRoot "build"
$buildType = "RelWithDebInfo"

if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
    throw "cmake is required but was not found in PATH."
}

Write-Host "Configuring project in $buildDir"
cmake -S $repoRoot -B $buildDir -DCMAKE_BUILD_TYPE=$buildType

Write-Host "Building project"
cmake --build $buildDir --config $buildType

Write-Host "Build completed."

