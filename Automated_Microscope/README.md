<div align="center">

# Microscópio automatizado V1

**Estrutura cortada a laser com ESP32-CAM, dois microservos e controle pelo navegador**

![Projeto](https://img.shields.io/badge/projeto-V1-informational?style=flat)
![Placa](https://img.shields.io/badge/placa-ESP32--CAM%20AI%20Thinker-E7352C?style=flat&logo=espressif&logoColor=white)
![IDE](https://img.shields.io/badge/firmware-Arduino%20IDE-00878F?style=flat&logo=arduino&logoColor=white)
![Linguagem](https://img.shields.io/badge/linguagem-C%2B%2B-blue?style=flat&logo=c%2B%2B&logoColor=white)
![Versão](https://img.shields.io/badge/firmware-1.0.1-informational?style=flat)
![Status](https://img.shields.io/badge/status-prot%C3%B3tipo%20did%C3%A1tico-yellow?style=flat)
![Licença](https://img.shields.io/badge/licen%C3%A7a-MIT-lightgrey?style=flat)

<img src="fotos/microscopio-montado.jpg" alt="Microscópio montado em MDF, com lâmina, elásticos, servo e fios" width="420">

<sub>Protótipo montado no bootcamp.</sub>

[← Todos os projetos](../README.md)

</div>

---

## 📖 Visão geral

A ESP32-CAM observa a amostra e serve, na rede local, uma página com o vídeo e quatro botões. Os botões comandam dois microservos.

### 🧪 Como é usado

1. A placa conecta ao Wi-Fi e mostra o endereço no Monitor Serial.
2. No navegador, a página exibe o vídeo da câmera.
3. Os botões **Up/Down** e **Left/Right** movem os servos.

<img src="fotos/microscopio-pagina-web.jpg" alt="Tela de celular com a página ESP32-CAM Pan and Tilt, vídeo e botões" width="240">

Captura feita durante o bootcamp, com uma versão anterior do firmware.

### 📊 Recursos do firmware

| Recurso | Como funciona |
| --- | --- |
| Vídeo | Fluxo MJPEG na porta 81 |
| Controle | Página na porta 80, passos de 10 graus, comandos entre 0 e 180 |
| Rede | Wi-Fi da própria instalação, configurado no `.ino` |

### ⚠️ Limitações conhecidas

- **Às vezes trava** com os servos e a câmera em uso (relato do mantenedor). A causa ainda não foi identificada. Não é disputa de PWM: com ESP32Servo 3.2.0 e o core 3.3.8, os servos usam os canais LEDC 0 e 1 do grupo de alta velocidade, e a câmera usa o canal 0 do grupo de baixa velocidade. A testar: alimentação dos servos (o sketch desativa o detector de queda de tensão) e sinal de Wi-Fi.
- A página, o vídeo e os comandos **não têm autenticação**: qualquer aparelho com acesso à mesma rede pode ver a imagem e mover os servos. Use apenas em rede confiável e não exponha à internet.
- Se a câmera não iniciar, o sketch encerra a inicialização antes de ligar o Wi-Fi; o erro aparece só no Monitor Serial.
- O sketch desativa o detector de queda de tensão (*brownout*) da ESP32.
- O curso mecânico seguro dos servos nesta montagem não está documentado.
- Sem sequência de montagem, ajuste óptico ou calibração.

---

## 🛠️ Ferramentas e processos

| Etapa | Ferramenta |
| --- | --- |
| Desenho das peças | Inkscape |
| Corte do MDF | Cortadora a laser CO₂ de 80 W (marca e modelo a confirmar) |
| Firmware | Arduino IDE |

---

## 🧾 Lista de materiais

| Item | Qtd. | Especificação |
| --- | :---: | --- |
| Chapa de MDF | 1 | 3 mm |
| ESP32-CAM | 1 | AI Thinker |
| Microservo | 2 | Tower Pro SG90, 9 g |
| Jumpers | 10 | — |
| Mini protoboard | 1 | — |
| Fonte | 1 | 5 V |
| Lâminas de vidro | 2 | — |
| Capacitor | 1 | 470 µF |
| Parafusos | 4 | M2,5 × 10 mm |
| Porcas | 4 | M2,5 |
| Suporte da lente e discos cinza | — | Material e fabricação não registrados |
| Elásticos | — | — |
| Conector de alimentação | — | Modelo não registrado |
| Suporte de bateria usado em bancada | — | Não se sabe se faz parte da montagem final |

<img src="fotos/microscopio-componentes.jpg" alt="Componentes do microscópio separados sobre uma folha" width="420">

---

## 🔧 Hardware

Confirmado pelo mantenedor deste fork em 26/11/2026:

| Item | Configuração |
| --- | --- |
| Placa | ESP32-CAM AI Thinker (`CAMERA_MODEL_AI_THINKER`) |
| Servo 1 (botões Up e Down) | GPIO 14 |
| Servo 2 (botões Left e Right) | GPIO 15 |

Não há esquema elétrico; a posição do capacitor e a alimentação dos servos não estão documentadas.

| Eletrônica | Tampa aberta | Teste de bancada |
| --- | --- | --- |
| <img src="fotos/microscopio-eletronica.jpg" alt="ESP32-CAM com lente, mini protoboard, capacitor e servo" width="240"> | <img src="fotos/microscopio-tampa-aberta.jpg" alt="Tampa do microscópio aberta, com servo e guias" width="240"> | <img src="fotos/microscopio-teste-servos.jpg" alt="ESP32-CAM ligada a dois servos sobre a bancada" width="240"> |

---

## 💻 Firmware

[Microscopio_Bootcamp_V1.ino](Microscopio_Bootcamp_V1/Microscopio_Bootcamp_V1.ino), versão **1.0.1** ([changelog](../CHANGELOG.md)).

| Biblioteca ou pacote | Versão usada na compilação |
| --- | --- |
| Pacote de placas `esp32` (Espressif Systems) | 3.3.8 |
| ESP32Servo | 3.2.0 |

Não são necessariamente as versões originais.

---

## 🧰 Como reproduzir na Arduino IDE

1. Instale o pacote de placas **esp32** (Espressif Systems) e escolha **AI Thinker ESP32-CAM**.
2. Pelo Gerenciador de Bibliotecas, instale **ESP32Servo**.
3. Abra `Microscopio_Bootcamp_V1/Microscopio_Bootcamp_V1.ino`.
4. No `.ino`, troque os valores `TROQUE_...` pelo nome e pela senha da sua rede. **Não publique o sketch com os seus dados reais.**
5. Compile e carregue. A gravação depende do adaptador. Em adaptadores USB-serial sem reset automático, ligue **GPIO 0 ao GND**, reinicie a placa e carregue; depois, **desfaça a ligação** e reinicie.
6. Abra o Monitor Serial em 115200 baud. Quando o Wi-Fi conectar, abra no navegador o endereço `http://` mostrado.

## 🚦 Status

| Verificação | Situação |
| --- | --- |
| Compilação (27/11/2026, arduino-cli) | OK |
| Servos com a câmera ativa, no protótipo | Funcionam; às vezes trava |
| Teste em placa da versão 1.0.1 | **Não feito** |
| Resolução óptica | **Não medida** |

**Próximos passos:** reproduzir o travamento com o Monitor Serial aberto, testar os servos com fonte separada (GND comum), publicar o esquema, a montagem e o ajuste óptico e medir o curso seguro dos servos.

---

## 📷 Fotos e imagens obtidas

<img src="fotos/microscopio-vista-superior.jpg" alt="Microscópio visto de cima" width="320">

| Mosquito | PLA impresso | Impressão digital |
| --- | --- | --- |
| <img src="Mosquito_Zoomed_Image.jpeg" alt="Imagem ampliada identificada como mosquito" width="240"> | <img src="PLA_Print_Zoomed_Image.jpeg" alt="Imagem ampliada identificada como peça impressa em PLA" width="240"> | <img src="Fingerprint_Zoomed_Image.jpeg" alt="Imagem ampliada identificada como impressão digital" width="240"> |

As legendas das imagens ampliadas seguem os nomes dos arquivos. Elas têm um botão "Save" sobreposto; as condições de aquisição não estão registradas, e elas não medem resolução óptica.

## 🗂️ Arquivos

| Arquivo | Conteúdo |
| --- | --- |
| [Sketch](Microscopio_Bootcamp_V1/Microscopio_Bootcamp_V1.ino) | Firmware 1.0.1: câmera, página HTTP e servos |
| [Microscope_LaserCuts_V0.1.0.svg](Microscope_LaserCuts_V0.1.0.svg) | Desenho de corte a laser (revisão V0.1.0) |

## 👥 Créditos

- Projeto do bootcamp do [FabLab Casa Firjan](https://github.com/fablabcasafirjan/Open_Hardware).
- Planejamento dos equipamentos: **Willian Barela**, **João Victor Dutra** ([JVSDutra](https://github.com/JVSDutra)) e **Ricardo Michel**.
- Construção: **João Victor Dutra**, em consulta com Willian e Michel.
- Fotos em `fotos/`: **Casa Firjan**.
- Firmware baseado no projeto de **Rui Santos & Sara Santos, [Random Nerd Tutorials](https://RandomNerdTutorials.com/esp32-cam-projects-ebook/)**; mantenha o aviso do sketch em cópias.

## 📄 Licença

MIT, a mesma do repositório original ([LICENSE](../LICENSE)). O aviso de terceiros no sketch é mantido. Veja [créditos](../README.md#créditos) e [licenças](../README.md#licenças).

## English summary

Educational microscope with an AI Thinker ESP32-CAM, two SG90 servos and a laser-cut structure. The board serves an unauthenticated local web page with video and servo controls. The sketch compiles but this version was not tested on hardware.
