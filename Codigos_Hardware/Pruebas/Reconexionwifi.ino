  19 de noviembre

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
  const char* mqttServer = "148.229.22.5"; // IP de tu broker
  const int mqttPort = 3011;  // Puerto del broker
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

  // Variables para la reconexión al broker MQTT
  unsigned long lastReconnectAttempt = 0;
  const unsigned long reconnectInterval = 5000; // Tiempo de espera entre intentos de reconexión (5 segundos)
  const int maxReconnectAttempts = 10;
  int reconnectAttempts = 0;

  // Log para estado de conexión
  String logMessage = "Inicializando...\n";

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
    logMessage = "Conectado a WiFi. IP: " + ip + "\n";

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
    // Verificar y reconectar WiFi si es necesario
    checkWiFi();

    // Verifica si hay datos disponibles en el puerto serial
    if (Serial.available() > 0) {
      String data = Serial.readStringUntil('\n');

      // Procesa los datos recibidos
      int separatorIndex1 = data.indexOf(':');
      int separatorIndex2 = data.indexOf(':', separatorIndex1 + 1);
      int separatorIndex3 = data.indexOf(':', separatorIndex2 + 1);
      int separatorIndex4 = data.indexOf(':', separatorIndex3 + 1);
      int separatorIndex5 = data.indexOf(':', separatorIndex4 + 1);

      if (separatorIndex1 != -1 && separatorIndex2 != -1 && separatorIndex3 != -1 && separatorIndex4 != -1 && separatorIndex5 != -1) {
        hora = data.substring(0, separatorIndex3); 
        temperatura = data.substring(separatorIndex3 + 1, separatorIndex4); 
        color = data.substring(separatorIndex4 + 1, separatorIndex5);
        humedad = data.substring(separatorIndex5 + 1);

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
      if (millis() - lastReconnectAttempt >= reconnectInterval) {
        lastReconnectAttempt = millis(); // Actualiza el tiempo del último intento
        connectMQTT(); // Intenta reconectar
      }
    } else if (!mqttClient.connected()) {
      mqttConnected = false;
      logMessage = "Desconectado del broker MQTT.\n";
      Serial.println("Desconectado del broker MQTT.");
    }
  }

  // Función para conectar al broker MQTT
  void connectMQTT() {
    if (mqttClient.connect("ESP8266Client", mqttUsername, mqttPassword)) {
      mqttConnected = true;
      reconnectAttempts = 0; // Reinicia el contador de intentos
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
        delay(30000);  // Pausa de 30 segundos
        reconnectAttempts = 0; // Reinicia después de la pausa
      }
    }
  }

  // Función para verificar y reconectar WiFi
  void checkWiFi() {
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi desconectado. Intentando reconectar...");
      WiFi.begin(ssid, password);
      unsigned long startAttemptTime = millis();

      // Esperar hasta 10 segundos para reconectar
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

  // Función que maneja la solicitud a la ruta raíz "/"
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

  // Función que maneja la solicitud de actualización de datos "/update"
  void handleUpdate() {
    String data = hora + "," + temperatura + "," + color + "," + humedad + "," + logMessage;
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
