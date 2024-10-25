import paho.mqtt.client as mqtt
import json
import time
import requests

# Configuración del broker MQTT
broker = "192.168.1.18"  # Cambia esto por la IP de tu broker
port = 1883
mqttUser = "admin"  # Usuario del broker MQTT
mqttPassword = "admin"  # Contraseña del broker MQTT
topic = "semaforo/data"  # Tema donde se enviarán los datos

# Coordenadas para Chihuahua
lat = 28.635  # Latitud de Chihuahua
lon = -106.088  # Longitud de Chihuahua
api_key = "3b0aeb5a413ba9e1fc40954e0db731d3"  # Reemplaza esto con tu API Key de OpenWeatherMap
url = f"https://api.openweathermap.org/data/2.5/weather?lat={lat}&lon={lon}&appid={api_key}&units=metric"  # Añade &units=metric para obtener la temperatura en Celsius

# Función de conexión al broker
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Conectado al broker MQTT")
    else:
        print(f"Error de conexión: {rc}")

# Función para obtener la temperatura
def get_temperature():
    try:
        response = requests.get(url)
        data = response.json()

        # Mostrar la respuesta completa
        print("Respuesta completa de la API:", data)

        # Verificar si existe la clave 'main' en la respuesta
        if 'main' in data:
            temperatura = data['main']['temp']
        else:
            temperatura = "No disponible"
            print("Error al obtener la temperatura: 'main' no encontrada")

        return temperatura
    except Exception as e:
        print(f"Error al obtener la temperatura: {e}")
        return "No disponible"

# Función para enviar datos
def send_data():
    #temperatura = get_temperature()  # Obtener temperatura de la API
    temperatura = 30.4
    hora_actual = time.strftime("%H:%M:%S")  # Obtener hora actual

    # Datos a enviar
    data = {
        "hora": hora_actual,
        "temperatura": temperatura,
        "color": "amarillo"
    }

    # Convertir los datos a formato JSON
    json_data = json.dumps(data)

    # Publicar los datos en el tema MQTT
    result = client.publish(topic, json_data)

    # Comprobar si la publicación fue exitosa
    if result.rc == mqtt.MQTT_ERR_SUCCESS:
        print(f"Datos enviados: {json_data}")
    else:
        print("Error al enviar los datos")

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
        time.sleep(1)  # Envía los datos cada 5 segundos
except KeyboardInterrupt:
    print("Desconexión")
finally:
    client.loop_stop()
    client.disconnect()
