# Microscópio automatizado V1

[← Todos os projetos](../README.md)

Protótipo didático com ESP32-CAM, dois microservos e estrutura cortada a laser. A placa serve, na rede local, uma página com o vídeo da câmera e quatro botões que movem os servos.

## Imagens do acervo

| Mosquito | PLA impresso | Impressão digital |
| --- | --- | --- |
| <img src="Mosquito_Zoomed_Image.jpeg" alt="Imagem ampliada identificada como mosquito" width="240"> | <img src="PLA_Print_Zoomed_Image.jpeg" alt="Imagem ampliada identificada como peça impressa em PLA" width="240"> | <img src="Fingerprint_Zoomed_Image.jpeg" alt="Imagem ampliada identificada como impressão digital" width="240"> |

As legendas seguem os nomes dos arquivos. As imagens têm um botão "Save" sobreposto; as condições de aquisição não estão registradas, e elas não medem resolução óptica.

## Arquivos

| Arquivo | Conteúdo |
| --- | --- |
| [Microscopio_Bootcamp_V1.ino](Microscopio_Bootcamp_V1/Microscopio_Bootcamp_V1.ino) | Firmware 1.0.2: câmera, página HTTP e servos. |
| [Microscope_LaserCuts_V01.svg](Microscope_LaserCuts_V01.svg) | Desenho de corte a laser. |

## Materiais registrados

- Chapa de MDF de 3 mm.
- Uma ESP32-CAM e dois microservos Tower Pro SG90 de 9 g.
- Dez jumpers, uma mini protoboard e uma fonte de 5 V.
- Duas lâminas de vidro e um capacitor de 470 µF.
- Quatro parafusos M2,5 de 10 mm e quatro porcas M2,5.

## Placa e pinos

Confirmados pelo mantenedor deste fork em 26/11/2026:

| Item | Configuração |
| --- | --- |
| Placa | ESP32-CAM AI Thinker (`CAMERA_MODEL_AI_THINKER`) |
| Servo 1 (botões Up e Down) | GPIO 14 |
| Servo 2 (botões Left e Right) | GPIO 15 |

Não há esquema elétrico; a posição do capacitor e a alimentação dos servos não estão documentadas.

## Usar na Arduino IDE

1. Instale o pacote de placas **esp32** (Espressif Systems) e escolha **AI Thinker ESP32-CAM**.
2. Pelo Gerenciador de Bibliotecas, instale **ESP32Servo**.
3. Abra `Microscopio_Bootcamp_V1/Microscopio_Bootcamp_V1.ino`.
4. Copie `secrets.h.example` para `secrets.h`, na mesma pasta, e preencha a cópia local.
5. Compile e carregue. A forma de gravação depende do adaptador usado com a ESP32-CAM.
6. Abra o Monitor Serial em 115200 baud. Quando o Wi-Fi conectar, abra no navegador o endereço `http://` mostrado. O vídeo usa a porta 81.

**Compilação verificada em 26/11/2026** (arduino-cli, `esp32:esp32` 3.3.8, ESP32Servo 3.2.0). Não são necessariamente as versões originais. Não houve teste em placa.

## Limitações

- **Possível conflito entre servos e câmera, não testado em placa:** com ESP32Servo 3.2.0, os servos recebem os canais PWM 0 e 1 (timer 0), e depois o sketch configura a câmera no canal 0 e no timer 0. Os servos podem não se mover corretamente.
- A página, o vídeo e os comandos **não têm autenticação**: qualquer aparelho com acesso à mesma rede pode ver a imagem e mover os servos. Use apenas em rede confiável e não exponha à internet.
- Se a câmera não iniciar, o sketch encerra a inicialização antes de ligar o Wi-Fi; o erro aparece só no Monitor Serial.
- O sketch desativa o detector de queda de tensão (*brownout*) da ESP32.
- O firmware limita os comandos a 0–180 graus; o curso mecânico seguro da montagem não está documentado.
- Sem sequência de montagem, ajuste óptico ou calibração.

## Próximos passos

- Testar os servos com a câmera ativa e, se o conflito se confirmar, reservar outro timer para eles.
- Registrar o esquema elétrico, a montagem e o ajuste óptico.
- Medir o curso seguro dos servos na estrutura.
- Fotografar o equipamento montado e documentar as condições das imagens.

## Créditos e licença

- O sketch preserva o aviso de **Rui Santos & Sara Santos, [Random Nerd Tutorials](https://RandomNerdTutorials.com/esp32-cam-projects-ebook/)**; mantenha-o em cópias.
- Projeto do bootcamp do [FabLab Casa Firjan](https://github.com/fablabcasafirjan/Open_Hardware). Veja [créditos](../README.md#créditos) e [licenças](../README.md#licenças).

## English summary

Educational microscope with an AI Thinker ESP32-CAM, two SG90 servos and a laser-cut structure. The board serves an unauthenticated local web page with video and servo controls. The sketch compiles but was not tested on hardware.
