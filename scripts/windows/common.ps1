# Shared helpers for the Windows scripts (dot-sourced, not run directly).
# Works on Windows PowerShell 5.1 and PowerShell 7. Keep this file ASCII-only:
# PowerShell 5.1 reads scripts without a BOM as ANSI.

$ErrorActionPreference = 'Stop'
try { [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12 } catch { }

# Repo (or release kit) root: scripts\windows\.. \..
$Root   = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$Build  = Join-Path $Root 'build'
$Backup = Join-Path $Root 'backup'

# Settings shared with the macOS scripts live in scripts/common.sh.
$Config = @{}
$commonSh = Join-Path $Root 'scripts\common.sh'
if (-not (Test-Path $commonSh)) { throw "Missing $commonSh" }
foreach ($line in Get-Content $commonSh) {
    if ($line -match '^([A-Z0-9_]+)="([^"$]*)"\s*$') { $Config[$Matches[1]] = $Matches[2] }
}

$FlasherDir = Join-Path $Build 'tools\sonixflasher-win'
$Flasher    = Join-Path $FlasherDir 'sonixflasher.exe'
if ($env:K4V2_SONIXFLASHER) { $Flasher = $env:K4V2_SONIXFLASHER }   # override (testing)
$VidPid     = $Config['BOOTLOADER_VIDPID']

function Fail([string]$msg) {
    Write-Host ""
    Write-Host "ERROR: $msg" -ForegroundColor Red
    exit 1
}

function Get-Sha256([string]$path) {
    (Get-FileHash -Algorithm SHA256 -LiteralPath $path).Hash.ToLowerInvariant()
}

function Save-Url([string]$url, [string]$out) {
    $old = $ProgressPreference
    $ProgressPreference = 'SilentlyContinue'   # much faster on PowerShell 5.1
    try { Invoke-WebRequest -UseBasicParsing -Uri $url -OutFile $out }
    finally { $ProgressPreference = $old }
}

# 16-bit little-endian word sum of a 64 KB image (padded with 0xFF), the same
# value the Sonix bootloader reports as "Flash checksum".
function Get-ImageChecksum([string]$path) {
    $data = [IO.File]::ReadAllBytes((Resolve-Path -LiteralPath $path).Path)
    $sum = 0
    for ($i = 0; $i -lt 65536; $i += 2) {
        $lo = 0xFF; $hi = 0xFF
        # [int]: shifting a [byte] left by 8 would overflow to 0.
        if ($i -lt $data.Length) { $lo = [int]$data[$i] }
        if ($i + 1 -lt $data.Length) { $hi = [int]$data[$i + 1] }
        $sum = ($sum + $lo + ($hi -shl 8)) -band 0xFFFF
    }
    '0x{0:x4}' -f $sum
}

# Downloads the official SonixFlasherC build for Windows on first use.
function Initialize-Flasher {
    if (Test-Path -LiteralPath $Flasher) { return }
    Write-Host "==> Downloading SonixFlasherC for Windows (first run only)"
    New-Item -ItemType Directory -Force -Path $FlasherDir | Out-Null
    $zip = Join-Path $FlasherDir 'sonixflasher.zip'
    Save-Url $Config['SFC_WIN_URL'] $zip
    if ((Get-Sha256 $zip) -ne $Config['SFC_WIN_SHA256']) {
        Remove-Item -Force $zip
        Fail "The downloaded SonixFlasherC has the wrong SHA-256. Try again later."
    }
    Expand-Archive -Force -LiteralPath $zip -DestinationPath $FlasherDir
    Remove-Item -Force $zip
    if (-not (Test-Path -LiteralPath $Flasher)) { Fail "sonixflasher.exe not found after extracting." }
    Write-Host "   OK  $Flasher"
}

# Runs sonixflasher and returns its output lines (stdout + stderr) without
# PowerShell turning stderr into exceptions. Exit code is in $LASTEXITCODE.
function Invoke-Flasher {
    $old = $ErrorActionPreference
    $ErrorActionPreference = 'Continue'
    try { $out = & $Flasher @args 2>&1 | ForEach-Object { "$_" } }
    finally { $ErrorActionPreference = $old }
    return $out
}

function Wait-Bootloader {
    Write-Host "Looking for the keyboard in bootloader mode ($VidPid)..."
    for ($i = 0; $i -lt 30; $i++) {
        $list = (Invoke-Flasher -c) -join "`n"
        if ($list -match '7040') { return }
        Start-Sleep -Seconds 1
    }
    Fail ("Bootloader not found. Unplug the keyboard, short the BOOT pads under the space bar " +
          "and plug it back in (or press Fn + Esc if this firmware is already installed). " +
          "If it still isn't found, see 'Windows driver' in the guide.")
}

function Confirm-Yes([string]$question) {
    $answer = Read-Host "$question Type 'yes' (or 'si') to continue"
    if (@('yes', 'y', 'si', 's') -notcontains $answer.Trim().ToLowerInvariant()) { Fail "Cancelled." }
}
