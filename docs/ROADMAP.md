# Roadmap — NetPanel SC01 Plus

**Estado atual:** hardware, USB, Flash e PSRAM identificados; ESP-IDF 5.5.5 instalado; backup/restauração integral validados. A inicialização do LCD ainda não está validada. Consulte também `docs/TENTATIVAS_E_ERROS.md`.

## Marco 0 — Fundação do projeto

**Objetivo:** tornar o desenvolvimento reproduzível.

- [x] Identificar a placa: Panlee SC01 Plus `ZX3D50CE08S-V16-USRC`.
- [x] Validar comunicação USB em `COM3`.
- [x] Confirmar ESP32-S3, 16 MB Flash e 2 MB PSRAM.
- [x] Criar ambiente virtual com `esptool`.
- [x] Inicializar repositório Git e publicar a branch `main` no GitHub.
- [x] Instalar e validar ESP-IDF 5.5.5.
- [ ] Definir versão inicial do firmware: `0.1.0`.

**Aceite:** qualquer computador de desenvolvimento consegue recriar o ambiente e consultar a placa.

## Marco 1 — SC01_DIAG via USB

**Objetivo:** validar o hardware real sem depender do servidor NetPanel.

- [x] Criar projetos ESP-IDF experimentais para diagnóstico via USB.
- [x] Criar e validar backup integral de 16 MB; validar restauração e retorno visual ao firmware original.
- [ ] Criar tabela de partições preparada para OTA A/B.
- [ ] Obter e validar a sequência de inicialização específica do LCD antes de outro teste de tela.
- [ ] Inicializar LCD e backlight com imagem estável.
- [ ] Inicializar touch e exibir coordenadas.
- [ ] Exibir chip, Flash, PSRAM, heap, MAC e versão.
- [ ] Conectar ao Wi-Fi por configuração temporária.
- [ ] Validar microSD, alto-falante e RS-485 quando aplicável.
- [ ] Registrar logs pela USB serial.

**Aceite:** tela de diagnóstico estável, touch funcional e dados de hardware exibidos.

## Marco 2 — Provisionamento e rede

**Objetivo:** inserir a SC01 na rede sem credenciais embutidas no firmware.

- [ ] Definir SSID e VLAN da SC01.
- [ ] Reservar endereço DHCP.
- [ ] Criar regra mínima: SC01 → Dell (API/OTA), DNS e NTP.
- [ ] Implementar captive portal e persistência NVS.
- [ ] Implementar modo offline e reconexão sem reinício em loop.

**Aceite:** uma SC01 nova é configurada pelo portal e volta a conectar após reinicialização.

## Marco 3 — OTA seguro na LAN

**Objetivo:** atualizar o firmware por Wi-Fi, mantendo recuperação por USB.

- [ ] Criar partições `ota_0`, `ota_1`, `otadata`, NVS e LittleFS.
- [ ] Criar manifesto de firmware com versão, tamanho e SHA-256.
- [ ] Hospedar manifesto e binário no Dell.
- [ ] Implementar OTA pull na SC01.
- [ ] Validar imagem após boot e confirmar somente após LCD, Wi-Fi e UI iniciarem.
- [ ] Validar rollback intencional para a versão anterior.

**Aceite:** atualização e rollback funcionam sem USB; USB permanece como recuperação.

## Marco 4 — NetPanel API mínima

**Objetivo:** fornecer dados normalizados para a SC01.

- [ ] Preparar FastAPI no Dell.
- [ ] Criar `GET /netpanel/api/status`.
- [ ] Coletar ping dos equipamentos definidos.
- [ ] Coletar CPU, RAM, temperatura e uptime do Dell com `psutil`.
- [ ] Adicionar cache, timeout e timestamp de última leitura válida.
- [ ] Publicar a API via Nginx e systemd.

**Aceite:** endpoint JSON estável e acessível pela SC01 a cada 5 segundos.

## Marco 5 — Dashboard MVP

**Objetivo:** entregar o painel útil inicial.

- [ ] Criar UI LVGL com Home, Rede e Sistema.
- [ ] Exibir hora, Wi-Fi e estado de comunicação.
- [ ] Exibir ER605, OC200, SG2008P, EAP650 e Dell.
- [ ] Exibir CPU, RAM e temperatura do Dell.
- [ ] Atualizar apenas widgets que tenham mudado.
- [ ] Exibir último estado conhecido se a API estiver indisponível.

**Aceite:** painel responsivo, legível e operacional mesmo com API temporariamente offline.

## Marco 6 — Métricas e alertas

**Objetivo:** aumentar a cobertura sem levar coleta pesada ao ESP32.

- [ ] Adicionar Raspberry Pi, serviços e discos.
- [ ] Integrar coletores SNMP/HTTP/Omada no Dell quando necessário.
- [ ] Adicionar status das duas conexões WAN e último speedtest.
- [ ] Definir limites de alerta e horário silencioso.
- [ ] Adicionar alertas visuais e, se desejado, áudio.

**Aceite:** falhas relevantes são exibidas de forma clara e sem sobrecarregar a rede.

## Marco 7 — Histórico e endurecimento

**Objetivo:** preparar uso contínuo.

- [ ] Persistir histórico inicialmente em SQLite.
- [ ] Exibir gráficos de 24 h, 7 d e 30 d sob demanda.
- [ ] Adicionar HTTPS e autenticação entre SC01 e Dell.
- [ ] Avaliar assinatura de firmware e anti-downgrade.
- [ ] Documentar backup e recuperação completa por USB.

**Aceite:** operação segura e recuperável, com histórico confiável no Dell.

## Decisões pendentes

| Tema | Decisão necessária antes do marco |
|---|---|
| LCD | Fonte, firmware ou documentação oficial com sequência de inicialização do painel | 1 |
| Rede | SSID, VLAN e regra de firewall | 2 |
| Dell | Endereço, porta e HTTP/HTTPS inicial | 3 e 4 |
| Interface | Tema, equipamentos visíveis e alertas | 5 |

## Próxima ação recomendada

Concluir a identificação do LCD: localizar fonte ou firmware oficial da Panlee/WT32-SC01 Plus que revele o controlador, a pinagem e a sequência de inicialização. Só então gerar um novo diagnóstico de tela, com o backup original como ponto de retorno.
