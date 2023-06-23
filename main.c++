#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>
#include <HeartSensorLibrary.h>
#include <ESP8266WiFi.h>  // Biblioteca para comunicação Wi-Fi

// Configurações do Wi-Fi
const char* ssid = "NomeDaRede";
const char* password = "SenhaDaRede";

// Configurações do servidor
const char* serverAddress = "endereco_do_dashboard.com";
const int serverPort = 80;
const String endpoint = "/dados";  // Endpoint do dashboard para envio dos dados

// Configurar o objeto do acelerômetro e do sensor de frequência cardíaca/ECG
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);
HeartSensorLibrary heartSensor;

// Pinos de conexão dos sensores
const int acelerometroPino = 0;
const int ecgPino = A1;

// Função de configuração inicial
void setup() {
  Serial.begin(9600);

  // Conectar-se à rede Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando ao Wi-Fi...");
  }
  Serial.println("Conectado ao Wi-Fi!");

  // Inicializar o acelerômetro e o sensor de frequência cardíaca/ECG
  if (!accel.begin()) {
    Serial.println("Erro ao inicializar o acelerômetro.");
    while (1);
  }
  heartSensor.begin(ecgPino);
}

// Função principal
void loop() {
  // Ler os dados do acelerômetro
  sensors_event_t event;
  accel.getEvent(&event);

  // Obter a frequência cardíaca/ECG
  int frequenciaCardiaca = heartSensor.getHeartRate();

  // Enviar os dados para o dashboard
  enviarDadosParaDashboard(event.acceleration.x, event.acceleration.y, event.acceleration.z, frequenciaCardiaca);

  // Exibir os resultados
  Serial.print("Acelerômetro - X: ");
  Serial.print(event.acceleration.x);
  Serial.print(", Y: ");
  Serial.print(event.acceleration.y);
  Serial.print(", Z: ");
  Serial.println(event.acceleration.z);

  Serial.print("Frequência cardíaca: ");
  Serial.println(frequenciaCardiaca);

  delay(1000);
}

// Função para enviar os dados para o dashboard via HTTP
void enviarDadosParaDashboard(float aceleracaoX, float aceleracaoY, float aceleracaoZ, int frequenciaCardiaca) {
  // Criar o payload dos dados
  String payload = "acceleration_x=" + String(aceleracaoX) +
                   "&acceleration_y=" + String(aceleracaoY) +
                   "&acceleration_z=" + String(aceleracaoZ) +
                   "&heart_rate=" + String(frequenciaCardiaca);

  // Criar a conexão HTTP
  WiFiClient client;
  if (client.connect(serverAddress, serverPort)) {
    // Enviar a requisição POST para o endpoint do dashboard
    client.println("POST " + endpoint + " HTTP/1.1");
    client.println("Host: " + String(serverAddress));
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.println("Content-Length: " + String(payload.length()));
    client.println();
    client.println(payload);
    client.println();

    // Aguardar pela resposta do servidor
    while (client.connected() && !client.available()) {
      delay(100);
    }

    // Exibir a resposta do servidor
    while (client.available()) {
      String line = client.readStringUntil('\r');
      Serial.println(line);
    }

    client.stop();
  } else {
    Serial.println("Falha na conexão com o servidor.");
  }
}
