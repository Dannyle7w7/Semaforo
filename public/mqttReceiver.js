console.log('mqttReceiver.js cargado correctamente');  // Verifica que el script se carga

// Función para conectarse al broker MQTT
function connectToBroker() {
    const brokerUrl = 'mqtt://10.20.8.199:1883';  // Cambia a la IP correcta si no es localhost
    const topic = 'semaforo/data';
    const clientId = 'mqttjs_' + Math.random().toString(16).substr(2, 8);

    console.log('Intentando conectar al broker en ' + brokerUrl);

    // Conectar al broker MQTT
    const client = mqtt.connect(brokerUrl, {
        username: 'admin',
        password: 'admin'
    });

    client.on('connect', function () {
        console.log('Conectado al broker MQTT con éxito');
        client.subscribe(topic, function (err) {
            if (!err) {
                console.log('Suscrito al tema ' + topic);
            } else {
                console.error('Error al suscribirse al tema:', err);
            }
        });
    });

    client.on('message', function (topic, message) {
        console.log('Mensaje recibido del tema', topic, ':', message.toString());
        // Aquí puedes procesar los datos recibidos
    });

    client.on('error', function (err) {
        console.error('Error en la conexión MQTT:', err);
    });
}

// Ejecutar la función al cargar la página
window.onload = function() {
    connectToBroker();
};
