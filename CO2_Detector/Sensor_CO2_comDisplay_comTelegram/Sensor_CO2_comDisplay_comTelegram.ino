/*******
  baseado no exemplo "Get CO2 value" da biblioteca S8_UART 
  por Ricardo Michel, em 2025/07/15

  Bibliotecas:
  EspSoftwareSerial, by Dirk Kaar, Peter Lerup, ver. 8.1.0
  S8_UART, by Josep Comas, ver. 1.0.2

  ESP32 Dev Module
  Versão do firmware: 1.1.0

 *******/

// Define quais as bibliotecas a usar
#include <Arduino.h>
#include <SoftwareSerial.h>
#include "s8_uart.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "secrets.h"

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

//token do bot
const char* botToken = TELEGRAM_BOT_TOKEN;
//id do chat do telegram
const char* chatId = TELEGRAM_CHAT_ID;

// URL base para a API do Telegram
const String telegramApiBase = "https://api.telegram.org/bot";

//controlar o tempo de envio da mensagem
unsigned long lastSendTime = 0; //armazena o tempo da ultima mensagem enviada
const unsigned long sendInterval = 30000UL; // a cada 30 segundos

#define WIFI_TIMEOUT_S 10     // tempo máximo de espera pelo WiFi na inicialização

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels


#define OLED_RESET -1        // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C  ///< Endereço do display 128x64 usado nas placas deste projeto
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


// Define os parâmetros do sistemas
#define RXD2 16
#define TXD2 17
#define DEBUG_BAUDRATE 115200
#define S8_RX_PIN 16  // esse RX deve ser ligado ao TX do S8
#define S8_TX_PIN 17  // esse TX deve ser ligado ao RX do S8

// Protocolos iniciais para SenseAir S8 e para a comunicação serial via software
S8_UART* sensor_S8;
S8_sensor sensor;
EspSoftwareSerial::UART S8_serial;

// Mostra no OLED a contagem regressiva da tentativa de conexão
void showWifiCountdown(int segundos) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Tentando conectar");
  display.setCursor(0, 12);
  display.print("ao WiFi");
  display.setTextSize(3);
  display.setCursor(46, 32);
  display.print(segundos);
  display.display();
}

void setup() {
  // inicia a serial do terminal; a do SenseAir S8 é iniciada mais abaixo
  Serial.begin(DEBUG_BAUDRATE);

  // O display é iniciado primeiro para mostrar a contagem de conexão
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;  // Don't proceed, loop forever
  }

  Serial.print("Conectando a ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  // Tenta por até 10 segundos; depois segue medindo mesmo sem rede
  for (int restante = WIFI_TIMEOUT_S; restante >= 0 && WiFi.status() != WL_CONNECTED; restante--) {
    showWifiCountdown(restante);
    if (restante > 0) delay(1000);
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi conectado!");
    Serial.print("Endereço IP: ");
    Serial.println(WiFi.localIP());
    // Envia uma mensagem de teste logo após conectar
    sendMessageToTelegram("ESP32 conectado e pronto!");
  } else {
    Serial.println("Sem WiFi: medindo offline.");
  }


  // Aguarda o início da comunicação com o S8
  int i = 0;
  while (!Serial && i < 50) {
    delay(10);
    i++;
  }
  // Informa que o sistema está vivo e funcionando
  Serial.println("");
  Serial.println("Iniciando...");
  // Inicializa o SenseAir S8
  S8_serial.begin(S8_BAUDRATE, SWSERIAL_8N1, S8_RX_PIN, S8_TX_PIN);
  sensor_S8 = new S8_UART(S8_serial);
  // Verifica se o SenseAir S8 está se comunicando
  sensor_S8->get_firmware_version(sensor.firm_version);
  int len = strlen(sensor.firm_version);
  if (len == 0) {
    Serial.println("Sensor de CO2 SenseAir S8 não foi encontrado!");
    while (1) { delay(1); };  // pausa para sempre - não sai daqui sem reset
  }

  // Informações iniciais do SenseAir S8
  Serial.println(">>> SenseAir S8 NDIR CO2 sensor <<<");
  printf("Versão do Firmware: %s\n", sensor.firm_version);
  sensor.sensor_id = sensor_S8->get_sensor_ID();
  Serial.print("Sensor ID: 0x");
  printIntToHex(sensor.sensor_id, 4);
  Serial.println("");
  // Avisa que completou a inicialização
  Serial.println("Tudo Ok! Iniciando as medidas!");
  Serial.flush();
}


void loop() {
  sensor.co2 = sensor_S8->get_co2();

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.print("Nivel de CO2:");

  display.setCursor(10, 20);
  display.print(sensor.co2);
  display.print(" ppm");

  display.display();

  printf("CO2 value = %d ppm\n", sensor.co2);

  if (millis() - lastSendTime >= sendInterval) {
    lastSendTime = millis();
    String message = "Sensor de CO2 FabLab! Nível de CO2: " + String(sensor.co2) + "ppm";
    sendMessageToTelegram(message);
  }

  delay(500);
}
// Codifica o texto para uso na URL (percent-encoding, RFC 3986)
String urlEncode(const String &text) {
  const char *hex = "0123456789ABCDEF";
  String out;
  out.reserve(text.length() * 3);
  for (size_t i = 0; i < text.length(); i++) {
    uint8_t c = text[i];
    if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
      out += (char)c;
    } else {
      out += '%';
      out += hex[c >> 4];
      out += hex[c & 0x0F];
    }
  }
  return out;
}

void sendMessageToTelegram(String message) {
  if (WiFi.status() == WL_CONNECTED) {  //so vai enviar se o wifi estiver conectado
    HTTPClient http;
    String url = telegramApiBase + botToken + "/sendMessage?chat_id=" + chatId + "&text=" + urlEncode(message);

    Serial.print("Enviando mensagem ao Telegram: ");
    Serial.println(message);

    http.begin(url);            // Inicia a requisição HTTP
    int httpCode = http.GET();  // Faz a requisição GET

    // httpCode > 0 indica que a requisição foi feita
    if (httpCode > 0) {
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        String payload = http.getString();
        Serial.println(payload);  // Imprime a resposta do Telegram (para debug)
      }
    } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();  // Libera recursos da requisição
  } else {
    Serial.println("WiFi desconectado, não pode enviar mensagem ao Telegram.");
  }
}
