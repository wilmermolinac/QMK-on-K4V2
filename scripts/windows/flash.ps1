# Flashes a .bin to the K4 V2 (the keyboard must be in bootloader mode).
# Usage: flash.cmd <file.bin>     (or drag the .bin onto flash.cmd)
#        flash.cmd                (asks you to pick the file)
param([string]$File)
. (Join-Path $PSScriptRoot 'common.ps1')

if (-not $File) {
    try {
        Add-Type -AssemblyName System.Windows.Forms
        $dlg = New-Object System.Windows.Forms.OpenFileDialog
        $dlg.Title = 'Pick the K4 V2 firmware (.bin) to flash'
        $dlg.Filter = 'Firmware (*.bin)|*.bin'
        $dlg.InitialDirectory = $Root
        if ($dlg.ShowDialog() -eq 'OK') { $File = $dlg.FileName }
    } catch {
        $File = Read-Host 'Path to the .bin file'
    }
}
if (-not $File -or -not (Test-Path -LiteralPath $File)) { Fail "Usage: flash.cmd <file.bin>" }
$File = (Resolve-Path -LiteralPath $File).Path
if ((Get-Item -LiteralPath $File).Length -gt 65536) { Fail "$File is bigger than 64 KB; that's not a K4 V2 firmware." }

Initialize-Flasher
Write-Host ""
Write-Host "File: $File"
Write-Host "      checksum $(Get-ImageChecksum $File)"
Confirm-Yes "Flash this file to the keyboard?"

Wait-Bootloader
Invoke-Flasher -v $VidPid -f $File | ForEach-Object { Write-Host $_ }
if ($LASTEXITCODE -ne 0) { Fail "sonixflasher failed (exit code $LASTEXITCODE). Nothing is broken: enter the bootloader again and retry." }

Write-Host ""
Write-Host "Done. Unplug the keyboard and plug it back in (without shorting BOOT)." -ForegroundColor Green
