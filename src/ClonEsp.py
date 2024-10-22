#pip install paho-mqtt Para librerias

import paho.mqtt.client as mqtt
import json
import time

# Configuración del broker MQTT
broker = "localhost"  # Cambia esto por la IP de tu broker
port = 1883
mqttUser = "tu_usuario"  # Usuario del broker MQTT
mqttPassword = "tu_contraseña"  # Contraseña del broker MQTT
topic = "semaforo/datos"  # Tema donde se enviarán los datos

# Función de conexión al broker
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Conectado al broker MQTT")
    else:
        print(f"Error de conexión: {rc}")

# Función para enviar datos
def send_data():
    # Datos estáticos que simulan los enviados por la ESP8266
    data = {
        "hora": "14:35",
        "temperatura": "25.6",
        "color": "verde"
    }

    # Convertir los datos a formato JSON
    json_data = json.dumps(data)

    # Publicar los datos en el tema MQTT
    client.publish(topic, json_data)
    print(f"Datos enviados: {json_data}")

# Crear el cliente MQTT
client = mqtt.Client()

# Asignar la función de conexión
client.on_connect = on_connect

# Configurar las credenciales para el broker
client.username_pw_set(mqttUser, mqttPassword)

# Conectar al broker
client.connect(broker, port)

# Esperar hasta que se conecte
client.loop_start()

# Simular el envío de datos cada 5 segundos
try:
    while True:
        send_data()
        time.sleep(5)  # Envía los datos cada 5 segundos
except KeyboardInterrupt:
    print("Desconexión")
finally:
    client.loop_stop()
    client.disconnect()

