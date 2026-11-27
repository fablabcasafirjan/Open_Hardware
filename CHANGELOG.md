# Changelog

Registra o **firmware**, com numeração própria por projeto no formato [SemVer](https://semver.org/lang/pt-BR/). Nomes como V1, V01, V02, V03 e V06 identificam o projeto físico ou peças e não mudam com o firmware. A versão 1.0.0 corresponde ao sketch original de cada projeto. Não há tags nem releases.

**Verificação:** em 26/11/2026, os quatro sketches nas versões mais recentes abaixo compilaram com arduino-cli, usando os valores de `secrets.h.example`. As versões das ferramentas estão no README de cada projeto. **Nenhum foi testado em placa.**

## Medidor de CO₂

### 1.0.2 (26/11/2026)

- Codifica o texto da mensagem na URL do Telegram.
- Passa a iniciar a serial do S8 uma única vez, com velocidade e pinos na mesma chamada.
- Move a versão offline, adicionada na origem em 11/11/2026, para a pasta própria `Sensor_CO2_offline`. Com os dois `.ino` na mesma pasta, a compilação falhava por definições repetidas.
- Aplica à versão offline as mudanças de 1.0.1 e 1.0.2. Ela continua sem esperar o Wi-Fi.
- Ajusta o comentário sobre o endereço do display.

### 1.0.1 (26/11/2026)

- Lê rede, token e chat de um `secrets.h` local, fora do Git.
- Corrige o intervalo de notificação para 30 segundos, conforme o comentário original (o valor era 3000 ms).
- Atualiza a leitura antes de desenhar a tela.

## Microscópio automatizado

### 1.0.2 (26/11/2026)

- Limita os contadores de posição a 0–180. A biblioteca ESP32Servo já limitava o movimento a essa faixa, mas o contador passava de 180, e os primeiros comandos de volta não moviam o servo.

### 1.0.1 (26/11/2026)

- Lê o Wi-Fi de um `secrets.h` local, fora do Git.

## Incubadora

### 1.0.1 (26/11/2026)

- Usa uma única leitura de temperatura por ciclo no controle, no LCD e no Monitor Serial.
- Usa `analogWrite` também ao aplicar a potência máxima do ventilador.
- Corrige os comentários trocados dos botões, sem mudar a lógica.
- A lógica do relé, os limites e os parâmetros de controle não mudaram.

## Turbidímetro

Não tem firmware.
