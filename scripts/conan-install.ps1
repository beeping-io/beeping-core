#!/usr/bin/env pwsh
# Install Conan dependencies for beeping-core on Windows.
#
# Usage:
#   .\scripts\conan-install.ps1            # Debug build (default)
#   .\scripts\conan-install.ps1 Release    # Release build

[CmdletBinding()]
param(
    [Parameter(Position = 0)]
    [string]$BuildType = "Debug"
)

$ErrorActionPreference = "Stop"

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$RepoRoot = (Resolve-Path (Join-Path $ScriptDir "..")).Path

if (-not (Get-Command conan -ErrorAction SilentlyContinue)) {
    Write-Error "❌ conan not found. Install it first: pipx install conan"
    exit 1
}

$ConanVersion = (conan --version) -replace ".*\s(\d)", '$1'
$ConanMajor = [int]($ConanVersion -split "\.")[0]
if ($ConanMajor -lt 2) {
    Write-Error "❌ Conan 2.x required, found $ConanVersion"
    exit 1
}

# Pick profile by architecture
$Arch = $env:PROCESSOR_ARCHITECTURE
if ($Arch -eq "ARM64") {
    $Profile = Join-Path $RepoRoot "profiles/windows-arm64"
} else {
    $Profile = Join-Path $RepoRoot "profiles/windows-x64"
}

Push-Location $RepoRoot
try {
    conan install . `
        --build=missing `
        --lockfile=conan.lock `
        -of "build/$BuildType" `
        "-pr:h=$Profile" `
        "-pr:b=$Profile" `
        "-s" "build_type=$BuildType"
    Write-Host "✅ Conan deps installed for build_type=$BuildType ($Arch)" -ForegroundColor Green
} finally {
    Pop-Location
}
