/*
Versão do firmware: 1.0.1

Código da Incubadora/Estufa V1 com:
- Controle ON/OFF de temperatura com a lâmpada 250W,
- Sensoreamento de temperatura com o DS18B20,
- 2 Botões para definir a temperatura alvo,
- Cooler para fluxo de ar dentro da incubadora,
- Lâmpada desligada ao iniciar e quando a leitura do DS18B20 falha,
- Temperatura alvo limitada a 60 °C.

Como o programa funciona:
O loop() não usa delay(). Ele passa milhares de vezes por segundo e, a cada volta:
1. confere os botões;
2. quando passam 750 ms, lê a temperatura, decide sobre a lâmpada e pede a próxima medida.
*/

#include <LiquidCrystal.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Sensor de temperatura DS18B20 (protocolo 1-Wire: um único fio de dados)
const int pinoSensor = 8;
OneWire oneWire(pinoSensor);
DallasTemperature sensors(&oneWire);

// LCD 16x2: pinos RS, EN, D4, D5, D6, D7
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

const int pinoLampada = 6;  // controla o relé da lâmpada de 250 W
const int pinoCooler = 10;  // ventilador, com velocidade controlada por PWM (0 a 255)
const int butDown = 9;      // botão que diminui a temperatura alvo
const int butUp = 13;       // botão que aumenta a temperatura alvo

// Nível do pino que liga e desliga o relé.
// ATENÇÃO: confira no circuito real, com a lâmpada fora da rede. Se o seu relé liga com HIGH, inverta as duas linhas.
const int RELE_LIGADO = LOW;
const int RELE_DESLIGADO = HIGH;

const int COOLER_MAXIMO = 255;  // usado ao aquecer e quando o sensor falha
const int COOLER_REPOUSO = 122;  // usado depois de atingir o alvo

float temperaturaAlvo = 37.5;               // em °C (ex.: 37,5 °C para ovos)
const float temperaturaAlvoMaxima = 60.0;  // limite dos botões; não substitui proteção térmica independente
// Histerese: a lâmpada só religa 0,5 °C abaixo do alvo, para não ficar ligando e desligando sem parar
const float histerese = 0.5;

int intensidadeCooler = COOLER_MAXIMO;

// Tempos em milissegundos
const unsigned long tempoConversao = 750;  // quanto o DS18B20 leva para medir em 12 bits
const unsigned long tempoDebounce = 50;    // quanto o botão precisa ficar estável
const unsigned long tempoRepeticao = 400;  // repetição com o botão pressionado
unsigned long inicioConversao = 0;

// Debounce: o contato do botão "treme" ao ser apertado. Só vale a leitura que ficar estável.
struct Botao {
  int pino;
  int leitura;  // última leitura crua
  int estado;   // leitura já filtrada
  unsigned long mudou;
  unsigned long repetiu;
};

// Com INPUT_PULLUP, o botão solto lê HIGH e o apertado lê LOW
Botao botaoSobe = {butUp, HIGH, HIGH, 0, 0};
Botao botaoDesce = {butDown, HIGH, HIGH, 0, 0};

// Retorna true uma vez a cada toque e, com o botão pressionado, a cada tempoRepeticao
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

void mostraAlvo() {
  escreveLinha(1, "TempAlvo: " + String(temperaturaAlvo));
}

// Único lugar que mexe nas duas saídas: relé da lâmpada e ventilador
void aplicaSaidas(bool aquecer, int cooler) {
  digitalWrite(pinoLampada, aquecer ? RELE_LIGADO : RELE_DESLIGADO);
  intensidadeCooler = cooler;
  analogWrite(pinoCooler, intensidadeCooler);
}

// Faixa que o DS18B20 mede. Desconectado, a biblioteca devolve -127 e cai fora dela.
bool leituraValida(float t) {
  return t >= -55.0 && t <= 125.0;
}

// F("texto") deixa o texto na memória flash e poupa a RAM do UNO (só 2 KB)
void controlaTemperatura(float temperaturaAtual) {
  if (!leituraValida(temperaturaAtual)) {
    // Segurança primeiro: sem leitura confiável, nada de aquecer
    aplicaSaidas(false, COOLER_MAXIMO);
    escreveLinha(0, "ERRO NO SENSOR");
    Serial.println(F("ERRO: DS18B20 desconectado ou leitura invalida. Lampada DESLIGADA."));
    return;
  }

  escreveLinha(0, "TempC: " + String(temperaturaAtual, 2));
  const bool ligada = digitalRead(pinoLampada) == RELE_LIGADO;

  if (temperaturaAtual < (temperaturaAlvo - histerese) && !ligada) {
    aplicaSaidas(true, COOLER_MAXIMO);
    Serial.println(F("Lâmpada LIGADA (temperatura muito baixa)!"));
  } else if (temperaturaAtual >= temperaturaAlvo && ligada) {
    aplicaSaidas(false, COOLER_REPOUSO);
    Serial.println(F("Lâmpada DESLIGADA (temperatura atingida)!"));
  } else {
    // Dentro da faixa da histerese, ou a lâmpada já está como deveria
    Serial.println(F("Temperatura dentro da faixa."));
  }

  Serial.print(F("Intensidade PWM cooler: "));
  Serial.print(intensidadeCooler);
  Serial.print(F(", Temperatura: "));
  Serial.print(temperaturaAtual, 2);
  Serial.print(F(", TemperaturaAlvo: "));
  Serial.println(temperaturaAlvo);
}

// Pede uma medida ao sensor sem esperar o resultado
void pedeTemperatura() {
  sensors.requestTemperatures();
  inicioConversao = millis();
}

void setup() {
  // Antes de tudo: relé desligado. O nível é definido antes de o pino virar saída.
  digitalWrite(pinoLampada, RELE_DESLIGADO);
  pinMode(pinoLampada, OUTPUT);

  Serial.begin(9600);
  pinMode(pinoCooler, OUTPUT);
  analogWrite(pinoCooler, intensidadeCooler);
  pinMode(butUp, INPUT_PULLUP);
  pinMode(butDown, INPUT_PULLUP);

  sensors.begin();
  sensors.setResolution(12);
  sensors.setWaitForConversion(false);  // não trava o loop: os botões continuam respondendo

  lcd.begin(16, 2);
  escreveLinha(0, "TempC: ...");
  mostraAlvo();

  pedeTemperatura();
}

void loop() {
  // Os dois botões são lidos em toda volta, para o debounce de cada um ficar em dia
  const bool sobe = botaoAcionado(botaoSobe);
  const bool desce = botaoAcionado(botaoDesce);
  if (sobe || desce) {
    if (sobe) temperaturaAlvo = min(temperaturaAlvo + 0.5f, temperaturaAlvoMaxima);
    if (desce) temperaturaAlvo -= 0.5;
    mostraAlvo();
  }

  if (millis() - inicioConversao >= tempoConversao) {
    controlaTemperatura(sensors.getTempCByIndex(0));
    pedeTemperatura();
  }
}
