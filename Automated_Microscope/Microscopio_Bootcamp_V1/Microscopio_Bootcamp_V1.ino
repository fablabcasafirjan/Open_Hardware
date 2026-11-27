/*********
  Versão do firmware: 1.0.1

  Rui Santos & Sara Santos - Random Nerd Tutorials
  Complete instructions at https://RandomNerdTutorials.com/esp32-cam-projects-ebook/
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*********/

/*
  Como o programa funciona:
  - A ESP32-CAM vira um pequeno servidor web na sua rede.
  - Porta 80: entrega a página com os botões e recebe os comandos dos servos.
  - Porta 81: envia o vídeo (MJPEG: uma sequência de fotos JPEG, uma atrás da outra).
  - Tudo acontece nos servidores; por isso o loop() fica quase vazio.
*/

#include "esp_camera.h"
#include <WiFi.h>
#include "Arduino.h"
#include "soc/soc.h"             // disable brownout problems
#include "soc/rtc_cntl_reg.h"    // disable brownout problems
#include "esp_http_server.h"
#include <ESP32Servo.h>

// >>> TROQUE pelos seus dados antes de carregar. Não publique o sketch com os dados reais. <<<
const char* ssid = "TROQUE_PELO_NOME_DA_REDE";
const char* password = "TROQUE_PELA_SENHA_DO_WIFI";

#define PART_BOUNDARY "123456789000000000000987654321"

// Pinos da câmera na placa ESP32-CAM AI Thinker (definidos pela placa; não mude)
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// Servos: sinal nos GPIO 14 e 15, andando 10 graus por comando
#define SERVO_1      14
#define SERVO_2      15
#define SERVO_STEP   10
#define SERVO_MAX    180  // ESP32Servo limita write() a 0-180 graus

Servo servo1;
Servo servo2;

int servo1Pos = 90;  // os dois começam no meio do curso
int servo2Pos = 90;

static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

httpd_handle_t camera_httpd = NULL;
httpd_handle_t stream_httpd = NULL;

static const char PROGMEM INDEX_HTML[] = R"rawliteral(
<html>
  <head>
    <title>ESP32-CAM Robot</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
      body { font-family: Arial; text-align: center; margin:0px auto; padding-top: 30px;}
      table { margin-left: auto; margin-right: auto; }
      td { padding: 8 px; }
      .button {
        background-color: #2f4468;
        border: none;
        color: white;
        padding: 10px 20px;
        text-align: center;
        text-decoration: none;
        display: inline-block;
        font-size: 18px;
        margin: 6px 3px;
        cursor: pointer;
        -webkit-touch-callout: none;
        -webkit-user-select: none;
        -khtml-user-select: none;
        -moz-user-select: none;
        -ms-user-select: none;
        user-select: none;
        -webkit-tap-highlight-color: rgba(0,0,0,0);
      }
      img {  width: auto ;
        max-width: 100% ;
        height: auto ; 
      }
    </style>
  </head>
  <body>
    <h1>ESP32-CAM Pan and Tilt</h1>
    <img src="" id="photo" >
    <table>
      <tr><td colspan="3" align="center"><button class="button" onpointerdown="toggleCheckbox('up');">Up</button></td></tr>
      <tr><td align="center"><button class="button" onpointerdown="toggleCheckbox('left');">Left</button></td><td align="center"></td><td align="center"><button class="button" onpointerdown="toggleCheckbox('right');">Right</button></td></tr>
      <tr><td colspan="3" align="center"><button class="button" onpointerdown="toggleCheckbox('down');">Down</button></td></tr>                   
    </table>
   <script>
   function toggleCheckbox(x) {
     var xhr = new XMLHttpRequest();
     xhr.open("GET", "/action?go=" + x, true);
     xhr.send();
   }
   document.getElementById("photo").src = window.location.protocol + "//" + window.location.hostname + ":81/stream";
  </script>
  </body>
</html>
)rawliteral";

static esp_err_t index_handler(httpd_req_t *req){
  httpd_resp_set_type(req, "text/html");
  return httpd_resp_send(req, (const char *)INDEX_HTML, strlen(INDEX_HTML));
}

// Envia o vídeo: repete "captura uma foto e manda" até o navegador desconectar.
// Cada parte tem um separador, um cabeçalho com o tamanho e a imagem JPEG.
static esp_err_t stream_handler(httpd_req_t *req){
  char cabecalho[64];
  esp_err_t res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);

  while(res == ESP_OK){
    camera_fb_t *fb = esp_camera_fb_get();  // a câmera já entrega em JPEG (veja setup)
    if (!fb) {
      Serial.println("Camera capture failed");
      return ESP_FAIL;
    }
    size_t tamanho = snprintf(cabecalho, sizeof(cabecalho), _STREAM_PART, fb->len);
    res = httpd_resp_send_chunk(req, cabecalho, tamanho);
    if(res == ESP_OK){
      res = httpd_resp_send_chunk(req, (const char *)fb->buf, fb->len);
    }
    if(res == ESP_OK){
      res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
    }
    esp_camera_fb_return(fb);  // devolve o buffer; sem isso a câmera para de capturar
  }
  return res;
}

// Move um servo e guarda a posição, sem passar de 0 nem de 180 graus
static void moveServo(Servo &servo, int &posicao, int passo){
  posicao = constrain(posicao + passo, 0, SERVO_MAX);
  servo.write(posicao);
  Serial.println(posicao);
}

// Trata os botões da página: /action?go=up, down, left ou right
static esp_err_t cmd_handler(httpd_req_t *req){
  char consulta[32];
  char comando[8];
  if (httpd_req_get_url_query_str(req, consulta, sizeof(consulta)) != ESP_OK ||
      httpd_query_key_value(consulta, "go", comando, sizeof(comando)) != ESP_OK) {
    httpd_resp_send_404(req);
    return ESP_FAIL;
  }

  Serial.println(comando);
  if      (!strcmp(comando, "up"))    moveServo(servo1, servo1Pos,  SERVO_STEP);
  else if (!strcmp(comando, "down"))  moveServo(servo1, servo1Pos, -SERVO_STEP);
  else if (!strcmp(comando, "left"))  moveServo(servo2, servo2Pos,  SERVO_STEP);
  else if (!strcmp(comando, "right")) moveServo(servo2, servo2Pos, -SERVO_STEP);
  else return httpd_resp_send_500(req);

  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  return httpd_resp_send(req, NULL, 0);
}

// Cria os dois servidores. O vídeo fica separado para não bloquear os botões.
void startCameraServer(){
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = 80;
  httpd_uri_t index_uri = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = index_handler,
    .user_ctx  = NULL
  };

  httpd_uri_t cmd_uri = {
    .uri       = "/action",
    .method    = HTTP_GET,
    .handler   = cmd_handler,
    .user_ctx  = NULL
  };
  httpd_uri_t stream_uri = {
    .uri       = "/stream",
    .method    = HTTP_GET,
    .handler   = stream_handler,
    .user_ctx  = NULL
  };
  if (httpd_start(&camera_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(camera_httpd, &index_uri);
    httpd_register_uri_handler(camera_httpd, &cmd_uri);
  }
  config.server_port += 1;
  config.ctrl_port += 1;
  if (httpd_start(&stream_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(stream_httpd, &stream_uri);
  }
}

void setup() {
  // Desliga o detector de queda de tensão (brownout). Ele reiniciaria a placa em quedas de energia;
  // para investigar travamentos, comente a linha e observe o Monitor Serial.
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  servo1.setPeriodHertz(50);    // standard 50 hz servo
  servo2.setPeriodHertz(50);    // standard 50 hz servo
  
  servo1.attach(SERVO_1, 1000, 2000);
  servo2.attach(SERVO_2, 1000, 2000);
  
  servo1.write(servo1Pos);
  servo2.write(servo2Pos);
  
  Serial.begin(115200);
  Serial.setDebugOutput(false);
  
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG; 
  
  // PSRAM é uma memória extra da placa: com ela dá para usar dois buffers de imagem
  if(psramFound()){
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  
  // Para inverter a imagem, depois de iniciar a câmera use:
  // esp_camera_sensor_get()->set_vflip(esp_camera_sensor_get(), 1);   (ou set_hmirror)
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
  // Wi-Fi connection
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  
  Serial.print("Camera Stream Ready! Go to: http://");
  Serial.println(WiFi.localIP());
  
  // Start streaming web server
  startCameraServer();
}

void loop() {
  
}
