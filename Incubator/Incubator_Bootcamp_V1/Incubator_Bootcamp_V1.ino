/*
Versão do firmware: 1.0.1

Código da Incubadora/Estufa V1 com:
- Controle ON/OFF de temperatura com a lâmpada 250W,
- Sensoreamento de temperatura com o DS18B20, 
- 2 Botões para definir a temperatura alvo,
- Cooler para fluxo de ar dentro da incubadora,
- Lâmpada desligada ao iniciar e quando a leitura do DS18B20 falha,
- Temperatura alvo limitada a 60 °C.
*/

// Incluir as bibliotecas para o LCD e Sensor de Temperatura DS18B20:
#include <LiquidCrystal.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Pino de dados para o sensor DS18B20:
#define ONE_WIRE_BUS 8

// Setup ado OneWire:
OneWire oneWire(ONE_WIRE_BUS);

// Referência do DS18B20 para o OneWire:
DallasTemperature sensors(&oneWire);

// Associando os pinos de interface do LCD:
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Definindo os pinos que não necessitam de biblioteca:

int intensidadeCooler = 255; // Intensidade do PWM do cooler em 8 bit (0 a 255)
const int pinoLampada = 6; // Pino de OUTPUT para controle do relé que permite a operação da lâmpada 250W
const int cooler = 10; // Pino de OUTPUT para o controle do cooler
const int butDown = 9; // Pino de INPUT com PULLUP para descer a temperatura alvo
const int butUp = 13; // Pino de INPUT com PULLUP para subir a temperatura alvo

float temperaturaAlvo = 37.5;     // Temperatura alvo desejada para a incubadora em graus Celsius (ex: 37.5°C para ovos)
float histerese = 0.5; // Valor de histerese em graus Celsius para não ficar com a lâmpada ligando e desligando muito rápido.
const float temperaturaAlvoMaxima = 60.0; // Limite do ajuste pelos botões. Não substitui uma proteção térmica independente.

// Nível do pino que liga e desliga o relé.
// ATENÇÃO: confira no circuito real, com a lâmpada fora da rede. Se o seu relé liga com HIGH, inverta as duas linhas.
const int RELE_LIGADO = LOW;
const int RELE_DESLIGADO = HIGH;

// Tempos em milissegundos
const unsigned long tempoConversao = 750;  // conversão do DS18B20 em 12 bits
const unsigned long tempoDebounce = 50;    // estabilidade exigida do botão
const unsigned long tempoRepeticao = 400;  // repetição com o botão pressionado

unsigned long inicioConversao = 0;
float temperaturaAtual = DEVICE_DISCONNECTED_C;


struct Botao {
  int pino;
  int leitura;    // última leitura crua
  int estado;     // leitura já filtrada
  unsigned long mudou;
  unsigned long repetiu;
};

Botao botaoSobe = {butUp, HIGH, HIGH, 0, 0};
Botao botaoDesce = {butDown, HIGH, HIGH, 0, 0};

// true uma vez a cada toque e, com o botão pressionado, a cada tempoRepeticao
bool botaoAcionado(Botao &b) {
  const int agoraLido = digitalRead(b.pino);
  const unsigned long agora = millis();
  if (agoraLido != b.leitura) {
    b.leitura = agoraLido;
    b.mudou = agora;
  }
  if (agora - b.mudou < tempoDebounce) return false;
  if (agoraLido != b.estado) {
    b.estado = agoraLido;
    b.repetiu = agora;
    return agoraLido == LOW;
  }
  if (b.estado == LOW && agora - b.repetiu >= tempoRepeticao) {
    b.repetiu = agora;
    return true;
  }
  return false;
}

// Escreve a linha inteira e completa com espaços, para não sobrar caractere antigo
void escreveLinha(int linha, String texto) {
  while (texto.length() < 16) texto += ' ';
  lcd.setCursor(0, linha);
  lcd.print(texto);
}

// Rejeita DEVICE_DISCONNECTED_C (-127) e valores fora da faixa do DS18B20
bool leituraValida(float t) {
  return t != DEVICE_DISCONNECTED_C && t >= -55.0 && t <= 125.0;
}

void defineAquecimento(bool ligar) {
  digitalWrite(pinoLampada, ligar ? RELE_LIGADO : RELE_DESLIGADO);
}

void controlaTemperatura() {
  const bool ligada = digitalRead(pinoLampada) == RELE_LIGADO;

  if (!leituraValida(temperaturaAtual)) {
    // Sem leitura confiável: lâmpada desligada e ventilador no máximo
    defineAquecimento(false);
    intensidadeCooler = 255;
    analogWrite(cooler, intensidadeCooler);
    escreveLinha(0, "ERRO NO SENSOR");
    Serial.println("ERRO: DS18B20 desconectado ou leitura invalida. Lampada DESLIGADA.");
    return;
  }

  escreveLinha(0, "TempC: " + String(temperaturaAtual, 2));

  if (temperaturaAtual < (temperaturaAlvo - histerese) && !ligada) {
    // Abaixo de temperaturaAlvo - histerese e desligada: liga para aquecer
    defineAquecimento(true);
    intensidadeCooler = 255;
    analogWrite(cooler, intensidadeCooler);
    Serial.println("Lâmpada LIGADA (temperatura muito baixa)!");
  } else if (temperaturaAtual >= temperaturaAlvo && ligada) {
    // Atingiu o alvo: desliga
    defineAquecimento(false);
    intensidadeCooler = 122;
    analogWrite(cooler, intensidadeCooler);
    Serial.println("Lâmpada DESLIGADA (temperatura atingida)!");
  } else {
    // Dentro da banda de histerese ou a lâmpada já está no estado correto
    Serial.println("Temperatura dentro da faixa.");
  }

  Serial.print("Intensidade PWM cooler: ");
  Serial.print(intensidadeCooler);
  Serial.print(", ");
  Serial.print("Temperatura: ");
  Serial.print(temperaturaAtual, 2);
  Serial.print(", ");
  Serial.print("TemperaturaAlvo: ");
  Serial.println(temperaturaAlvo);
}

void setup() {
  // Antes de tudo: relé desligado. O nível é definido antes de o pino virar saída.
  digitalWrite(pinoLampada, RELE_DESLIGADO);
  pinMode(pinoLampada, OUTPUT);

  // Start serial communication for debugging purposes
  Serial.begin(9600);

  pinMode(cooler, OUTPUT);
  pinMode(butUp, INPUT_PULLUP);
  pinMode(butDown, INPUT_PULLUP);

  // Start up the library
  sensors.begin();
  sensors.setResolution(12);
  sensors.setWaitForConversion(false); // não trava o loop: os botões continuam respondendo

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  escreveLinha(0, "TempC: ...");
  escreveLinha(1, "TempAlvo: " + String(temperaturaAlvo));

  analogWrite(cooler, intensidadeCooler);

  sensors.requestTemperatures();
  inicioConversao = millis();
}

void loop() {
  // Os dois botões são lidos em todo ciclo, para o debounce de cada um ficar em dia
  const bool sobe = botaoAcionado(botaoSobe);
  const bool desce = botaoAcionado(botaoDesce);
  if (sobe || desce) {
    if (sobe) temperaturaAlvo = min(temperaturaAlvo + 0.5f, temperaturaAlvoMaxima);
    if (desce) temperaturaAlvo -= 0.5;
    escreveLinha(1, "TempAlvo: " + String(temperaturaAlvo));
  }

  if (millis() - inicioConversao >= tempoConversao) {
    temperaturaAtual = sensors.getTempCByIndex(0);
    controlaTemperatura();
    sensors.requestTemperatures();
    inicioConversao = millis();
  }
}
