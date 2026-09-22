# repositorio-firmware — Projeto Motiva (OTA) — S2-CP02

**Integrante(s) / RM(s):** _preencher_

## Arquitetura da solução

```
ESP32 / Wokwi (Firmware 1.0)
        |
        |  HTTP(S) via rede Wokwi-GUEST
        v
Repositório remoto (GitHub, arquivos RAW públicos)
  |-- version.json      -> manifesto com a versão disponível e a URL do .bin
  |-- firmware_v2.bin    -> binário compilado do Firmware 2.0
  |-- firmware_v1.ino    -> código-fonte do Firmware 1.0
  |-- firmware_v2.ino    -> código-fonte do Firmware 2.0

Fluxo: consultar version.json -> comparar versão -> baixar .bin -> gravar OTA -> reiniciar -> Firmware 2.0 em execução
```

## Como funciona

- O **Firmware 1.0** roda o monitoramento (5 leituras simuladas de 10–20 cm por
  sessão, a cada 2 s, com nova sessão a cada 48 s medidos com `millis()`) e
  acende o LED azul.
- Depois de pelo menos 3 sessões, ele conecta ao Wi-Fi `Wokwi-GUEST`, baixa
  `version.json` e compara a versão disponível com a instalada (`1.0`).
- Se houver versão mais nova, baixa `firmware_v2.bin` e executa a gravação
  OTA via `HTTPUpdate`, reiniciando o ESP32.
- O **Firmware 2.0** mantém tudo do 1.0 e adiciona: ordenação das leituras,
  cálculo da mediana, histerese de estado (NORMAL/ALERTA) baseada na
  mediana, e o LED passa a indicar o estado (verde = NORMAL, vermelho =
  ALERTA).

## Como gerar o firmware_v2.bin

1. Abra `firmware_v2.ino` na Arduino IDE com a placa **ESP32 Dev Module**
   selecionada (via Boards Manager: pacote *esp32* by Espressif Systems).
2. Ajuste `VERSION_URL` para a URL RAW do seu `version.json` no GitHub.
3. Menu **Sketch > Export Compiled Binary**.
4. Renomeie o `.bin` gerado para `firmware_v2.bin` e suba para este
   repositório.

## Links

- Projeto Wokwi (demonstração): `<colar link público>`
- Repositório remoto (este): `<colar link do GitHub>`

## Testes realizados

Ver tabela de testes obrigatórios no relatório de entrega (PDF).
