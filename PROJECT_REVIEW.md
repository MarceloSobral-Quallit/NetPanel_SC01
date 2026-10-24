# Revisão do Projeto — NetPanel SC01

Data: 28/07/2026

## Estado atual

O repositório contém documentação, ferramentas de diagnóstico e um firmware `SC01_BOOTSTRAP` ainda baseado no exemplo `simple_ota_example` do ESP-IDF. O firmware não foi gravado na SC01. Há backup integral da Flash local, ignorado pelo Git.

## Arquitetura encontrada

- Raiz: documentação e scripts PowerShell de ambiente/backup.
- `firmware/`: projeto CMake ESP-IDF, um componente `main` e dependência local `protocol_examples_common` do ESP-IDF.
- Firmware: Wi-Fi em modo station, OTA HTTPS imediato após conexão e duas partições OTA.
- Não há servidor NetPanel, BSP, interface gráfica ou driver de periféricos no repositório.

## Verificações solicitadas

| Item | Resultado |
|---|---|
| ESP-IDF | ESP-IDF 5.5.5 instalado localmente em `.tools/esp-idf`; não fixado por submódulo/lock do projeto. |
| Target | `firmware/sdkconfig` atual: `esp32s3`. |
| Flash / PSRAM | Flash configurada para 16 MB. PSRAM não está habilitada no `sdkconfig`, apesar do hardware ter 2 MB. |
| BSP | Ausente. Não há componente específico para Panlee SC01 Plus. |
| LCD / touch | Ausentes: nenhum driver, pinagem ou inicialização localizada. |
| GPIOs | Não há GPIOs da SC01 no código. GPIOs Ethernet em arquivos CI são herdados do exemplo e não se aplicam à placa. |
| LVGL | Ausente. |
| Wi-Fi | Presente via `protocol_examples_common`; SSID/senha foram aplicados apenas ao `sdkconfig` local ignorado. |
| OTA | Presente via `esp_https_ota`, duas partições e rollback habilitado. Não há manifesto, verificação SHA-256 independente, servidor, interface de controle nem confirmação explícita da imagem nova. |
| Código duplicado | Inclusão duplicada de `protocol_examples_common.h` em `simple_ota_example.c`. Arquivos de teste/configuração CI herdados não foram ajustados ao projeto. |
| Operações bloqueantes | OTA roda em task própria. `esp_https_ota()` é bloqueante nessa task; não há LVGL hoje. Quando LVGL existir, ele deverá ficar em task separada e nunca chamar OTA nela. |
| Credenciais versionadas | Nenhuma encontrada em arquivos rastreados. `firmware/sdkconfig` contém credenciais locais, mas é ignorado pelo Git. |
| Dependências | `requirements-dev.txt` usa faixa `esptool>=5.0,<6.0`; não há lock. A dependência ESP-IDF é um caminho local, sem versão fixada no repositório. |
| Build reproduzível | Parcial. Compila localmente somente com ESP-IDF instalado manualmente e `SDKCONFIG_DEFAULTS` apontando para arquivo privado. Falta script de build, versão ESP-IDF declarada e configuração privada de exemplo. |

## Análise de `partitions.csv`

`ota_0` e `ota_1` possuem 0x680000 bytes cada e suportam imagens grandes. `nvs`, `otadata`, `phy_init` e `littlefs` completam exatamente 16 MB. A tabela é coerente com a Flash detectada e reserva espaço para rollback. O rótulo `littlefs` usa subtipo `spiffs`; isso é aceitável apenas se o projeto montar SPIFFS, não LittleFS.

## Problemas e riscos

| Severidade | Achado | Correção proposta |
|---|---|---|
| CRITICAL | Não há BSP/LCD/touch; o firmware não prova funcionamento da SC01 como painel. | Adicionar BSP validado e diagnóstico de LCD/touch antes de UI. |
| HIGH | O exemplo OTA inicia download automaticamente e não confirma `esp_ota_mark_app_valid_cancel_rollback()` após health check. | Implementar estado de saúde, confirmação tardia e fluxo explícito de update. |
| HIGH | `sdkconfig` local contém credencial Wi-Fi e pode vazar se a regra de ignore for removida/forçada. | Usar provisioning em NVS ou secret store local; manter arquivo privado fora de commits. |
| HIGH | PSRAM física não está habilitada. | Habilitar e validar PSRAM no `SC01_DIAG` antes de LVGL. |
| MEDIUM | Projeto herdou testes, nomes, comentários e CI do exemplo OTA, incluindo referências a ESP32 e offsets antigos. | Limpar/adequar somente após revisão aprovada. |
| MEDIUM | Dependências não são completamente fixadas. | Declarar release ESP-IDF, script de bootstrap e lock de ferramentas. |
| LOW | Include duplicado e nomenclatura `simple_ota_example`. | Corrigir em refatoração posterior. |

## Sequência recomendada

1. Corrigir a base de build para target ESP32-S3 e registrar ESP-IDF 5.5.5.
2. Criar `SC01_DIAG` com BSP, LCD, touch, PSRAM e logs USB.
3. Validar partições OTA e o boot do firmware mínimo pela USB.
4. Implementar provisioning Wi-Fi em NVS, sem credenciais compiladas.
5. Implementar OTA pull com manifesto, SHA-256, health check e rollback confirmado.
6. Validar uma atualização real e rollback controlado.
7. Somente então adicionar LVGL e a UI NetPanel.

## Resumo final

- **BUILD STATUS:** parcial; uma compilação local foi possível, mas a última recompilação para ESP32-S3 foi interrompida pela solicitação de revisão.
- **HARDWARE STATUS:** ESP32-S3, 16 MB Flash e 2 MB PSRAM confirmados por leitura; backup completo disponível localmente.
- **LCD STATUS:** não implementado/configurado.
- **TOUCH STATUS:** não implementado/configurado.
- **WIFI STATUS:** suporte station herdado do exemplo; configuração local presente, sem provisioning.
- **OTA STATUS:** HTTPS OTA e tabela A/B presentes; health check/manifesto/validação de rollback ainda ausentes.
- **NEXT RECOMMENDED STEP:** aprovar a correção da base do firmware para ESP32-S3 e iniciar `SC01_DIAG` com BSP antes de qualquer flash.
