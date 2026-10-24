# Tentativas e erros — SC01 Plus

**Objetivo:** manter um histórico técnico de experimentos que não produziram o resultado esperado, sem confundi-los com marcos confirmados do projeto.

## Regras de registro

- Registrar somente fatos observados e a configuração utilizada.
- Não promover hipótese a configuração válida sem confirmação visual ou funcional na placa.
- Quando um experimento escrever na Flash, registrar o mecanismo de retorno ao estado conhecido.

## 28/07/2026 — Diagnóstico LCD com BSP genérico

| Campo | Registro |
|---|---|
| Projeto | `diag` |
| Abordagem | BSP `espp/smartpanlee-sc01-plus`, com inicialização de LCD e LVGL |
| Resultado observado | Tela acesa, sem imagem válida; posteriormente foram vistos pixels coloridos distribuídos pela tela. |
| Estado | Não validado |
| Conclusão | O BSP genérico não pode ser adotado como referência de pinagem/inicialização para esta unidade sem validação adicional. |

## 28/07/2026 — Diagnóstico LCD direto com ESP-LCD

| Campo | Registro |
|---|---|
| Projeto | `diag_direct` |
| ESP-IDF | 5.5.5 |
| Controlador pressuposto | ST7796UI |
| Interface pressuposta | MCU/i80 de 8 bits, 480 × 320, RGB565 |
| Configuração testada | DC GPIO0; WR GPIO47; RESET GPIO4; backlight GPIO45; D0…D7 = GPIO9, GPIO46, GPIO3, GPIO8, GPIO18, GPIO17, GPIO16, GPIO15; relógio de 10 MHz. |
| Padrão enviado | Quatro faixas horizontais: vermelho, verde, azul e branco. |
| Resultado observado | A tela piscava com pixels coloridos, sem imagem exibida e sem cores sólidas. |
| Estado | Falhou na validação visual |
| Evidência positiva limitada | O backlight ligou e houve atividade no barramento; isso não confirma a inicialização correta do painel. |
| Hipóteses ainda abertas | Controlador do LCD diferente do pressuposto; ordem/polaridade de dados ou sinais divergente; sequência de inicialização específica do fabricante; modo de cor, orientação ou sinal de controle inadequados. |
| Próxima ação segura | Obter fonte/binário/documentação oficial que contenha a sequência de inicialização do LCD e compará-la antes de outro flash de teste. |

## 28/07/2026 — Restauração após testes de LCD

| Campo | Registro |
|---|---|
| Situação | Após os diagnósticos de LCD, foi solicitada a volta ao firmware original. |
| Ação | Imagem integral de 16 MB regravada a partir de `backups/sc01-original-20260728.bin`. |
| Validação técnica | Todos os blocos escritos receberam confirmação de hash pelo esptool. |
| Validação visual | Operador confirmou que a tela voltou ao firmware original. |
| Estado | Recuperação concluída com sucesso. |

## Decisões decorrentes

1. Não usar a configuração do `diag_direct` como base de firmware funcional.
2. Não avançar para LVGL, touch ou OTA enquanto o LCD não tiver uma inicialização reproduzível e visualmente validada.
3. Preservar o backup original como ponto de recuperação antes de cada novo teste de escrita na Flash.
