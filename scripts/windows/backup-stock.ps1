# Backs up the STOCK firmware before installing QMK. Never writes to the keyboard.
#
# The SN32F248B bootloader can't read the flash back over USB, so:
#   1. Read the keyboard's flash checksum, protection level and chip info.
#   2. Download the stock K4 V2 firmware (ANSI v1.07 / ISO v1.01) and check its SHA-256.
#   3. If the keyboard's checksum matches one of them, that file is bit-for-bit
#      your firmware; it's copied to backup\MI_FIRMWARE_ORIGINAL.bin.
. (Join-Path $PSScriptRoot 'common.ps1')

Initialize-Flasher
New-Item -ItemType Directory -Force -Path $Backup | Out-Null
$stamp = Get-Date -Format 'yyyyMMdd-HHmmss'
$info  = Join-Path $Backup "device-info-$stamp.txt"

Wait-Bootloader
Write-Host "==> Reading chip info (read-only)"
$out = Invoke-Flasher -v $VidPid -i
$out | ForEach-Object { Write-Host $_ }
$out | Set-Content -LiteralPath $info
$devSum = $null
foreach ($l in $out) {
    if ($l -match 'Flash checksum:\s*(0x[0-9a-fA-F]{4})') { $devSum = $Matches[1].ToLowerInvariant(); break }
}

Write-Host ""
Write-Host "==> Downloading the stock K4 V2 firmware"
function Get-Stock([string]$name, [string]$sha) {
    $path = Join-Path $Backup $name
    if (-not (Test-Path -LiteralPath $path)) { Save-Url "$($Config['STOCK_BASE_URL'])/$name" $path }
    if ((Get-Sha256 $path) -ne $sha) { Fail "Wrong SHA-256 in $name" }
    Write-Host "   OK  $name  (checksum $(Get-ImageChecksum $path))"
    return $path
}
$ansi = Get-Stock $Config['STOCK_ANSI_FILE'] $Config['STOCK_ANSI_SHA256']
$iso  = Get-Stock $Config['STOCK_ISO_FILE']  $Config['STOCK_ISO_SHA256']
$mine = Join-Path $Backup 'MI_FIRMWARE_ORIGINAL.bin'

Write-Host ""
if (-not $devSum) {
    Write-Host "WARNING: the bootloader didn't report a checksum; nothing to compare." -ForegroundColor Yellow
} elseif ($devSum -eq $Config['STOCK_ANSI_CHECKSUM']) {
    Copy-Item -Force -LiteralPath $ansi -Destination $mine
    Write-Host "MATCH: your keyboard has exactly $($Config['STOCK_ANSI_FILE'])" -ForegroundColor Green
} elseif ($devSum -eq $Config['STOCK_ISO_CHECKSUM']) {
    Copy-Item -Force -LiteralPath $iso -Destination $mine
    Write-Host "MATCH: your keyboard has exactly $($Config['STOCK_ISO_FILE'])" -ForegroundColor Green
} else {
    # The stock firmware keeps lighting settings in two flash pages inside the
    # image, so the same firmware with other saved settings differs by whole high bytes.
    $dev = [Convert]::ToInt32($devSum.Substring(2), 16)
    foreach ($pair in @(@('ANSI', $Config['STOCK_ANSI_CHECKSUM']), @('ISO', $Config['STOCK_ISO_CHECKSUM']))) {
        $ref  = [Convert]::ToInt32($pair[1].Substring(2), 16)
        $diff = ($ref - $dev) -band 0xFFFF
        if ((($diff -band 0xFF) -eq 0) -and (($diff -le 0x2000) -or ($diff -ge 0xE000))) {
            Write-Host ("ALMOST A MATCH with {0} (difference 0x{1:x4}): most likely the same firmware" -f $pair[0], $diff) -ForegroundColor Yellow
            Write-Host "with different saved lighting settings. To confirm: leave bootloader mode, factory"
            Write-Host "reset the keyboard (Fn + J + Z for 3-5 s, in Cable mode), enter bootloader mode"
            Write-Host "again and run this script again."
        }
    }
    Write-Host "NO MATCH: your keyboard reports checksum $devSum." -ForegroundColor Yellow
    Write-Host "Your firmware version isn't in the database (another version or language)."
    Write-Host "Before flashing QMK, either:"
    Write-Host "  - download Keychron's official K4 V2 firmware updater and keep it to restore later, or"
    Write-Host "  - dump the flash with an ST-Link (https://github.com/SonixQMK/sonix_dumper)."
}
Write-Host ""
Write-Host "Backup folder: $Backup"
Write-Host "Chip info:     $info"
