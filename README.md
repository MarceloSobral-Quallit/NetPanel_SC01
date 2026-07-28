# NetPanel SC01

Firmware e serviços do NetPanel para a Smart Panlee SC01 Plus (`ZX3D50CE08S-V16-USRC`), baseada em ESP32-S3.

O projeto utiliza a SC01 como interface touchscreen e um servidor Linux como fonte de métricas, API e repositório OTA.

## Estado atual

- Hardware identificado: ESP32-S3, 16 MB de Flash e 2 MB de PSRAM.
- Comunicação USB validada em `COM3`.
- Ambiente Python local criado para diagnóstico com `esptool`.
- Próximo objetivo: firmware `SC01_DIAG v0.1.0` em ESP-IDF.

## Documentação

- [Plano técnico](SC01_PLUS_NETPANEL_PLANO.md)
- [Registro inicial da placa](docs/REGISTRO_INICIAL_SC01.md)
- [Roadmap](docs/ROADMAP.md)

## Ambiente de diagnóstico

```powershell
.\tools\bootstrap-dev.ps1
.\.venv\Scripts\python.exe -m esptool --port COM3 flash-id
```
