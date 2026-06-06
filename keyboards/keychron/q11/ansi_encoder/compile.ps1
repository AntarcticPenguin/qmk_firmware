# 编译 Keychron Q11 固件，输出到 qmk_firmware/firmware/keychron_q11/
$ErrorActionPreference = "Stop"

$FirmwareRoot = "C:\Tools\qmk\qmk_firmware"
$KeyboardName = "keychron_q11"
$OutputDir    = Join-Path $FirmwareRoot "firmware\$KeyboardName"
$ArchiveDir   = Join-Path $OutputDir "archive"
$SourceBin    = Join-Path $FirmwareRoot "keychron_q11_ansi_encoder_default.bin"

Write-Host "Compiling keychron/q11/ansi_encoder:default ..." -ForegroundColor Cyan
& C:\QMK_MSYS\shell_connector.cmd -lc "cd /c/Tools/qmk/qmk_firmware && qmk compile -kb keychron/q11/ansi_encoder -km default"
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

if (-not (Test-Path $SourceBin)) {
    Write-Error "Compiled bin not found: $SourceBin"
}

New-Item -ItemType Directory -Force -Path $ArchiveDir | Out-Null

Copy-Item $SourceBin (Join-Path $OutputDir "latest.bin") -Force
$Stamp = Get-Date -Format "yyyy-MM-dd_HHmmss"
$ArchiveName = "keychron_q11_$Stamp.bin"
Copy-Item $SourceBin (Join-Path $ArchiveDir $ArchiveName) -Force

# 编译产物留在 QMK 根目录是默认行为，移入 firmware/ 后删除根目录副本
Remove-Item $SourceBin -Force -ErrorAction SilentlyContinue

Write-Host ""
Write-Host "Firmware ready:" -ForegroundColor Green
Write-Host "  Flash this -> $OutputDir\latest.bin"
Write-Host "  Archive    -> $ArchiveDir\$ArchiveName"
