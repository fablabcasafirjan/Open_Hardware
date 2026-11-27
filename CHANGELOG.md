# Changelog

Registra o **firmware**, com numeração própria por projeto no formato [SemVer](https://semver.org/lang/pt-BR/). Os quatro projetos estão na **V1**; peças e desenhos têm revisões `V0.x.0`. A versão de firmware 1.0.0 corresponde ao sketch original de cada projeto. Não há tags nem releases.

**Verificação (27/11/2026):** os três sketches compilaram com arduino-cli. O da incubadora também rodou num ATmega328 simulado, sem sensor. **Nenhuma destas versões foi testada em placa.** As versões das ferramentas estão no README de cada projeto.

## Medidor de CO₂

### 1.1.0 (27/11/2026)

- Na inicialização, mostra no OLED "Tentando conectar ao WiFi" com contagem de 10 a 0 segundos.
- Sem conexão em 10 segundos, segue para a medição offline; antes, o sketch principal esperava o Wi-Fi sem limite.
- Mostra a checagem de inicialização no OLED e no Monitor Serial: display, monitor de CO₂ e Wi-Fi (OK ou offline).
- Passa a existir um único firmware: a versão offline separada, adicionada na origem em 11/11/2026, foi removida.
- Rede, senha, token e chat voltam ao `.ino`, com valores de exemplo `TROQUE_...`.
- O display é iniciado antes do Wi-Fi.
- Codifica o texto da mensagem na URL do Telegram.
- Inicia a serial do S8 uma única vez, com velocidade e pinos na mesma chamada.
- Corrige o intervalo de notificação para 30 segundos, conforme o comentário original (o valor era 3000 ms).
- Atualiza a leitura antes de desenhar a tela.
- As leituras não são armazenadas.

## Microscópio automatizado

### 1.0.1 (27/11/2026)

- Rede e senha ficam no `.ino`, com valores de exemplo `TROQUE_...`.
- Limita os contadores de posição dos servos a 0–180; antes, o contador passava de 180, e os primeiros comandos de volta não moviam o servo.
- Monta o endereço do vídeo a partir do nome do servidor, sem depender do formato da URL da página.
- Remove a condição de largura no fluxo de vídeo que, em resoluções de até 400 pixels, levava a enviar um quadro vazio.

## Incubadora/Estufa

### 1.0.1 (27/11/2026)

- Coloca o relé em `HIGH` (lâmpada desligada) logo no início, antes de o pino virar saída.
- Desliga a lâmpada, leva o ventilador ao máximo e mostra "ERRO NO SENSOR" quando o DS18B20 é desconectado ou a leitura é inválida.
- Limita a temperatura alvo a 60 °C.
- Adiciona *debounce* aos botões, com repetição quando pressionados, e lê o sensor sem travar o loop.
- Limpa a linha inteira do LCD a cada escrita.
- Usa uma única leitura de temperatura por ciclo no controle, no LCD e no Monitor Serial.
- Usa `analogWrite` também ao aplicar a potência máxima do ventilador.
- Corrige os comentários trocados dos botões.

## Turbidímetro

Não tem firmware.
