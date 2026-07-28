# Registro inicial — SC01 Plus

**Data:** 28/07/2026  
**Objetivo:** confirmar a comunicação USB e identificar o hardware real antes de qualquer alteração de firmware.

## Placa identificada

- Fabricante: Panlee
- Modelo serigrafado: `ZX3D50CE08S-V16-USRC`
- PCB: `240221`
- Porta serial no computador de desenvolvimento: `COM3`

## Procedimento executado

1. Conectar a placa ao computador por cabo USB-C com dados.
2. Confirmar que o Windows criou a porta `COM3`.
3. Criar ambiente Python isolado para as ferramentas locais:

   ```powershell
   .\tools\bootstrap-dev.ps1
   ```

4. Executar consultas de identificação, sem escrita na Flash:

   ```powershell
   .\.venv\Scripts\python.exe -m esptool --port COM3 chip-id
   .\.venv\Scripts\python.exe -m esptool --port COM3 flash-id
   ```

## Resultado confirmado

| Item | Valor |
|---|---|
| Chip | ESP32-S3 QFN56, revisão v0.2 |
| CPU | Dual Core + LP Core, até 240 MHz |
| Rádio | Wi-Fi e Bluetooth LE 5 |
| Cristal | 40 MHz |
| Flash | 16 MB, Quad (4 linhas), 3,3 V |
| PSRAM | 2 MB incorporados |
| Interface de debug | USB-Serial/JTAG |
| MAC Wi-Fi | `74:4D:BD:57:3C:18` |
| Ferramenta | esptool 5.3.1 |

## Ambiente local criado

- Script de criação: `tools/bootstrap-dev.ps1`
- Dependências: `requirements-dev.txt`
- Ambiente virtual (não versionado): `.venv/`

Para selecionar outra versão de Python instalada no Windows:

```powershell
.\tools\bootstrap-dev.ps1 -PythonCommand "py -3.12" -Recreate
```

## Garantias desta etapa

Não foi executada nenhuma operação de gravação, apagamento ou alteração de firmware. As consultas do `esptool` apenas abriram a porta serial, identificaram o chip e leram as informações da Flash.

## Impacto no projeto

Os 16 MB de Flash permitem adotar desde o primeiro firmware uma tabela de partições com `OTA_0`, `OTA_1`, dados OTA, NVS e LittleFS. A definição final dos tamanhos será feita ao iniciar o firmware `SC01_DIAG`.

## Próxima etapa

Criar o projeto ESP-IDF `SC01_DIAG`, inicialmente para validar LCD, touch, Wi-Fi e periféricos, preservando a capacidade de recuperação via USB.
