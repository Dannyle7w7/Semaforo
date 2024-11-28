    // Este código configura un ESP8266 para recibir datos de un Arduino, visualizarlos
// en una página web y enviarlos a un broker MQTT en formato JSON.
// Ultima actualizacion 28/11/2024 
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// Configuración de red WiFi
const char* ssid = "wifi_uach";          // Nombre de la red WiFi
const char* password = "";              // Contraseña de la red WiFi

// Configuración del cliente MQTT
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
const char* mqttServer = "148.229.22.5"; // Dirección IP del broker MQTT
const int mqttPort = 3011;                // Puerto del broker MQTT
const char* mqttUsername = "admin";     // Usuario del broker
const char* mqttPassword = "admin";     // Contraseña del broker
const char* mqttTopic = "semaforo/data"; // Tema para publicar datos

// Servidor web
ESP8266WebServer server(80);

// Variables para datos recibidos
String temperatura = "N/A";
String hora = "N/A";
String color = "N/A";
String humedad = "N/A";
String ip = "N/A";
bool mqttConnected = false;              // Estado de conexión al broker MQTT

// Variables para reconexión
unsigned long lastReconnectAttempt = 0;
const unsigned long reconnectInterval = 5000; // Intervalo entre intentos (ms)
const int maxReconnectAttempts = 10;         // Máximos intentos
int reconnectAttempts = 0;

// Log de estado
String logMessage = "Inicializando...\n";

void setup() {
  Serial.begin(9600);  // Inicializa la comunicación serial

  // Conexión WiFi
  WiFi.begin(ssid, password);
  Serial.print("Conectando a WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConectado a WiFi. Dirección IP: ");
  Serial.println(WiFi.localIP());
  ip = WiFi.localIP().toString();
  logMessage = "Conectado a WiFi. IP: " + ip + "\n";

  // Configuración MQTT
  mqttClient.setServer(mqttServer, mqttPort);
  connectMQTT();

  // Configuración del servidor web
  server.on("/", handleRoot);
  server.on("/update", handleUpdate);
  server.begin();
  Serial.println("Servidor iniciado");
}

void loop() {
  // Verifica conexión WiFi
  checkWiFi();

  // Lee datos desde el puerto serial
  if (Serial.available() > 0) {
    String data = Serial.readStringUntil('\n');
    processSerialData(data);
  }

  // Manejo de solicitudes HTTP
  server.handleClient();

  // Reconexión al broker MQTT si es necesario
  if (!mqttConnected) {
    if (millis() - lastReconnectAttempt >= reconnectInterval) {
      lastReconnectAttempt = millis();
      connectMQTT();
    }
  } else if (!mqttClient.connected()) {
    mqttConnected = false;
    logMessage = "Desconectado del broker MQTT.\n";
    Serial.println("Desconectado del broker MQTT.");
  }
}

// Conexión al broker MQTT
void connectMQTT() {
  if (mqttClient.connect("ESP8266Client", mqttUsername, mqttPassword)) {
    mqttConnected = true;
    reconnectAttempts = 0;
    logMessage = "Conexion exitosa al broker MQTT\n";
    Serial.println("Conectado al broker MQTT");
  } else {
    mqttConnected = false;
    reconnectAttempts++;
    logMessage = "Error de conexion al broker. Intento " + String(reconnectAttempts) + "\n";
    Serial.print("Conexion al broker MQTT fallida, rc=");
    Serial.println(mqttClient.state());

    if (reconnectAttempts >= maxReconnectAttempts) {
      logMessage = "Max. intentos alcanzado, pausa de 30 seg\n";
      delay(30000);
      reconnectAttempts = 0;
    }
  }
}

// Verifica y reconecta WiFi si es necesario
void checkWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi desconectado. Intentando reconectar...");
    WiFi.begin(ssid, password);
    unsigned long startAttemptTime = millis();

    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
      delay(500);
      Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nReconectado a WiFi.");
      ip = WiFi.localIP().toString();
      logMessage = "Reconectado a WiFi. IP: " + ip + "\n";
    } else {
      Serial.println("\nNo se pudo reconectar a WiFi.");
      logMessage = "Error reconexión WiFi.\n";
    }
  }
}

// Procesa los datos seriales recibidos
void processSerialData(String data) {
  int separatorIndex1 = data.indexOf(':');
  int separatorIndex2 = data.indexOf(':', separatorIndex1 + 1);
  int separatorIndex3 = data.indexOf(':', separatorIndex2 + 1);
  int separatorIndex4 = data.indexOf(':', separatorIndex3 + 1);
  int separatorIndex5 = data.indexOf(':', separatorIndex4 + 1);

  if (separatorIndex1 != -1 && separatorIndex5 != -1) {
    hora = data.substring(0, separatorIndex3);
    temperatura = data.substring(separatorIndex3 + 1, separatorIndex4);
    color = data.substring(separatorIndex4 + 1, separatorIndex5);
    humedad = data.substring(separatorIndex5 + 1);

    if (mqttConnected) {
      sendMQTTData();
    }
  }
}

// Manejo de la ruta raíz "/"
void handleRoot() {
  String html = "<html><head>";
  html += "<meta http-equiv='refresh' content='2'>";
  html += "</head><body>";
  html += "<h1>Datos recibidos:</h1>";
  html += "<p>Hora: " + hora + "</p>";
  html += "<p>Temperatura: " + temperatura + "</p>";
  html += "<p>Estado del semaforo: " + color + "</p>";
  html += "<p>Humedad: " + humedad + "</p>";
  html += "<p>IP: " + ip + "</p>";
  html += "<h2>Log de estado:</h2>";
  html += "<p>" + logMessage + "</p>";
  html += "</body></html>";

  server.send(200, "text/html", html);
}

// Manejo de la ruta "/update"
void handleUpdate() {
  String data = hora + "," + temperatura + "," + color + "," + humedad + "," + logMessage;
  server.send(200, "text/plain", data);
}

// Enviar datos en formato JSON a través de MQTT
void sendMQTTData() {
  StaticJsonDocument<200> jsonDoc;
  jsonDoc["hora"] = hora;
  jsonDoc["temperatura"] = temperatura;
  jsonDoc["color"] = color;
  jsonDoc["humedad"] = humedad;
  jsonDoc["ip"] = ip;
  char jsonBuffer[256];
  serializeJson(jsonDoc, jsonBuffer);

  mqttClient.publish(mqttTopic, jsonBuffer);
}