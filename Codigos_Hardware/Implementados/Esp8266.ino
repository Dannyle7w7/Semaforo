#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// Configura los datos de tu red WiFi
const char* ssid = "wifi_uach";
const char* password = ""; // Coloca la contraseña de tu red WiFi aquí

// Declara el cliente MQTT
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// Cambia esto por la IP de tu broker
const char* mqttServer = "10.20.14.250"; // IP de tu broker
const int mqttPort = 1883;  // Puerto del broker
const char* mqttUsername = "admin"; // Nombre de usuario para la autenticación
const char* mqttPassword = "admin"; // Contraseña para la autenticación
const char* mqttTopic = "semaforo/data"; // Tema al que enviamos los datos

// Crea un objeto servidor web en el puerto 80
ESP8266WebServer server(80);

// Variables globales para almacenar los datos recibidos del Arduino
String temperatura = "N/A";
String hora = "N/A";
String color = "N/A";
String humedad = "N/A";
String ip = "N/A";
bool mqttConnected = false; // Variable para rastrear la conexión al broker

void setup() {
  Serial.begin(9600);  // Inicializa la comunicación serial para la ESP8266

  // Conexión a la red WiFi
  WiFi.begin(ssid, password);
  Serial.print("Conectando a WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Conectado a WiFi. Dirección IP: ");
  Serial.println(WiFi.localIP());
  ip = WiFi.localIP().toString();
  // Configura el servidor MQTT
  mqttClient.setServer(mqttServer, mqttPort);

  // Conectar al broker MQTT
  connectMQTT();

  // Define la ruta raíz ("/") y asigna una función que se ejecutará cuando se acceda a esa ruta
  server.on("/", handleRoot);

  // Define la ruta "/update" para manejar las solicitudes de actualización de datos
  server.on("/update", handleUpdate);

  // Inicia el servidor
  server.begin();
  Serial.println("Servidor iniciado");
}

void loop() {
  // Verifica si hay datos disponibles en el puerto serial
  if (Serial.available() > 0) {
    // Lee los datos enviados por el Arduino en un formato de una sola línea:
    // Formato esperado: "hh:mm:ss:temperatura:estadoSemaforo"
    String data = Serial.readStringUntil('\n');

    // Encuentra los índices de los separadores
    int separatorIndex1 = data.indexOf(':');
    int separatorIndex2 = data.indexOf(':', separatorIndex1 + 1);
    int separatorIndex3 = data.indexOf(':', separatorIndex2 + 1);
    int separatorIndex4 = data.indexOf(':', separatorIndex3 + 1);
    int separatorIndex5 = data.indexOf(':', separatorIndex4 + 1);

    if (separatorIndex1 != -1 && separatorIndex2 != -1 && separatorIndex3 != -1 && separatorIndex4 != -1 && separatorIndex5 != -1 ) {
      hora = data.substring(0, separatorIndex3); // Hora completa (hh:mm:ss)
      temperatura = data.substring(separatorIndex3 + 1, separatorIndex4); // Temperatura
      color = data.substring(separatorIndex4 + 1,separatorIndex5); // Estado del semáforo
      humedad = data.substring(separatorIndex5 + 1);// Humedad en %
      // Publicar los datos en formato JSON a través de MQTT solo si está conectado
      if (mqttConnected) {
        sendMQTTData();
      }
    }
  }

  // Atiende las solicitudes HTTP que lleguen al servidor
  server.handleClient();

  // Intentar reconectar al broker si no está conectado
  if (!mqttConnected) {
    connectMQTT();
  }
}

// Función para conectar al broker MQTT
void connectMQTT() {
  // Intenta conectar con el nombre de cliente y las credenciales
  if (mqttClient.connect("ESP8266Client", mqttUsername, mqttPassword)) {
    mqttConnected = true; // Actualiza la variable de conexión
    Serial.println("Conectado al broker MQTT");
  } else {
    Serial.print("Conexión al broker MQTT fallida, rc=");
    Serial.println(mqttClient.state());
    delay(2000); // Espera un tiempo antes de volver a intentar
  }
}

// Función que maneja la solicitud a la ruta raíz "/"
void handleRoot() {
  // Se construye la página web que se enviará como respuesta a la solicitud
  String html = "<html><head>";
  html += "<meta http-equiv='refresh' content='1'>";  // Actualiza automáticamente la página cada 1 segundo
  html += "<script>";
  html += "function fetchData() {";
  html += "fetch('/update')";  // Solicita los datos actualizados
  html += ".then(response => response.text())";
  html += ".then(data => {";
  html += "let values = data.split(',');";
  html += "document.getElementById('hora').innerHTML = 'Hora: ' + values[0];";
  html += "document.getElementById('temperatura').innerHTML = 'Temperatura: ' + values[1];";
  html += "document.getElementById('color').innerHTML = 'Estado del semaforo: ' + values[2];";
  html += "document.getElementById('humedad').innerHTML = 'Humedad:' + values[3];";
  html += "});";
  html += "}";  
  html += "setInterval(fetchData, 1000);";  // Actualiza los datos cada 1 segundo
  html += "</script>";
  html += "</head><body onload='fetchData()'>"; // Ejecuta la función fetchData al cargar la página
  html += "<h1>Datos recibidos:</h1>"; // Título de la página
  html += "<p id='hora'>Hora: " + hora + "</p>"; // Muestra la hora recibida
  html += "<p id='temperatura'>Temperatura: " + temperatura + "</p>"; // Muestra la temperatura recibida
  html += "<p id='color'>Estado del semaforo: " + color + "</p>"; // Muestra el estado del semáforo recibido
  html += "<p id='humedad'>Humedad: " + humedad + "</p>";
  html += "</body></html>"; // Cierre del HTML

  // Enviar la respuesta HTTP con el código 200 (OK) y el contenido generado en 'html'
  server.send(200, "text/html", html);
}

// Función que maneja la solicitud de actualización de datos "/update"
void handleUpdate() {
  String data = hora + "," + temperatura + "," + color + "," + humedad;
  server.send(200, "text/plain", data);
}

// Función para enviar los datos en formato JSON a través de MQTT
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
