#include <ESP8266WiFi.h>
#include <Wire.h>
#include <DHT.h>


// Definições de rede e autenticação Blynk
const char* ssid = "VIVOFIBRA-35D8";
const char* password = "Br14151610";
const char* auth = "tXm9jypggWts9fmR8Dnt0NfqTG727g0l";
#define BLYNK_TEMPLATE_ID "TMPL2hjD7jp-l"
#define BLYNK_TEMPLATE_NAME "irrigação"

// Definições de conexão HTTPS
const char* host = "script.google.com";
const int httpsPort = 443;
const char* GAS_ID = "AKfycbwACw4nniztYMXEDuLsHHAW3YJTAieKEhqfq8zYnv-Wi9OBNeA7lkrHrhhWh6eYoIWq3Q";

// Definições de pinos
#define RelayPin 4
#define VPIN_BUTTON V5
#define VPIN_RAW 15
#define VPIN_RAW_MIN 16
#define VPIN_RAW_MAX 17

#include <BlynkSimpleEsp8266.h>
const int dhtPin = 2; // Pino do sensor DHT22

// Variáveis de controle
bool toggleState = LOW;
int msensor = A0;
int mspercent;
int msvalue = 0;
unsigned long interval = 3600000; 
unsigned long previousMillis = 0;
unsigned long lastConnectionTime = 0;
const unsigned long reconnectInterval = 10 * 1000;
bool internetConnected = false;

// Inicialização
DHT dht(dhtPin, DHT22);

// Variáveis para calibração do sensor
int rawMin = 282;   // Valor mínimo inicial
int rawMax = 640;   // Valor máximo inicial

void setup() {
  Serial.begin(115200);
  Blynk.begin(auth, ssid, password);
  dht.begin();
  pinMode(msensor, INPUT);
  pinMode(RelayPin, OUTPUT);
  digitalWrite(RelayPin, HIGH);
  Blynk.virtualWrite(V5, toggleState);
}

void loop() {
  if (!Blynk.connected()) {
    unsigned long currentMillis = millis();
    if (currentMillis - lastConnectionTime >= reconnectInterval) {
      lastConnectionTime = currentMillis;
      Serial.println("Attempting to reconnect to Blynk...");
      Blynk.connect();
    }
  } else {
    internetConnected = true;
  }

  Blynk.run();

  // Ler o valor atual de msvalue
  msvalue = analogRead(msensor);

  // Enviar o valor do sensor para o aplicativo Blynk
  Blynk.virtualWrite(VPIN_RAW, msvalue);

  // Verificar se houve alteração nos valores dos sliders de calibração
  Blynk.syncVirtual(VPIN_RAW_MIN);
  Blynk.syncVirtual(VPIN_RAW_MAX);

  // Atualizar o valor de mspercent com base nos limites de calibração
  mspercent = map(msvalue, rawMin, rawMax, 100, 0);

  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  Blynk.virtualWrite(V1, mspercent);
  Blynk.virtualWrite(V3, humidity);
  Blynk.virtualWrite(V10, temperature);
  Blynk.syncVirtual(V5);

  if (!internetConnected && mspercent < 30) {
    digitalWrite(RelayPin, LOW);
  } else if (!internetConnected && mspercent >= 80) {
    digitalWrite(RelayPin, HIGH);
  } else if (mspercent < 40) {
    Blynk.logEvent("notify");
  }

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    sendData(temperature, humidity, mspercent);
    Serial.println(msvalue);
  }
}

BLYNK_WRITE(V5) {
  toggleState = param.asInt();
  if (toggleState == 1) {
    digitalWrite(RelayPin, HIGH);
  } else { 
    digitalWrite(RelayPin, LOW);
  }
}

// Capturar os valores dos sliders e armazená-los nas variáveis correspondentes
BLYNK_WRITE(VPIN_RAW_MIN) {
  rawMin = param.asInt();
}

BLYNK_WRITE(VPIN_RAW_MAX) {
  rawMax = param.asInt();
}

void sendData(float tem, int hum, int ms) {
  Serial.println("==========");
  Serial.print("connecting to ");
  Serial.println(host);

  WiFiClientSecure client;
  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return;
  }

  String string_temperature = String(tem);
  String string_humidity = String(hum, DEC);
  String string_ms = String(ms, DEC);

  String url = "/macros/s/" + String(GAS_ID) + "/exec?temperature=" + string_temperature + "&humidity=" + string_humidity + "&Msensor=" + string_ms ;
  Serial.print("requesting URL: ");
  Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: BuildFailureDetectorESP8266\r\n" +
               "Connection: close\r\n\r\n");

  Serial.println("request sent");

  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
}
