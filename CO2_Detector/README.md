# Medidor de CO₂

[← Todos os projetos](../README.md)

Protótipo com sensor SenseAir S8, ESP32 e display OLED. Mostra a leitura em ppm no display e no Monitor Serial e, quando há Wi-Fi, envia mensagens a um chat do Telegram a cada 30 segundos.

## Modelo da caixa

Capturas do modelo 3D (não são fotos do equipamento montado):

| Frente | Verso |
| --- | --- |
| <img src="CO2_Sensor_STL_FrontSide.jpeg" alt="Modelo 3D da caixa vista de frente" width="300"> | <img src="CO2_Sensor_STL_BackSide.jpeg" alt="Modelo 3D da caixa vista por trás" width="300"> |

## Arquivos

| Arquivo | Conteúdo |
| --- | --- |
| [Sensor_CO2_comDisplay_comTelegram.ino](Sensor_CO2_comDisplay_comTelegram/Sensor_CO2_comDisplay_comTelegram.ino) | Firmware 1.0.2. **Só começa a medir depois que o Wi-Fi conecta.** |
| [Sensor_CO2_offline.ino](Sensor_CO2_offline/Sensor_CO2_offline.ino) | Firmware 1.0.2. Não espera o Wi-Fi: mede e mostra mesmo sem rede. Ainda tenta conectar e envia mensagens quando há conexão. |
| [V02: corpo](Case%20Sensor%20CO2%20V02%20Bootcamp%20Corpo.stl), [tampa do corpo](Case%20Sensor%20CO2%20V02%20Bootcamp%20Tampa%20Corpo.stl), [topo](Case%20Sensor%20CO2%20V02%20Bootcamp%20Topo.stl) | STL da caixa V02. |
| [V03: base, frente](Case%20Sensor%20CO2%20V03%20Bootcamp%20Base%20frente.stl), [base, trás](Case%20Sensor%20CO2%20V03%20Bootcamp%20Base%20tras.stl) | STL da base V03. |

A combinação final de peças V02 e V03 não está documentada.

## Materiais registrados

- Filamento PLA.
- Sensor de CO₂ SenseAir S8.
- ESP32 Wroom 32 DevKit V1.
- Display OLED I²C (a lista original diz 0,92"), 128 × 64, endereço `0x3C`.
- Fonte de 5 V.

## Placa e ligações

Confirmadas pelo mantenedor deste fork em 26/11/2026:

| Sinal | ESP32 |
| --- | --- |
| TX do SenseAir S8 | GPIO 16 (RX) |
| RX do SenseAir S8 | GPIO 17 (TX) |
| OLED | I²C, endereço `0x3C`, nos pinos I²C padrão da placa (o sketch não os redefine) |

Isso não é um esquema elétrico completo: alimentação e níveis de tensão não estão documentados.

## Usar na Arduino IDE

1. Instale o pacote de placas **esp32** (Espressif Systems) e escolha **ESP32 Dev Module**.
2. Instale **EspSoftwareSerial 8.1.0** (Dirk Kaar, Peter Lerup), **Adafruit GFX Library** e **Adafruit SSD1306** (com a dependência Adafruit BusIO).
3. Instale **S8_UART 1.0.2** (Josep Comas): baixe o ZIP da tag `v1.0.2` em [jcomas/S8_UART](https://github.com/jcomas/S8_UART/tree/v1.0.2) e use **Sketch > Incluir Biblioteca > Adicionar biblioteca .ZIP**.
4. Escolha um dos dois sketches da tabela e abra o `.ino`.
5. Na pasta dele, copie `secrets.h.example` para `secrets.h` e preencha a cópia local com a rede, o token do bot e o ID do chat. A versão offline também precisa do arquivo para compilar.
6. Compile, carregue e abra o Monitor Serial em 115200 baud.

**Compilação dos dois sketches verificada em 26/11/2026** com os valores de exemplo (arduino-cli, `esp32:esp32` 3.3.8, EspSoftwareSerial 8.1.0, S8_UART 1.0.2, Adafruit GFX 1.12.6, SSD1306 2.5.16, BusIO 1.17.4). Não houve teste em placa nem envio ao Telegram.

## Limitações

- Leituras não aferidas neste repositório.
- Se o S8 não responder na inicialização, o sketch para até a placa ser reiniciada. Falhas durante a medição não são tratadas.
- O envio ao Telegram acontece dentro do ciclo de medição e pode atrasar a atualização do display.
- O nome da rede é impresso no Monitor Serial. `secrets.h` fica em texto puro no computador, e os valores são gravados na placa.

## Próximos passos

- Fotografar o equipamento montado.
- Registrar o esquema elétrico e a combinação de peças da caixa.
- Testar os dois firmwares em placa e comparar as leituras com uma referência.

## Créditos e licença

- Sketch de **Ricardo Michel** (15/07/2025), baseado no exemplo "Get CO2 value" da biblioteca [S8_UART](https://github.com/jcomas/S8_UART) (MIT), de **Josep Comas**.
- A parte superior da caixa é um remix de [CO2 Weather station - case for Wemos, SSD1306, Senseair S8, MH-Z19](https://www.thingiverse.com/thing:5382369), de **localhorst_**, licenciado sob [CC BY 4.0](https://creativecommons.org/licenses/by/4.0/) (página consultada em 26/11/2026).
- Projeto do bootcamp do [FabLab Casa Firjan](https://github.com/fablabcasafirjan/Open_Hardware). Veja [créditos](../README.md#créditos) e [licenças](../README.md#licenças).

## English summary

CO₂ meter with a SenseAir S8, an ESP32 and a 128×64 OLED. Two Arduino sketches: one waits for Wi-Fi, the other starts without a network. Both compile; neither was tested on hardware. The upper part of the case is a remix of a CC BY 4.0 design by localhorst_.
