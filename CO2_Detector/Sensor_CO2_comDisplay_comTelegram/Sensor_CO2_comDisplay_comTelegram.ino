/*******
  baseado no exemplo "Get CO2 value" da biblioteca S8_UART
  por Ricardo Michel, em 2025/07/15

  Bibliotecas:
  EspSoftwareSerial, by Dirk Kaar, Peter Lerup, ver. 8.1.0
  S8_UART, by Josep Comas, ver. 1.0.2

  ESP32 Dev Module
  Versão do firmware: 1.1.0

  Como o programa funciona:
  1. setup() roda uma vez: liga o display, tenta o Wi-Fi por 10 s e confere o sensor.
  2. loop() repete para sempre: lê o CO2, mostra no display e, a cada 30 s,
     envia a leitura ao Telegram (se houver Wi-Fi).
 *******/

#include <Arduino.h>
#include <SoftwareSerial.h>
#include "s8_uart.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <HTTPClient.h>

// >>> TROQUE pelos seus dados antes de carregar. Não publique o sketch com os dados reais. <<<
// Como criar o bot e achar o ID do chat: veja o README desta pasta.
const char* ssid = "TROQUE_PELO_NOME_DA_REDE";
const char* password = "TROQUE_PELA_SENHA_DO_WIFI";
const char* botToken = "TROQUE_PELO_TOKEN_DO_BOT";
const char* chatId = "TROQUE_PELO_ID_DO_CHAT";

// Tempos
const int WIFI_TIMEOUT_S = 10;                   // espera máxima pelo Wi-Fi na inicialização
const unsigned long INTERVALO_ENVIO_MS = 30000;  // uma mensagem a cada 30 segundos
unsigned long ultimoEnvio = 0;

// Display OLED 128x64 ligado por I2C
const int SCREEN_WIDTH = 128;
const int SCREEN_HEIGHT = 64;
const uint8_t SCREEN_ADDRESS = 0x3C;  // endereço do display usado nas placas deste projeto
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);  // -1: sem pino de reset

// Sensor SenseAir S8: os fios se cruzam (o TX de um vai no RX do outro)
const int S8_RX_PIN = 16;  // ligado ao TX do S8
const int S8_TX_PIN = 17;  // ligado ao RX do S8
EspSoftwareSerial::UART S8_serial;
S8_UART* sensor_S8;
S8_sensor sensor;

// Apaga a tela e deixa o cursor no topo, com letra pequena.
// O desenho só aparece quando display.display() é chamado.
void limpaTela() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
}

void showWifiCountdown(int segundos) {
  limpaTela();
  display.print("Tentando conectar");
  display.setCursor(0, 12);
  display.print("ao WiFi");
  display.setTextSize(3);
  display.setCursor(46, 32);
  display.print(segundos);
  display.display();
}

void showStartupCheck(bool monitorOk) {
  limpaTela();
  display.print("Inicializacao");
  display.setCursor(0, 18);
  display.print("Display: OK");
  display.setCursor(0, 32);
  display.print(monitorOk ? "Monitor CO2: OK" : "Monitor CO2: FALHA");
  display.setCursor(0, 46);
  display.print(WiFi.status() == WL_CONNECTED ? "WiFi: OK" : "WiFi: offline");
  display.display();
}

// Um endereço de internet não aceita espaços nem acentos:
// cada byte especial vira % seguido de dois dígitos hexadecimais (percent-encoding).
String urlEncode(const String &text) {
  const char *hex = "0123456789ABCDEF";
  String out;
  out.reserve(text.length() * 3);  // reserva a memória de uma vez
  for (size_t i = 0; i < text.length(); i++) {
    uint8_t c = text[i];
    if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
      out += (char)c;
    } else {
      out += '%';
      out += hex[c >> 4];    // 4 bits de cima
      out += hex[c & 0x0F];  // 4 bits de baixo
    }
  }
  return out;
}

// A API do Telegram é um endereço web: pedir essa URL envia a mensagem pelo bot
void sendMessageToTelegram(const String &message) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi desconectado, não pode enviar mensagem ao Telegram.");
    return;
  }

  String url = String("https://api.telegram.org/bot") + botToken +
               "/sendMessage?chat_id=" + chatId + "&text=" + urlEncode(message);

  Serial.print("Enviando mensagem ao Telegram: ");
  Serial.println(message);

  HTTPClient http;
  http.begin(url);  // sem certificado: o servidor não é verificado (veja o README)
  int httpCode = http.GET();

  if (httpCode > 0) {
    // 200 = enviado. Em caso de erro, a resposta do Telegram diz o motivo.
    Serial.printf("[HTTP] GET... code: %d\n", httpCode);
    Serial.println(http.getString());
  } else {
    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }
  http.end();  // libera a memória usada na requisição
}

void setup() {
  Serial.begin(115200);  // Monitor Serial do computador

  // O display é iniciado primeiro para mostrar a contagem de conexão
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (true) delay(1000);  // sem memória para o display: para aqui
  }
  // begin() não percebe um display ausente; a resposta no endereço I2C confirma a ligação
  Wire.beginTransmission(SCREEN_ADDRESS);
  Serial.println(Wire.endTransmission() == 0 ? "Display: OK" : "Display: FALHA (sem resposta no I2C)");

  Serial.print("Conectando a ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  // Conta de 10 a 0 e sai antes se conectar; sem rede, o medidor segue offline
  for (int restante = WIFI_TIMEOUT_S; restante >= 0 && WiFi.status() != WL_CONNECTED; restante--) {
    showWifiCountdown(restante);
    if (restante > 0) delay(1000);
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi: OK");
    Serial.print("Endereço IP: ");
    Serial.println(WiFi.localIP());
    sendMessageToTelegram("ESP32 conectado e pronto!");
  } else {
    Serial.println("WiFi: offline. Medindo sem rede.");
  }

  // Se o sensor responder à versão do firmware, a ligação está funcionando
  S8_serial.begin(S8_BAUDRATE, SWSERIAL_8N1, S8_RX_PIN, S8_TX_PIN);
  sensor_S8 = new S8_UART(S8_serial);
  sensor_S8->get_firmware_version(sensor.firm_version);
  if (sensor.firm_version[0] == '\0') {  // texto vazio: ninguém respondeu
    Serial.println("Monitor CO2: FALHA (SenseAir S8 não foi encontrado)");
    showStartupCheck(false);
    while (true) delay(1000);  // só sai daqui com reset
  }

  Serial.println("Monitor CO2: OK");
  showStartupCheck(true);
  delay(3000);  // tempo para ler a checagem no display

  Serial.printf("Versão do firmware do S8: %s\n", sensor.firm_version);
  sensor.sensor_id = sensor_S8->get_sensor_ID();
  Serial.print("Sensor ID: 0x");
  printIntToHex(sensor.sensor_id, 4);
  Serial.println("\nTudo Ok! Iniciando as medidas!");
}

void loop() {
  sensor.co2 = sensor_S8->get_co2();
  const bool leituraOk = sensor.co2 > 0;  // a biblioteca devolve 0 quando a leitura falha

  limpaTela();
  display.print("Nivel de CO2:");
  display.setCursor(10, 20);
  if (leituraOk) {
    display.print(sensor.co2);
    display.print(" ppm");
    Serial.printf("CO2 value = %d ppm\n", sensor.co2);
  } else {
    display.print("sem leitura");
    Serial.println("Falha ao ler o sensor de CO2.");
  }
  display.display();

  // millis() conta o tempo ligado: assim o intervalo não trava o programa
  if (leituraOk && millis() - ultimoEnvio >= INTERVALO_ENVIO_MS) {
    ultimoEnvio = millis();
    sendMessageToTelegram("Sensor de CO2 FabLab! Nível de CO2: " + String(sensor.co2) + "ppm");
  }

  delay(500);
}
