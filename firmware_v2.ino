/*
  ==========================================================
  PROJETO MOTIVA - MONITORAMENTO DE VEGETACAO
  FIRMWARE 2.0
  Checkpoint S2-CP02 - Atualizacao Remota de Firmware (OTA)
  ==========================================================
  Mantem tudo do FW 1.0 e acrescenta:
  - Ordenacao das 5 leituras (copia) em ordem crescente
  - Exibicao da ordem original e da ordem ordenada
  - Calculo da mediana (3o elemento do vetor ordenado)
  - Histerese baseada na mediana (estados NORMAL / ALERTA)
  - LED verde = NORMAL, LED vermelho = ALERTA
*/

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>

// ---------- CONFIGURACOES (AJUSTE AQUI) ----------
const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASS = "";

const char* VERSION_URL = "https://raw.githubusercontent.com/SEU_USUARIO/repositorio-firmware/main/version.json";

const String CURRENT_VERSION = "2.0";

// ---------- PINOS ----------
const int LED_BLUE  = 25;
const int LED_GREEN = 26;
const int LED_RED   = 27;

// ---------- CONSTANTES DE TEMPO ----------
const unsigned long INTERVALO_LEITURA = 2000UL;
const unsigned long INTERVALO_SESSAO  = 48000UL;
const int LEITURAS_POR_SESSAO = 5;

// ---------- VARIAVEIS GLOBAIS ----------
float leituras[LEITURAS_POR_SESSAO];
float leiturasOrdenadas[LEITURAS_POR_SESSAO];
int sessaoAtual = 0;
unsigned long inicioSessao = 0;
bool atualizacaoRealizada = false;

enum EstadoVegetacao { NORMAL, ALERTA };
EstadoVegetacao estadoAtual = NORMAL;

// ================= SETUP =================
void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(LED_BLUE, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  atualizarLed();

  randomSeed(analogRead(0));

  Serial.println("========================================");
  Serial.println("MONITORAMENTO DE VEGETACAO - FW 2.0");
  Serial.println("========================================");

  inicioSessao = millis();
}

// ================= LOOP =================
void loop() {
  executarSessao();
  sessaoAtual++;

  if (sessaoAtual >= 3 && !atualizacaoRealizada) {
    verificarAtualizacao();
  }

  aguardarProximaSessao();
}

// ================= LEITURAS =================

void gerarLeituras() {
  Serial.println("Leituras (ordem original):");
  for (int i = 0; i < LEITURAS_POR_SESSAO; i++) {
    leituras[i] = random(10, 21);
    Serial.print("Leitura ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.print(leituras[i]);
    Serial.println(" cm");
    delay(INTERVALO_LEITURA);
  }
}

void ordenarLeituras() {
  for (int i = 0; i < LEITURAS_POR_SESSAO; i++) {
    leiturasOrdenadas[i] = leituras[i];
  }
  // Bubble sort (vetor pequeno, 5 elementos)
  for (int i = 0; i < LEITURAS_POR_SESSAO - 1; i++) {
    for (int j = 0; j < LEITURAS_POR_SESSAO - 1 - i; j++) {
      if (leiturasOrdenadas[j] > leiturasOrdenadas[j + 1]) {
        float tmp = leiturasOrdenadas[j];
        leiturasOrdenadas[j] = leiturasOrdenadas[j + 1];
        leiturasOrdenadas[j + 1] = tmp;
      }
    }
  }
}

void imprimirOrdenadas() {
  Serial.print("Valores ordenados: ");
  for (int i = 0; i < LEITURAS_POR_SESSAO; i++) {
    Serial.print(leiturasOrdenadas[i]);
    if (i < LEITURAS_POR_SESSAO - 1) Serial.print(", ");
  }
  Serial.println();
}

float calcularMedia() {
  float soma = 0;
  for (int i = 0; i < LEITURAS_POR_SESSAO; i++) soma += leituras[i];
  return soma / LEITURAS_POR_SESSAO;
}

float calcularMediana() {
  return leiturasOrdenadas[2]; // terceiro elemento (indice 2) do vetor ordenado
}

// ================= HISTERESE / LED =================

void avaliarHisterese(float mediana) {
  if (mediana >= 16.0) {
    estadoAtual = ALERTA;
  } else if (mediana <= 14.0) {
    estadoAtual = NORMAL;
  }
  // Entre 14 e 16 cm: mantem o estado anterior (nao faz nada aqui)
  atualizarLed();
}

void atualizarLed() {
  digitalWrite(LED_BLUE, LOW);
  if (estadoAtual == NORMAL) {
    digitalWrite(LED_GREEN, HIGH);
    digitalWrite(LED_RED, LOW);
  } else {
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_RED, HIGH);
  }
}

// ================= SESSAO =================

void executarSessao() {
  Serial.println("----------------------------------------");
  Serial.print("Sessao ");
  Serial.println(sessaoAtual + 1);

  gerarLeituras();
  ordenarLeituras();
  imprimirOrdenadas();

  float media = calcularMedia();
  float mediana = calcularMediana();

  Serial.print("Media da sessao: ");
  Serial.print(media);
  Serial.println(" cm");
  Serial.print("Mediana da sessao: ");
  Serial.print(mediana);
  Serial.println(" cm");

  avaliarHisterese(mediana);

  Serial.print("Estado atual: ");
  Serial.println(estadoAtual == NORMAL ? "NORMAL" : "ALERTA");
  Serial.println("Proxima sessao em 48 segundos.");
}

void aguardarProximaSessao() {
  unsigned long decorrido = millis() - inicioSessao;
  if (decorrido < INTERVALO_SESSAO) {
    delay(INTERVALO_SESSAO - decorrido);
  }
  inicioSessao += INTERVALO_SESSAO;
}

// ================= OTA (mantido do FW 1.0) =================

bool conectarWiFi() {
  Serial.println("Conectando ao WiFi (Wokwi-GUEST)...");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  unsigned long inicio = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - inicio < 10000) {
    delay(300);
    Serial.print(".");
  }
  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("WiFi conectado. IP: ");
    Serial.println(WiFi.localIP());
    return true;
  }
  Serial.println("ERRO: nao foi possivel conectar ao WiFi.");
  return false;
}

String extrairValorJson(String json, String chave) {
  String busca = "\"" + chave + "\"";
  int idx = json.indexOf(busca);
  if (idx == -1) return "";
  idx = json.indexOf(":", idx);
  if (idx == -1) return "";
  int inicioValor = json.indexOf("\"", idx + 1);
  int fimValor = json.indexOf("\"", inicioValor + 1);
  if (inicioValor == -1 || fimValor == -1) return "";
  return json.substring(inicioValor + 1, fimValor);
}

void verificarAtualizacao() {
  Serial.println("========================================");
  Serial.println("Verificando atualizacao de firmware...");

  if (!conectarWiFi()) {
    Serial.println("Atualizacao cancelada: sem conexao Wi-Fi.");
    return;
  }

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  http.begin(client, VERSION_URL);
  int codigo = http.GET();

  if (codigo != 200) {
    Serial.print("ERRO: nao foi possivel acessar o manifesto. Codigo HTTP: ");
    Serial.println(codigo);
    http.end();
    return;
  }

  String payload = http.getString();
  http.end();

  String versaoDisponivel = extrairValorJson(payload, "version");
  String urlFirmware = extrairValorJson(payload, "url");

  Serial.print("Versao instalada: ");
  Serial.println(CURRENT_VERSION);
  Serial.print("Versao disponivel: ");
  Serial.println(versaoDisponivel);

  if (versaoDisponivel == "" || urlFirmware == "") {
    Serial.println("ERRO: manifesto invalido ou incompleto.");
    return;
  }

  if (versaoDisponivel == CURRENT_VERSION) {
    Serial.println("A versao instalada ja e a mais recente. Nenhuma atualizacao necessaria.");
    return;
  }

  Serial.println("Nova versao disponivel! Iniciando download e atualizacao OTA...");
  Serial.println(urlFirmware);

  WiFiClientSecure clientOta;
  clientOta.setInsecure();

  t_httpUpdate_return ret = httpUpdate.update(clientOta, urlFirmware);

  switch (ret) {
    case HTTP_UPDATE_FAILED:
      Serial.printf("ERRO na atualizacao OTA (%d): %s\n",
                     httpUpdate.getLastError(),
                     httpUpdate.getLastErrorString().c_str());
      break;
    case HTTP_UPDATE_NO_UPDATES:
      Serial.println("Nenhuma atualizacao disponivel.");
      break;
    case HTTP_UPDATE_OK:
      Serial.println("Atualizacao OK! Reiniciando...");
      atualizacaoRealizada = true;
      break;
  }
}
