/*
Código da Incubadora V03 com:
- Controle ON/OFF de temperatura com a lâmpada 250W,
- Sensoreamento de temperatura com o DS18B20, 
- 2 Botões para definir a temperatura alvo,
- Cooler para fluxo de ar dentro da incubadora.
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
const int butDown = 9; // Pino de INPUT com PULLUP para subir a temperatura alvo
const int butUp = 13; // Pino de INPUT com PULLUP para descer a temperatura alvo

float temperaturaAlvo = 37.5;     // Temperatura alvo desejada para a incubadora em graus Celsius (ex: 37.5°C para ovos)
float histerese = 0.5; // Valor de histerese em graus Celsius para não ficar com a lâmpada ligando e desligando muito rápido.


void setup() {
  // Start serial communication for debugging purposes
  Serial.begin(9600);

  pinMode(cooler, OUTPUT);
  pinMode(pinoLampada, OUTPUT);
  pinMode(butUp, INPUT_PULLUP);
  pinMode(butDown, INPUT_PULLUP);

  // Start up the library
  sensors.begin();
  sensors.setResolution(0, 12);
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("TempC: ");

  lcd.setCursor(0, 1);

  lcd.print("TempAlvo: ");

  lcd.print(String(temperaturaAlvo));

  analogWrite(cooler, intensidadeCooler);
}

void loop() {
  sensors.requestTemperatures();
  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  lcd.setCursor(7, 0);
  // print the number of seconds since reset:
  lcd.print(sensors.getTempCByIndex(0), 2);

  float temperaturaAtual = sensors.getTempCByIndex(0);

  int releState = digitalRead(pinoLampada);

  if (temperaturaAtual < (temperaturaAlvo - histerese) && releState == HIGH) {
      // Se a temperatura estiver abaixo do temperaturaAlvo - histerese E a lâmpada estiver desligada,
      // liga a lâmpada para aquecer.
      // ATENÇÃO: Se o seu relé liga com HIGH, troque LOW por HIGH e vice-versa aqui e no 'else if'
      digitalWrite(pinoLampada, LOW);
      intensidadeCooler = 255;
      digitalWrite(cooler, intensidadeCooler);
      Serial.println("Lâmpada LIGADA (temperatura muito baixa)!");
    } else if (temperaturaAtual >= temperaturaAlvo && releState == LOW) {
      // Se a temperatura estiver acima ou igual ao temperaturaAlvo E a lâmpada estiver ligada,
      // desliga a lâmpada para parar de aquecer.
      // ATENÇÃO: Se o seu relé liga com HIGH, troque HIGH por LOW e vice-versa aqui e no 'if'
      digitalWrite(pinoLampada, HIGH);
      intensidadeCooler = 122;
      analogWrite(cooler, intensidadeCooler);
      Serial.println("Lâmpada DESLIGADA (temperatura atingida)!");
    } else {
      // Caso a temperatura esteja dentro da banda de histerese
      // ou a lâmpada já esteja no estado correto, não faz nada.
      Serial.println("Temperatura dentro da faixa.");
    }

  if (digitalRead(butUp) == LOW) {
    temperaturaAlvo += 0.5;
    lcd.setCursor(10,1);
    lcd.print(temperaturaAlvo);
  }
  if (digitalRead(butDown) == LOW) {
    temperaturaAlvo -= 0.5;
    lcd.setCursor(10,1);
    lcd.print(String(temperaturaAlvo));
  }

  Serial.print("Intensidade PWM cooler: ");
  Serial.print(intensidadeCooler);
  Serial.print(", ");
  Serial.print("Temperatura: ");
  Serial.print(sensors.getTempCByIndex(0), 2);
  Serial.print(", ");
  Serial.print("TemperaturaAlvo: ");
  Serial.println(temperaturaAlvo);
}
