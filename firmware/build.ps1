[CmdletBinding()]
param(
    [string]$Workspace,
    [string]$ZmkSource,
    [string]$West = "west",
    [string]$BuildDirectory
)

$ErrorActionPreference = "Stop"
$repoRoot = Split-Path -Parent $PSScriptRoot
$pinnedZmkCommit = "ead91ca094c6cd778574f73b9ae011ddab611c66"

if (-not $Workspace) {
    $Workspace = Split-Path -Parent $repoRoot
}
if (-not $ZmkSource) {
    $ZmkSource = Join-Path $Workspace "zmk"
}
if (-not $BuildDirectory) {
    $BuildDirectory = Join-Path $repoRoot ".build/tac-w"
}

$Workspace = (Resolve-Path -LiteralPath $Workspace).Path
$ZmkSource = (Resolve-Path -LiteralPath $ZmkSource).Path
$configPath = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot "config")).Path
$patchPath = Join-Path $PSScriptRoot "patches/tac-w-zmk.patch"

$sourceCommit = & git -C $ZmkSource rev-parse HEAD
if ($LASTEXITCODE -ne 0 -or $sourceCommit -ne $pinnedZmkCommit) {
    throw "ZMK must be checked out at $pinnedZmkCommit. Run west update in the pinned workspace."
}

& git -C $ZmkSource apply --reverse --check $patchPath 2>$null
if ($LASTEXITCODE -eq 0) {
    Write-Host "TAC-W ZMK patch is already applied."
} else {
    & git -C $ZmkSource apply --check $patchPath
    if ($LASTEXITCODE -ne 0) {
        throw "The TAC-W patch does not apply cleanly. Check local ZMK changes before building."
    }
    & git -C $ZmkSource apply $patchPath
    if ($LASTEXITCODE -ne 0) {
        throw "Could not apply the TAC-W ZMK patch."
    }
}

$overlayPaths = @(
    (Join-Path $configPath "boards/nicenano_nosd.overlay"),
    (Join-Path $configPath "boards/shields/wire_x/wire_x_st7789_test.overlay")
)
$buildArguments = @(
    "build", "-p", "always",
    "-b", "promicro_nrf52840/nrf52840/uf2",
    (Join-Path $ZmkSource "app"),
    "-d", $BuildDirectory,
    "--",
    "-DSHIELD=wire_x",
    "-DZMK_CONFIG=$configPath",
    "-DEXTRA_CONF_FILE=$(Join-Path $configPath 'wire_x_st7789_test.conf')",
    "-DEXTRA_DTC_OVERLAY_FILE=$($overlayPaths -join ';')",
    "-DWIRE_X_ST7789_TEST=ON"
)

Push-Location -LiteralPath $Workspace
try {
    & $West @buildArguments
    if ($LASTEXITCODE -ne 0) {
        throw "TAC-W firmware build failed."
    }
} finally {
    Pop-Location
}

$outputPath = Join-Path $PSScriptRoot "uf2/TAC-W.uf2"
Copy-Item -LiteralPath (Join-Path $BuildDirectory "zephyr/zmk.uf2") -Destination $outputPath
$checksum = (Get-FileHash -LiteralPath $outputPath -Algorithm SHA256).Hash.ToLowerInvariant()
"$checksum  TAC-W.uf2" | Set-Content -LiteralPath (Join-Path $PSScriptRoot "uf2/SHA256SUMS.local") -Encoding ascii
Write-Host "Firmware: $outputPath"
Write-Host "SHA-256: $checksum"
