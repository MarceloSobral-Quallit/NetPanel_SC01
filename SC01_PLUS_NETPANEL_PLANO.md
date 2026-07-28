# SC01 Plus / NetPanel
## Plano de infraestrutura, desenvolvimento e atualização OTA

**Documento:** SC01_PLUS_NETPANEL_PLANO.md  
**Versão:** 0.1  
**Data:** 28/07/2026  
**Status:** proposta técnica para análise

---

# 1. Objetivo

Criar um painel touchscreen baseado na **SC01 Plus / ZX3D50CE08S-V16-USRC**, usando o ESP32-S3 da placa como interface gráfica e um servidor Linux como núcleo de coleta e distribuição de dados.

O sistema deverá permitir:

- monitorar equipamentos de rede;
- monitorar servidores e Raspberry Pi;
- mostrar estado das conexões de Internet;
- apresentar CPU, memória, temperatura, disco, uptime e latência;
- exibir históricos e alertas;
- permitir atualização do firmware da SC01 Plus via Wi-Fi;
- manter a coleta pesada fora do ESP32;
- possibilitar evolução futura para MQTT, RS-485/Modbus e outros sensores.

A premissa principal da arquitetura é:

> **SC01 Plus = interface e comunicação**  
> **Dell = coleta, processamento, histórico, API e OTA**

---

# 2. Hardware identificado

A placa disponível foi identificada visualmente como:

```text
ZX3D50CE08S-V16-USRC
PCB: 240221
```

Família:

```text
Smart Panlee SC01 Plus
ESP32-S3
LCD 3,5"
480 x 320
Touch capacitivo
microSD
RS-485
Áudio
Wi-Fi
Bluetooth
```

## 2.1 Pontos que deverão ser confirmados pelo firmware de diagnóstico

Apesar da identificação da placa, o primeiro firmware deverá consultar o hardware real em tempo de execução e registrar:

- modelo/chip ESP32;
- revisão do chip;
- tamanho real da Flash;
- tamanho real da PSRAM;
- MAC Wi-Fi;
- disponibilidade do microSD;
- funcionamento do LCD;
- funcionamento do touch;
- backlight;
- áudio;
- RS-485;
- GPIOs disponíveis.

Não devemos depender exclusivamente da especificação comercial porque existem variantes dessa família.

---

# 3. Infraestrutura existente

A estrutura considerada neste projeto é:

| Equipamento | Endereço planejado | Função |
|---|---:|---|
| ER605 | 192.168.10.1 | Gateway / roteamento |
| OC200 | 192.168.10.2 | Controller Omada |
| TL-SG2008P | 192.168.10.3 | Switch gerenciável |
| EAP650 | 192.168.10.4 | Access Point |
| Dell Ubuntu | 192.168.10.5 | Servidor NetPanel |
| SC01 Plus | DHCP reservado | Display / cliente NetPanel |

A SC01 não deverá usar IP fixo gravado no firmware.

Preferência:

```text
DHCP + Address Reservation
```

Isso facilita:

- troca de VLAN;
- alteração de sub-rede;
- recuperação do equipamento;
- substituição da placa;
- manutenção do firmware.

---

# 4. Arquitetura geral

```text
                            INTERNET
                               |
                        +------+------+
                        |    ER605    |
                        |192.168.10.1 |
                        +------+------+
                               |
                        +------v------+
                        |  SG2008P    |
                        |192.168.10.3 |
                        +------+------+
                               |
                 +-------------+-------------+
                 |                           |
          +------v------+             +------v------+
          | Dell Ubuntu |             |   EAP650    |
          |192.168.10.5 |             |192.168.10.4 |
          +------+------+             +------+------+
                 |                           |
                 |                           | Wi-Fi 2.4 GHz
                 |                           |
                 |                    +------v------+
                 +------------------->| SC01 Plus  |
                      HTTP/HTTPS      | ESP32-S3   |
                                     +-------------+
```

O Dell será o ponto central.

A SC01 não deverá consultar diretamente todos os equipamentos de infraestrutura.

---

# 5. Funções do servidor Dell

O Dell deverá executar inicialmente:

```text
NetPanel API
Nginx
OTA repository
systemd
Python
```

Estrutura proposta:

```text
/opt/netpanel/
|
+-- api/
|   +-- netpanel.py
|   +-- collectors/
|   |   +-- hosts.py
|   |   +-- system.py
|   |   +-- ping.py
|   |   +-- network.py
|   |   +-- omada.py
|   |   +-- speedtest.py
|   |   +-- snmp.py
|   |
|   +-- config.yaml
|
+-- firmware/
|   +-- stable/
|   |   +-- manifest.json
|   |   +-- sc01.bin
|   |
|   +-- versions/
|       +-- sc01-0.1.0.bin
|       +-- sc01-0.1.1.bin
|
+-- logs/
|
+-- scripts/
|
+-- data/
```

---

# 6. NetPanel API

A SC01 deverá receber os dados por uma API simples.

Exemplo:

```http
GET /netpanel/api/status
```

Resposta aproximada:

```json
{
  "timestamp": "2026-07-28T16:30:00-03:00",

  "network": {
    "er605": {
      "online": true,
      "ping_ms": 1.2
    },

    "oc200": {
      "online": true,
      "ping_ms": 1.4
    },

    "sg2008p": {
      "online": true,
      "ping_ms": 0.8
    },

    "eap650": {
      "online": true,
      "ping_ms": 1.1
    }
  },

  "server": {
    "dell": {
      "online": true,
      "cpu_percent": 18.5,
      "memory_percent": 42.1,
      "temperature_c": 48.0,
      "uptime_seconds": 1543560
    }
  }
}
```

A API deverá fazer o trabalho de:

- coleta;
- normalização;
- tratamento de erro;
- timeout;
- cache;
- histórico;
- autenticação futura.

A SC01 deverá receber dados já preparados para exibição.

---

# 7. Comunicação SC01 -> Dell

Para a primeira versão:

```text
HTTP GET
```

Exemplo:

```text
SC01
  |
  | GET /netpanel/api/status
  v
Dell
```

Intervalo inicial sugerido:

```text
5 segundos
```

Esse valor deverá ser configurável.

Não existe necessidade de atualizar CPU, temperatura, disco e status de equipamentos dezenas de vezes por segundo.

---

# 8. MQTT

MQTT não é necessário na primeira versão.

Arquitetura inicial:

```text
SC01 -> HTTP -> Dell
```

Possível evolução:

```text
                  Mosquitto
                     |
        +------------+-------------+
        |            |             |
       Dell         RPi           SC01
```

Possíveis tópicos:

```text
netpanel/network/er605
netpanel/network/oc200
netpanel/network/sg2008p
netpanel/network/eap650

netpanel/server/dell
netpanel/server/rpi5

netpanel/wan/yo
netpanel/wan/vivo
```

MQTT deverá ser incluído apenas quando existir uma justificativa concreta.

---

# 9. Rede Wi-Fi da SC01

O ESP32-S3 deverá conectar ao Wi-Fi de 2,4 GHz.

O EAP650 deverá disponibilizar um SSID que permita acesso da SC01 ao Dell.

Exemplo:

```text
SSID: NET-IOT
```

Não é necessário que a SC01 tenha acesso administrativo direto aos equipamentos Omada.

Arquitetura preferida:

```text
SC01
  |
  | acesso permitido
  v
Dell
  |
  +--> ER605
  +--> OC200
  +--> SG2008P
  +--> EAP650
  +--> Raspberry Pi
  +--> servidores
```

---

# 10. VLAN da SC01

A VLAN deverá ser decidida antes da instalação definitiva.

## Opção A - utilizar uma VLAN já existente

É possível, desde que exista comunicação:

```text
SC01 -> Dell
```

## Opção B - criar VLAN IoT específica

Exemplo apenas para planejamento:

```text
VLAN 50 - IOT
```

Esse número ainda não é uma decisão de projeto.

A política poderia ser:

```text
SC01 -> Dell TCP 80/443       PERMITIR
SC01 -> DNS                   PERMITIR
SC01 -> NTP                   PERMITIR
SC01 -> Internet              OPCIONAL

SC01 -> ER605 administração   BLOQUEAR
SC01 -> OC200 administração   BLOQUEAR
SC01 -> PCs                   BLOQUEAR
SC01 -> NAS                   BLOQUEAR
```

---

# 11. Atenção à VLAN isolada

Uma VLAN configurada como:

```text
somente Internet
isolamento entre clientes
sem acesso à LAN
```

não funcionará para o NetPanel sem uma exceção.

A SC01 precisa conseguir atingir o Dell.

Mínimo:

```text
SC01 -> 192.168.10.5
```

nas portas utilizadas pela API e pelo OTA.

---

# 12. Firmware da SC01

Stack recomendado:

```text
ESP-IDF
LVGL
BSP da SC01 Plus
Wi-Fi
HTTP/HTTPS
OTA
NVS
LittleFS
```

Evitar inicialmente:

```text
Arduino + conjunto grande de bibliotecas sem controle de versão
```

Motivos:

- OTA mais controlável;
- particionamento explícito;
- rollback;
- melhor controle de memória;
- integração direta com ESP-IDF;
- manutenção futura.

---

# 13. Primeiro firmware: SC01_DIAG

Antes do NetPanel deverá existir um firmware de diagnóstico.

Nome provisório:

```text
SC01_DIAG
```

Funções:

```text
SC01_DIAG
|
+-- SYSTEM
|   +-- chip
|   +-- revisão
|   +-- Flash
|   +-- PSRAM
|   +-- MAC
|   +-- heap livre
|   +-- uptime
|
+-- LCD
|   +-- resolução
|   +-- teste RGB
|   +-- backlight
|   +-- FPS
|
+-- TOUCH
|   +-- coordenada X
|   +-- coordenada Y
|   +-- calibração/verificação
|
+-- WIFI
|   +-- scan
|   +-- SSID
|   +-- RSSI
|   +-- IP
|   +-- gateway
|   +-- DNS
|
+-- MICROSD
|   +-- detectar cartão
|   +-- leitura
|   +-- gravação
|
+-- AUDIO
|   +-- beep
|
+-- RS485
|   +-- inicialização
|   +-- teste
|
+-- OTA
    +-- verificar versão
    +-- baixar firmware
    +-- validar
    +-- reiniciar
```

---

# 14. Tela de diagnóstico

Exemplo:

```text
+----------------------------------------------+
| SC01 DIAG                           16:42     |
+----------------------------------------------+
| CHIP       ESP32-S3                         |
| FLASH      16 MB                            |
| PSRAM      2 MB                             |
| FW         0.1.0                            |
|                                              |
| WIFI       ONLINE                           |
| IP         192.168.x.20                     |
| RSSI       -55 dBm                          |
|                                              |
| LCD        OK                               |
| TOUCH      OK                               |
| SD         OK                               |
|                                              |
| [TESTES]       [OTA]       [SISTEMA]        |
+----------------------------------------------+
```

---

# 15. OTA - atualização por Wi-Fi

Após o primeiro firmware ser instalado via USB, as atualizações deverão ser feitas por rede.

Fluxo:

```text
PC
 |
 | build
 v
firmware.bin
 |
 v
Dell
 |
 | Wi-Fi
 v
SC01
```

---

# 16. OTA Pull

Esse deverá ser o mecanismo principal em produção.

A SC01 consulta:

```text
GET /netpanel/firmware/manifest.json
```

Exemplo:

```json
{
  "version": "0.2.3",
  "filename": "sc01-0.2.3.bin",
  "sha256": "HASH_AQUI",
  "size": 1849344
}
```

A SC01 compara:

```text
Firmware instalado : 0.2.2
Firmware disponível: 0.2.3
```

Se houver nova versão:

```text
+--------------------------------------+
| ATUALIZAÇÃO                          |
|                                      |
| Atual      0.2.2                     |
| Nova       0.2.3                     |
|                                      |
| [CANCELAR]           [INSTALAR]      |
+--------------------------------------+
```

---

# 17. OTA Push para desenvolvimento

Também poderá existir um script no computador de desenvolvimento.

Exemplo conceitual:

```bash
sc01-update 192.168.x.20
```

O script poderá:

```text
1. Compilar
2. Gerar .bin
3. Calcular SHA256
4. Copiar firmware para o Dell
5. Atualizar manifest.json
6. Solicitar atualização à SC01
7. Aguardar reboot
8. Consultar nova versão
9. Informar sucesso ou falha
```

Resultado esperado:

```text
Build................ OK
Firmware............. 0.3.7
SHA256............... OK
Upload Dell.......... OK

SC01................. 192.168.x.20
Versão atual......... 0.3.6
Atualização.......... iniciada

Download............. 100%
Validação............ OK
Reboot............... OK

Versão ativa......... 0.3.7
STATUS............... OK
```

---

# 18. Particionamento OTA

O primeiro firmware já deverá ter uma tabela de partições preparada para OTA.

Representação conceitual:

```text
FLASH

+----------------------------+
| Bootloader                 |
+----------------------------+
| Partition Table            |
+----------------------------+
| NVS                        |
+----------------------------+
| OTA DATA                   |
+----------------------------+
| OTA_0                      |
| Firmware A                 |
+----------------------------+
| OTA_1                      |
| Firmware B                 |
+----------------------------+
| LittleFS                   |
| configuração / assets      |
+----------------------------+
```

O tamanho exato das partições deverá ser definido depois que o firmware inicial medir a Flash real da placa.

---

# 19. Rollback

Rollback deverá ser tratado como requisito, não como melhoria futura.

Fluxo:

```text
Firmware 0.9.2
     |
     v
Instala 0.9.3 em OTA_1
     |
     v
Reboot
     |
     v
Firmware 0.9.3
     |
     +-- inicialização OK?
     +-- LVGL OK?
     +-- Wi-Fi OK?
     +-- API acessível?
     |
     +---- SIM ---> confirmar 0.9.3
     |
     +---- NÃO ---> rollback para 0.9.2
```

A confirmação do firmware deverá ocorrer apenas depois que as funções essenciais estiverem operacionais.

---

# 20. Servidor OTA

O Nginx poderá expor:

```text
/netpanel/firmware/manifest.json
/netpanel/firmware/sc01-0.2.3.bin
```

Estrutura:

```text
/opt/netpanel/firmware/
|
+-- stable/
|   +-- manifest.json
|   +-- sc01.bin
|
+-- versions/
    +-- sc01-0.1.0.bin
    +-- sc01-0.1.1.bin
    +-- sc01-0.2.0.bin
```

É recomendável manter versões antigas para facilitar rollback manual e análise de problemas.

---

# 21. Segurança do OTA

A primeira versão poderá funcionar exclusivamente dentro da LAN.

Mesmo assim, o projeto deverá prever evolução para:

```text
HTTPS
autenticação
assinatura de firmware
controle de versão
SHA256
anti-downgrade
```

Não é recomendável expor o endpoint OTA diretamente à Internet.

---

# 22. Interface principal NetPanel

Tela inicial proposta:

```text
+----------------------------------------------+
| NETPANEL                            16:42     |
+----------------------------------------------+
| INTERNET                                     |
|                                              |
| YO       ONLINE         11 ms                |
| VIVO     ONLINE         18 ms                |
|                                              |
+----------------------------------------------+
| REDE                                         |
|                                              |
| ER605       OK      OC200       OK           |
| SG2008P     OK      EAP650      OK           |
|                                              |
+----------------------------------------------+
| DELL                                         |
| CPU 18%     RAM 43%     TEMP 49 C            |
|                                              |
+----------------------------------------------+
| HOME       REDE       HOSTS       SISTEMA    |
+----------------------------------------------+
```

---

# 23. Tela de servidores

```text
+----------------------------------------------+
| SERVIDORES                                   |
+----------------------------------------------+
| DELL                              ONLINE     |
| CPU    [###------] 31%                       |
| RAM    [#####----] 54%                       |
| TEMP              48 C                       |
| UPTIME            18d 04h                    |
+----------------------------------------------+
| RPI5-SQL                          ONLINE     |
| CPU    [##-------] 19%                       |
| RAM    [###------] 32%                       |
| TEMP              52 C                       |
| MariaDB           ONLINE                     |
+----------------------------------------------+
```

---

# 24. Tela de rede

Exemplo:

```text
+----------------------------------------------+
| REDE                                         |
+----------------------------------------------+
| ER605       192.168.10.1       1.2 ms       |
| OC200       192.168.10.2       1.4 ms       |
| SG2008P     192.168.10.3       0.8 ms       |
| EAP650      192.168.10.4       1.1 ms       |
| DELL        192.168.10.5       0.5 ms       |
+----------------------------------------------+
```

---

# 25. WAN / Internet

Exemplo:

```text
+----------------------------------------------+
| INTERNET                                     |
+----------------------------------------------+
| YO                                           |
| Status       ONLINE                          |
| Ping         11 ms                           |
| Download     598 Mbps                        |
| Upload       239 Mbps                        |
|                                              |
| VIVO                                         |
| Status       ONLINE                          |
| Ping         18 ms                           |
| Download     604 Mbps                        |
| Upload       295 Mbps                        |
+----------------------------------------------+
```

Os testes de velocidade deverão ser executados pelo servidor, não pelo ESP32.

A SC01 apenas exibirá o último resultado.

---

# 26. Coleta de métricas

Possíveis coletores no Dell:

```text
ping
SNMP
HTTP API
Omada API
SSH
Linux /proc
psutil
smartctl
sensors
systemctl
MariaDB
```

Cada coletor deverá tratar:

```text
timeout
erro
equipamento offline
valor inválido
última coleta válida
timestamp
```

---

# 27. Histórico

O histórico não deverá ficar primariamente no microSD da SC01.

Melhor:

```text
Dell
 |
 +-- banco de dados
 +-- arquivos históricos
 +-- cache
```

A SC01 solicitará somente o intervalo necessário para gerar um gráfico.

Exemplo:

```text
GET /netpanel/api/history/ping/yo?period=24h
```

---

# 28. Banco de dados

Não é necessário criar banco já na primeira etapa.

Fase inicial:

```text
memória + JSON + logs
```

Depois:

```text
SQLite
```

ou, se houver razão para centralização:

```text
MariaDB
```

A escolha deverá ser feita pelo volume real de dados, não antecipadamente.

---

# 29. microSD

Possíveis usos:

```text
backup de configuração
assets gráficos
fontes
screenshots
logs locais
firmware de recuperação
```

Não utilizar o microSD como armazenamento primário de telemetria contínua sem necessidade.

---

# 30. RS-485

O RS-485 poderá ser explorado futuramente.

Possíveis aplicações:

```text
Modbus RTU
sensores
medidores de energia
controladores
UPS
equipamentos industriais
```

Esse recurso é independente do NetPanel e pode ser adicionado depois.

---

# 31. Áudio

O speaker poderá ser usado para alertas simples.

Exemplos:

```text
WAN caiu
servidor offline
temperatura crítica
erro de atualização
```

Os alertas deverão possuir:

```text
mute
volume
horário silencioso
```

para evitar comportamento incômodo.

---

# 32. Configuração persistente

O firmware deverá armazenar em NVS somente configurações necessárias.

Exemplo:

```text
Wi-Fi
URL da API
URL OTA
hostname
brightness
intervalo de atualização
preferências da interface
```

Evitar gravar constantemente na Flash.

---

# 33. Provisionamento Wi-Fi

Precisamos definir o método.

Possibilidades:

## Método 1 - configuração compilada

Não recomendado como solução final.

## Método 2 - arquivo no microSD

Útil para laboratório, mas não é o ideal.

## Método 3 - captive portal

Recomendado.

Fluxo:

```text
SC01 sem Wi-Fi configurado
      |
      v
cria AP temporário

SSID:
SC01-SETUP-XXXX

      |
      v
usuário abre página
      |
      v
seleciona SSID
      |
      v
informa senha
      |
      v
SC01 salva em NVS
      |
      v
reboot
```

---

# 34. Hostname

Sugestão:

```text
sc01-netpanel
```

Possível nome mDNS:

```text
sc01-netpanel.local
```

Isso pode facilitar desenvolvimento e manutenção.

---

# 35. Nginx

Arquitetura proposta:

```text
                       DELL
                  192.168.10.5

                +--------------+
                |    NGINX     |
                +------+-------+
                       |
             +---------+---------+
             |                   |
          /api/              /firmware/
             |                   |
      +------v------+       +----v-------+
      | Python API  |       | BIN / JSON |
      +-------------+       +------------+
```

O Nginx poderá ser inicialmente responsável por:

```text
reverse proxy
firmware
manifest
logs HTTP
TLS posteriormente
```

---

# 36. Serviços systemd

Possível estrutura:

```text
netpanel-api.service
nginx.service
```

Posteriormente:

```text
mosquitto.service
```

se MQTT for adotado.

---

# 37. Docker

Não é recomendado para a primeira versão.

Motivo:

```text
Python + Nginx + systemd
```

já atende ao projeto com menor complexidade operacional.

Docker poderá ser reavaliado caso o NetPanel passe a ser distribuído para múltiplos hosts.

---

# 38. Desenvolvimento

Ambiente recomendado:

```text
VS Code
ESP-IDF
Git
Python
```

Estrutura de repositório possível:

```text
netpanel/
|
+-- firmware/
|   +-- main/
|   +-- components/
|   +-- partitions.csv
|   +-- sdkconfig
|
+-- server/
|   +-- api/
|   +-- collectors/
|   +-- scripts/
|
+-- docs/
|
+-- tools/
```

---

# 39. Controle de versão

Utilizar Git desde o primeiro firmware.

Branches possíveis:

```text
main
develop
```

Tags:

```text
v0.1.0
v0.1.1
v0.2.0
```

O número de versão deverá estar presente:

- na tela;
- no firmware;
- no manifest;
- no log;
- no endpoint `/version`.

---

# 40. Endpoint de versão

Exemplo:

```http
GET /netpanel/api/version
```

Resposta:

```json
{
  "server": "0.1.0",
  "api": "1",
  "firmware_latest": "0.2.3"
}
```

---

# 41. Log da SC01

A SC01 deverá registrar eventos relevantes.

Exemplo:

```text
BOOT
Wi-Fi conectado
IP obtido
API conectada
API timeout
OTA iniciada
OTA concluída
OTA falhou
touch inicializado
LCD inicializado
SD detectado
```

Evitar geração excessiva de log.

---

# 42. Watchdog

O firmware deverá usar watchdog.

Situações que não podem travar a interface:

```text
API offline
Wi-Fi offline
DNS falhou
OTA indisponível
servidor desligado
```

A UI deve continuar responsiva mesmo sem rede.

---

# 43. Funcionamento offline

Se o Dell estiver indisponível:

```text
+----------------------------------------------+
| NETPANEL                                     |
+----------------------------------------------+
| SERVIDOR NETPANEL INDISPONÍVEL               |
|                                              |
| Última atualização: 16:41:27                 |
|                                              |
| Wi-Fi: OK                                    |
| IP: 192.168.x.20                             |
|                                              |
| Tentando novamente...                        |
+----------------------------------------------+
```

A SC01 não deve reiniciar em loop por ausência da API.

---

# 44. Atualização da interface

Evitar redesenhar a tela inteira a cada atualização.

Atualizar somente os objetos modificados:

```text
label
barra
status
gráfico
ícone
```

Isso reduz:

```text
uso de CPU
flicker
tráfego
alocações
```

---

# 45. Fases do projeto

## Fase 0 - documentação

Objetivo:

```text
confirmar arquitetura
definir VLAN
definir SSID
definir ambiente de desenvolvimento
```

---

## Fase 1 - SC01_DIAG via USB

Objetivo:

```text
boot
LCD
touch
Flash
PSRAM
Wi-Fi
IP
microSD
áudio
RS-485
```

Critério de conclusão:

```text
hardware identificado e estável
```

---

## Fase 2 - OTA

Objetivo:

```text
particionamento
OTA_0
OTA_1
manifest
SHA256
rollback
```

Critério:

```text
atualizar firmware sem USB
```

---

## Fase 3 - NetPanel API

Objetivo:

```text
Python
FastAPI
systemd
Nginx
/api/status
```

Critério:

```text
SC01 consegue obter JSON estável do Dell
```

---

## Fase 4 - UI básica

Objetivo:

```text
Home
Rede
Hosts
Sistema
```

Critério:

```text
interface funcional e responsiva
```

---

## Fase 5 - equipamentos Omada

Adicionar:

```text
ER605
OC200
SG2008P
EAP650
```

---

## Fase 6 - servidores

Adicionar:

```text
Dell
Raspberry Pi
serviços
temperaturas
discos
uptime
```

---

## Fase 7 - Internet/WAN

Adicionar:

```text
YO
Vivo
latência
status
último speedtest
```

---

## Fase 8 - histórico

Adicionar:

```text
gráficos
24h
7d
30d
```

---

## Fase 9 - segurança

Adicionar:

```text
HTTPS
assinatura OTA
autenticação
hardening
```

---

# 46. Ordem sugerida de implantação

```text
1. Preparar ESP-IDF
2. Fazer backup do firmware original, se possível
3. Flash SC01_DIAG por USB
4. Identificar Flash/PSRAM
5. Validar LCD
6. Validar touch
7. Validar Wi-Fi
8. Validar microSD
9. Configurar tabela OTA
10. Instalar firmware OTA inicial
11. Configurar servidor Dell
12. Criar /api/status
13. Conectar SC01 à API
14. Criar UI inicial
15. Adicionar métricas
16. Implementar histórico
17. Implementar segurança
```

---

# 47. Riscos técnicos

## 47.1 Variantes de hardware

Risco:

```text
SC01 Plus de lotes diferentes podem possuir diferenças.
```

Mitigação:

```text
SC01_DIAG antes do desenvolvimento completo.
```

---

## 47.2 OTA mal configurado

Risco:

```text
firmware inválido tornar o equipamento indisponível.
```

Mitigação:

```text
OTA A/B
rollback
SHA256
imagem validada
USB disponível para recuperação
```

---

## 47.3 VLAN bloqueando API

Risco:

```text
SC01 conecta ao Wi-Fi, mas não alcança o Dell.
```

Mitigação:

```text
regra explícita SC01 -> Dell.
```

---

## 47.4 Dependência da API

Risco:

```text
Dell desligado deixar a SC01 inutilizável.
```

Mitigação:

```text
cache
último estado
modo offline
UI independente da comunicação
```

---

## 47.5 Complexidade prematura

Evitar inicialmente:

```text
MQTT
Docker
banco distribuído
WebSocket
SNMP no ESP32
Omada API no ESP32
histórico no microSD
```

Adicionar somente quando houver necessidade.

---

# 48. Segurança

A SC01 deve ser tratada como dispositivo IoT.

Princípio:

```text
acesso mínimo necessário
```

Ideal:

```text
SC01
 |
 +--> Dell API
 +--> Dell OTA
 +--> DNS
 +--> NTP
```

Não conceder acesso administrativo geral à LAN.

---

# 49. Backup e recuperação

Manter:

```text
firmware estável
partitions.csv
sdkconfig
chaves
manifest
versões anteriores
procedimento USB de recuperação
```

O procedimento de recuperação deve funcionar mesmo se:

```text
Wi-Fi falhar
OTA falhar
API falhar
configuração NVS estiver inválida
```

---

# 50. Decisões ainda pendentes

Antes da implementação definitiva precisamos decidir:

### Rede

```text
[ ] VLAN da SC01
[ ] SSID da SC01
[ ] endereço DHCP reservado
[ ] acesso à Internet permitido ou não
```

### Firmware

```text
[ ] tamanho real Flash
[ ] tamanho real PSRAM
[ ] BSP final
[ ] tabela de partições
[ ] captive portal
```

### Servidor

```text
[ ] porta da API
[ ] HTTP ou HTTPS na primeira fase
[ ] diretório final
[ ] formato de logs
```

### Interface

```text
[ ] tema
[ ] dashboard inicial
[ ] equipamentos exibidos
[ ] intervalo de atualização
[ ] política de alertas
```

---

# 51. Proposta de MVP

O MVP não deve tentar implementar tudo.

## SC01

```text
LCD
touch
Wi-Fi
hora
status API
status de 5 equipamentos
CPU/RAM/TEMP Dell
OTA
```

## Dell

```text
FastAPI
Nginx
ping
psutil
manifest OTA
```

Resultado mínimo:

```text
SC01
 |
 +-- mostra rede
 +-- mostra Dell
 +-- atualiza via Wi-Fi
```

Somente depois disso o projeto deve avançar.

---

# 52. Critérios de sucesso do MVP

Considerar o MVP concluído quando:

```text
[ ] SC01 inicia normalmente
[ ] LCD funciona
[ ] touch funciona
[ ] conecta ao Wi-Fi
[ ] recebe IP por DHCP
[ ] acessa o Dell
[ ] recebe JSON da API
[ ] exibe status dos equipamentos
[ ] exibe CPU/RAM/TEMP do Dell
[ ] continua operando se a API cair
[ ] consegue atualizar firmware via Wi-Fi
[ ] rollback funciona
[ ] USB continua disponível para recuperação
```

---

# 53. Arquitetura final resumida

```text
                 +----------------+
                 |    INTERNET    |
                 +-------+--------+
                         |
                  +------v------+
                  |    ER605    |
                  +------+------+
                         |
                  +------v------+
                  |  SG2008P    |
                  +------+------+
                         |
          +--------------+---------------+
          |                              |
   +------v------+                +------v------+
   | Dell Ubuntu |                |   EAP650    |
   |             |                |             |
   | NetPanel API|                +------+------+
   | Nginx       |                       |
   | OTA         |                       | Wi-Fi
   | Coleta      |                       |
   +------+------+                 +------v------+
          |                        | SC01 Plus  |
          |                        | ESP32-S3   |
          |                        | LVGL       |
          |                        +-------------+
          |
          +--> ER605
          +--> OC200
          +--> SG2008P
          +--> EAP650
          +--> Raspberry Pi
          +--> servidores
```

---

# 54. Conclusão técnica

A infraestrutura necessária já está praticamente disponível.

Não é necessário adquirir um servidor dedicado, broker MQTT, gateway adicional ou outro microcontrolador para iniciar o projeto.

A estratégia recomendada é:

```text
1. Validar a SC01 Plus real
2. Configurar OTA corretamente
3. Criar API mínima no Dell
4. Criar UI LVGL
5. Adicionar métricas progressivamente
```

O ponto mais importante é não começar pela interface final.

O primeiro objetivo deve ser obter:

```text
SC01_DIAG + Wi-Fi + OTA + rollback
```

Depois disso, o desenvolvimento poderá ocorrer quase inteiramente pela rede, mantendo o USB apenas como mecanismo de instalação inicial e recuperação.

---

# 55. Próxima etapa recomendada

Quando este documento estiver revisado, a próxima etapa técnica deverá ser:

```text
SC01_DIAG v0.1.0
```

com:

```text
ESP-IDF
LCD
touch
Wi-Fi
hardware info
OTA preparado
```

Somente após confirmar a Flash, PSRAM e funcionamento real da placa deverá ser definida a tabela de partições definitiva e iniciado o NetPanel.
