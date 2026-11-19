# Puts the STOCK Keychron firmware back.
# Usage: restore-stock.cmd            (uses backup\MI_FIRMWARE_ORIGINAL.bin)
#        restore-stock.cmd ansi|iso   (forces the ANSI or ISO stock version)
# The keyboard must be in bootloader mode (BOOT pads shorted, or Fn + Esc with QMK).
param([string]$Layout)
. (Join-Path $PSScriptRoot 'common.ps1')

switch ("$Layout".ToLowerInvariant()) {
    'ansi' { $file = Join-Path $Backup $Config['STOCK_ANSI_FILE']; $sha = $Config['STOCK_ANSI_SHA256'] }
    'iso'  { $file = Join-Path $Backup $Config['STOCK_ISO_FILE'];  $sha = $Config['STOCK_ISO_SHA256'] }
    ''     { $file = Join-Path $Backup 'MI_FIRMWARE_ORIGINAL.bin'; $sha = $null }
    default { Fail "Usage: restore-stock.cmd [ansi|iso]" }
}

if (-not (Test-Path -LiteralPath $file)) { Fail "$file doesn't exist. Run backup-stock.cmd first (or pass ansi|iso)." }
if ($sha -and (Get-Sha256 $file) -ne $sha) { Fail "Wrong SHA-256 in $file" }

& (Join-Path $PSScriptRoot 'flash.ps1') -File $file
exit $LASTEXITCODE
