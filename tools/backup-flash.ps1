[CmdletBinding()]
param(
    [string]$Port = "COM3",
    [string]$Python = ".\.venv\Scripts\python.exe",
    [string]$OutputFile = "backups\sc01-original.bin",
    [int]$Retries = 3
)

$ErrorActionPreference = "Stop"
$projectRoot = Split-Path -Parent $PSScriptRoot
$pythonPath = Join-Path $projectRoot $Python
$outputPath = Join-Path $projectRoot $OutputFile
$chunksPath = Join-Path $projectRoot "backups\chunks"
$flashSize = 0x1000000 # 16 MB, confirmado por esptool
$blockSize = 0x40000  # 256 KB: evita a interrupção observada nas leituras longas

if (-not (Test-Path -LiteralPath $pythonPath)) {
    throw "Python do ambiente não encontrado: $pythonPath"
}

New-Item -ItemType Directory -Force -Path $chunksPath | Out-Null

for ($offset = 0; $offset -lt $flashSize; $offset += $blockSize) {
    $chunkName = "{0:X8}.bin" -f $offset
    $chunkPath = Join-Path $chunksPath $chunkName
    $success = $false

    for ($attempt = 1; $attempt -le $Retries -and -not $success; $attempt++) {
        Remove-Item -LiteralPath $chunkPath -Force -ErrorAction SilentlyContinue
        Write-Host ("Lendo bloco {0}/{1}: 0x{2:X8}" -f (($offset / $blockSize) + 1), ($flashSize / $blockSize), $offset)
        & $pythonPath -m esptool --port $Port --baud 115200 --no-stub read-flash ("0x{0:X}" -f $offset) ("0x{0:X}" -f $blockSize) $chunkPath
        $success = ($LASTEXITCODE -eq 0) -and (Test-Path -LiteralPath $chunkPath) -and ((Get-Item -LiteralPath $chunkPath).Length -eq $blockSize)
    }

    if (-not $success) {
        throw "Não foi possível ler o bloco 0x{0:X8} após {1} tentativas." -f $offset, $Retries
    }
}

$outputDirectory = Split-Path -Parent $outputPath
New-Item -ItemType Directory -Force -Path $outputDirectory | Out-Null
$outputStream = [System.IO.File]::Open($outputPath, [System.IO.FileMode]::Create, [System.IO.FileAccess]::Write)
try {
    for ($offset = 0; $offset -lt $flashSize; $offset += $blockSize) {
        $chunkPath = Join-Path $chunksPath ("{0:X8}.bin" -f $offset)
        $chunkStream = [System.IO.File]::OpenRead($chunkPath)
        try { $chunkStream.CopyTo($outputStream) }
        finally { $chunkStream.Dispose() }
    }
}
finally { $outputStream.Dispose() }

if ((Get-Item -LiteralPath $outputPath).Length -ne $flashSize) {
    throw "Backup consolidado não possui 16 MB."
}

Get-FileHash -LiteralPath $outputPath -Algorithm SHA256
Write-Host "Backup concluído: $outputPath"
