<div align="center">

# Medidor de CO₂ V1

**Leitura de CO₂ com sensor SenseAir S8, ESP32 e display OLED, com envio opcional ao Telegram**

![Projeto](https://img.shields.io/badge/projeto-V1-informational?style=flat)
![Placa](https://img.shields.io/badge/placa-ESP32%20DevKit-E7352C?style=flat&logo=espressif&logoColor=white)
![IDE](https://img.shields.io/badge/firmware-Arduino%20IDE-00878F?style=flat&logo=arduino&logoColor=white)
![Linguagem](https://img.shields.io/badge/linguagem-C%2B%2B-blue?style=flat&logo=c%2B%2B&logoColor=white)
![Versão](https://img.shields.io/badge/firmware-1.1.0-informational?style=flat)
![Status](https://img.shields.io/badge/status-prot%C3%B3tipo%20did%C3%A1tico-yellow?style=flat)
![Licença](https://img.shields.io/badge/licen%C3%A7a-MIT-lightgrey?style=flat)

<img src="fotos/co2-montado-leitura.jpg" alt="Medidor montado, com o display mostrando uma leitura em ppm" width="420">

<sub>Protótipo montado no bootcamp. A leitura na tela é um registro visual, sem comparação com uma referência.</sub>

[← Todos os projetos](../README.md)

</div>

---

## 📖 Visão geral

O sensor SenseAir S8 mede a concentração de CO₂, o ESP32 lê o valor pela serial e o mostra em ppm no OLED e no Monitor Serial. Com Wi-Fi, o equipamento também envia a leitura a um chat do Telegram.

### 🧪 Como funciona

1. Ao ligar, o display mostra **"Tentando conectar ao WiFi"** com uma contagem de **10 até 0** segundos.
2. Se conectar, a contagem termina antes, e o equipamento envia a mensagem "ESP32 conectado e pronto!". Se não conectar em 10 segundos, ele **segue offline**.
3. Em seguida, mostra por 3 segundos a checagem de inicialização, também enviada ao Monitor Serial: **Display: OK**, **Monitor CO2: OK** e **WiFi: OK** (ou **offline**). Se o sensor não responder, aparece **Monitor CO2: FALHA**, e o sketch para. Se o display não responder, o aviso sai só no Monitor Serial.
4. A leitura é atualizada no display cerca de duas vezes por segundo. Com Wi-Fi, uma mensagem é enviada a cada 30 segundos.

Há **um único firmware**. As leituras **não são armazenadas**: o que não é enviado se perde. No modo offline, o sketch não desliga o Wi-Fi; se a conexão acontecer depois, os envios passam a ocorrer. Esse comportamento não foi testado em placa.

### 📊 O que o protótipo entrega

| Informação | Onde aparece | Precisa de Wi-Fi? |
| --- | --- | :---: |
| CO₂ em ppm | Display OLED | Não |
| CO₂ em ppm | Monitor Serial (115200 baud) | Não |
| Mensagem com a leitura | Chat do Telegram, a cada 30 s | Sim |

### ⚠️ Limitações conhecidas

- Leituras não aferidas contra uma referência.
- Se o S8 não responder na inicialização, o sketch para até a placa ser reiniciada. Falhas durante a medição não são tratadas.
- O envio ao Telegram acontece dentro do ciclo de medição e pode atrasar a atualização do display.
- **A conexão com o Telegram não valida o certificado do servidor.** No core `esp32` 3.3.8, `http.begin(url)` sem certificado desativa essa verificação. Quem conseguir interceptar o tráfego pode obter o token do bot; use apenas em rede confiável e com um bot exclusivo para este protótipo.
- O nome da rede é impresso no Monitor Serial. A senha e o token ficam em texto puro no `.ino` e são gravados na placa.

---

## 🛠️ Ferramentas e processos

| Etapa | Ferramenta |
| --- | --- |
| Caixa em PLA | Impressora 3D Bambu Lab A1 |
| Firmware | Arduino IDE |

---

## 🧾 Lista de materiais

| Item | Qtd. | Especificação |
| --- | :---: | --- |
| Sensor de CO₂ | 1 | SenseAir S8, comunicação serial a 9600 baud |
| Microcontrolador | 1 | ESP32 Wroom 32 DevKit V1 |
| Display OLED I²C | 1 | 128 × 64, endereço `0x3C` (a lista original diz 0,92") |
| Fonte | 1 | 5 V |
| Filamento | — | PLA |
| Peças da caixa | 1 conjunto | STL V0.2.0 ou V0.3.0 (combinação a definir) |
| Placa de apoio para o ESP32 | 1 | Modelo não identificado |
| Jumpers | — | Quantidade não registrada |
| Plugue e cabo de alimentação | 1 | Conector cilíndrico; medidas não registradas |

---

## 🔧 Hardware

Ligações confirmadas pelo mantenedor deste fork em 26/11/2026:

| Sinal | ESP32 |
| --- | --- |
| TX do SenseAir S8 | GPIO 16 (RX) |
| RX do SenseAir S8 | GPIO 17 (TX) |
| OLED | I²C, endereço `0x3C`, nos pinos I²C padrão da placa (o sketch não os redefine) |

Isso não é um esquema elétrico completo: alimentação e níveis de tensão não estão documentados.

| Eletrônica | Interior da caixa |
| --- | --- |
| <img src="fotos/co2-eletronica.jpg" alt="ESP32, sensor SenseAir S8 e display OLED ligados por fios, fora da tampa" width="380"> | <img src="fotos/co2-interior.jpg" alt="Interior da caixa amarela com a eletrônica e os fios" width="380"> |

---

## 💻 Firmware

[Sensor_CO2_comDisplay_comTelegram.ino](Sensor_CO2_comDisplay_comTelegram/Sensor_CO2_comDisplay_comTelegram.ino), versão **1.1.0** ([changelog](../CHANGELOG.md)).

| Biblioteca ou pacote | Versão usada na compilação |
| --- | --- |
| Pacote de placas `esp32` (Espressif Systems) | 3.3.8 |
| EspSoftwareSerial (Dirk Kaar, Peter Lerup) | 8.1.0 (também registrada no sketch) |
| [S8_UART](https://github.com/jcomas/S8_UART/tree/v1.0.2) (Josep Comas) | 1.0.2 (também registrada no sketch) |
| Adafruit GFX Library | 1.12.6 |
| Adafruit SSD1306 | 2.5.16 |
| Adafruit BusIO | 1.17.4 |

---

## 🧰 Como reproduzir na Arduino IDE

1. Instale o pacote de placas **esp32** (Espressif Systems) e escolha **ESP32 Dev Module**.
2. Pelo Gerenciador de Bibliotecas, instale **EspSoftwareSerial 8.1.0**, **Adafruit GFX Library** e **Adafruit SSD1306** (com a dependência Adafruit BusIO).
3. Instale a **S8_UART 1.0.2**: baixe o ZIP da tag `v1.0.2` em [jcomas/S8_UART](https://github.com/jcomas/S8_UART/tree/v1.0.2) e use **Sketch > Incluir Biblioteca > Adicionar biblioteca .ZIP**.
4. Abra o `.ino` e troque os valores `TROQUE_...` pela sua rede, senha, token do bot e ID do chat. Para uso só offline, pode deixar como está. **Não publique o sketch com os seus dados reais.**
5. Compile, carregue e abra o Monitor Serial em 115200 baud.
6. Confira a checagem de inicialização no display.

## 🚦 Status

| Verificação | Situação |
| --- | --- |
| Compilação (27/11/2026, arduino-cli) | OK |
| Display e leitura do sensor no protótipo | OK |
| Wi-Fi no protótipo | OK |
| Envio ao Telegram no protótipo | OK |

Situação do protótipo informada pelo mantenedor. A contagem de Wi-Fi e a checagem de inicialização da versão 1.1.0 ainda não foram gravadas em placa.

**Próximos passos:** testar a versão 1.1.0 em placa, comparar as leituras com uma referência, publicar o esquema elétrico e definir as peças finais da caixa.

---

## 📷 Fotos e modelo 3D

| Montado, vista lateral | Peças da tampa |
| --- | --- |
| <img src="fotos/co2-montado-lateral.jpg" alt="Medidor montado visto de lado, com tampa amarela e base azul" width="320"> | <img src="fotos/co2-pecas-da-tampa.jpg" alt="Duas peças amarelas impressas em 3D que formam a parte superior" width="320"> |

Capturas do modelo 3D:

| Frente | Verso |
| --- | --- |
| <img src="CO2_Sensor_STL_FrontSide.jpeg" alt="Modelo 3D da caixa vista de frente" width="280"> | <img src="CO2_Sensor_STL_BackSide.jpeg" alt="Modelo 3D da caixa vista por trás" width="280"> |

## 🗂️ Arquivos

| Arquivo | Conteúdo |
| --- | --- |
| [Sketch](Sensor_CO2_comDisplay_comTelegram/Sensor_CO2_comDisplay_comTelegram.ino) | Firmware 1.1.0 |
| [V0.2.0: corpo](Case%20Sensor%20CO2%20V0.2.0%20Bootcamp%20Corpo.stl), [tampa do corpo](Case%20Sensor%20CO2%20V0.2.0%20Bootcamp%20Tampa%20Corpo.stl), [topo](Case%20Sensor%20CO2%20V0.2.0%20Bootcamp%20Topo.stl) | STL da caixa, revisão V0.2.0 |
| [V0.3.0: base, frente](Case%20Sensor%20CO2%20V0.3.0%20Bootcamp%20Base%20frente.stl), [base, trás](Case%20Sensor%20CO2%20V0.3.0%20Bootcamp%20Base%20tras.stl) | STL da base, revisão V0.3.0 |

## 👥 Créditos

- Projeto do bootcamp do [FabLab Casa Firjan](https://github.com/fablabcasafirjan/Open_Hardware).
- Planejamento dos equipamentos: **Willian Barela**, **João Victor Dutra** ([JVSDutra](https://github.com/JVSDutra)) e **Ricardo Michel**.
- Construção: **João Victor Dutra**, em consulta com Willian e Michel.
- Fotos em `fotos/`: **Casa Firjan**.
- Firmware: sketch de **Ricardo Michel** (15/07/2025), baseado no exemplo "Get CO2 value" da biblioteca [S8_UART](https://github.com/jcomas/S8_UART) (MIT), de **Josep Comas**.
- Carcaça: a parte superior é remix de [CO2 Weather station - case for Wemos, SSD1306, Senseair S8, MH-Z19](https://www.thingiverse.com/thing:5382369), de **localhorst_**, licenciado sob [CC BY 4.0](https://creativecommons.org/licenses/by/4.0/) (página consultada em 26/11/2026).

## 📄 Licença

MIT, a mesma do repositório original ([LICENSE](../LICENSE)). Materiais de terceiros, como o remix acima, mantêm as próprias condições. Veja [créditos](../README.md#créditos) e [licenças](../README.md#licenças).

## English summary

CO₂ meter with a SenseAir S8, an ESP32 and a 128×64 OLED. On boot, it shows a 10-second Wi-Fi countdown and a component check, then measures even without a network; readings are not stored. Single Arduino sketch. It compiles; version 1.1.0 was not tested on hardware. The upper part of the case is a remix of a CC BY 4.0 design by localhorst_.
