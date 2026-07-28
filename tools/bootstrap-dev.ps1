[CmdletBinding()]
param(
    # Informe, por exemplo, "py -3.12" ou o caminho completo de outro Python.
    [string]$PythonCommand = "python",
    [string]$VenvPath = ".venv",
    [switch]$Recreate,
    [switch]$SkipInstall
)

$ErrorActionPreference = "Stop"
$projectRoot = Split-Path -Parent $PSScriptRoot
$venvFullPath = Join-Path $projectRoot $VenvPath
$requirementsFile = Join-Path $projectRoot "requirements-dev.txt"

function Invoke-PythonCommand {
    param([string[]]$Arguments)

    # Permite comandos simples, como "python", e caminhos absolutos para python.exe.
    $commandParts = $PythonCommand -split '\s+'
    & $commandParts[0] @($commandParts | Select-Object -Skip 1) @Arguments
    if ($LASTEXITCODE -ne 0) {
        throw "Falha ao executar: $PythonCommand $($Arguments -join ' ')"
    }
}

if ($Recreate -and (Test-Path -LiteralPath $venvFullPath)) {
    Remove-Item -LiteralPath $venvFullPath -Recurse -Force
}

if (-not (Test-Path -LiteralPath $venvFullPath)) {
    Write-Host "Criando ambiente virtual em $venvFullPath"
    Invoke-PythonCommand -Arguments @("-m", "venv", $venvFullPath)
}

$venvPython = Join-Path $venvFullPath "Scripts\\python.exe"
if (-not (Test-Path -LiteralPath $venvPython)) {
    throw "Python do ambiente virtual não encontrado: $venvPython"
}

& $venvPython --version
if ($LASTEXITCODE -ne 0) { throw "Não foi possível consultar a versão do Python do ambiente." }

if (-not $SkipInstall) {
    Write-Host "Atualizando pip e instalando as ferramentas de desenvolvimento"
    & $venvPython -m pip install --upgrade pip
    if ($LASTEXITCODE -ne 0) { throw "Falha ao atualizar pip." }

    & $venvPython -m pip install -r $requirementsFile
    if ($LASTEXITCODE -ne 0) { throw "Falha ao instalar as dependências." }
}

Write-Host "Ambiente pronto. Ative-o com: .\\.venv\\Scripts\\Activate.ps1"
Write-Host "Para consultar a SC01: .\\.venv\\Scripts\\python.exe -m esptool --port COM3 chip-id"
