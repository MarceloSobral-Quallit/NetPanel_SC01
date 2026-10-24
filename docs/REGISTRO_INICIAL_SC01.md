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

## Sucessos confirmados — continuação

| Data | Ação | Evidência | Estado |
|---|---|---|---|
| 28/07/2026 | Backup integral da Flash | Arquivo `backups/sc01-original-20260728.bin`, 16 MB, SHA-256 `3772C1BF7D6D2B713973212DDF5C671E3C844A13A8464F675343D9AED4E7F044` | Confirmado |
| 28/07/2026 | Restauração da imagem original | `esptool write-flash` concluiu a escrita dos 16 MB e verificou o hash de cada bloco | Confirmado |
| 28/07/2026 | Identificação da interface atual | ESP32-S3 respondeu em `COM3`; MAC confirmado como `74:4D:BD:57:3C:18` | Confirmado |
| 28/07/2026 | Diagnóstico de LCD compilado | Projeto `diag_direct` compilado com ESP-IDF 5.5.5; binário `sc01_direct_lcd_test.bin` gerado (217.008 bytes) | Confirmado |
| 28/07/2026 | Diagnóstico de LCD gravado | Bootloader, tabela de partições e aplicação gravados em `COM3`; hashes verificados pelo esptool | Confirmado |

O resultado visual das quatro faixas horizontais permanece pendente de confirmação na placa. Ele será registrado como sucesso, ou como diagnóstico, após a observação.

## Diagnóstico visual — LCD direto

| Data | Teste | Resultado observado | Conclusão |
|---|---|---|---|
| 28/07/2026 | `diag_direct` (ST7796UI, barramento i80) | Tela pisca com pixels coloridos, sem imagem ou faixas sólidas | Não confirmado. O backlight e a atividade no barramento estão presentes, porém a configuração de inicialização, ordem do barramento ou controlador efetivo ainda não foi validada. |

Este resultado não é considerado um sucesso de LCD. A imagem original continua preservada em backup para restauração após os testes.

## Sucesso confirmado — restauração após diagnóstico

| Data | Ação | Evidência | Estado |
|---|---|---|---|
| 28/07/2026 | Restauração integral do firmware original | Backup de 16 MB regravado de `0x000000` a `0xFFFFFF` em quatro grupos; cada bloco de 256 KB recebeu confirmação `Hash of data verified` do esptool | Confirmado |

Backup usado: `backups/sc01-original-20260728.bin` (SHA-256 `3772C1BF7D6D2B713973212DDF5C671E3C844A13A8464F675343D9AED4E7F044`).

| Data | Validação na placa | Resultado | Estado |
|---|---|---|---|
| 28/07/2026 | Inicialização após a restauração | Tela retornou à interface do firmware original | Confirmado pelo operador |
