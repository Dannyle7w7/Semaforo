
// Incluimos la biblioteca mqtt.js usando un script dinámico
const mqttScript = document.createElement('script');
mqttScript.src = "https://unpkg.com/mqtt/dist/mqtt.min.js";
document.head.appendChild(mqttScript);

mqttScript.onload = function () {
    // Función para conectarse al broker MQTT y recibir datos
    function connectToBroker() {
        const brokerUrl = 'ws://localhost:8080';  // Cambia esto por la URL del broker en formato WebSocket
        const topic = 'semaforo/datos';  // El tema al que se suscribirá
        const clientId = 'mqttjs_' + Math.random().toString(16).substr(2, 8);  // ID único del cliente MQTT

        // Conectar al broker MQTT
        const client = mqtt.connect(brokerUrl, {
            username: 'tu_usuario',       // Coloca tu usuario del broker
            password: 'tu_contraseña'     // Coloca tu contraseña del broker
        });

        // Evento cuando se conecta correctamente
        client.on('connect', function () {
            console.log('Conectado al broker MQTT');
            client.subscribe(topic, function (err) {
                if (!err) {
                    console.log('Suscrito al tema ' + topic);
                }
            });
        });

        // Evento cuando recibe un mensaje del broker
        client.on('message', function (topic, message) {
            console.log('Mensaje recibido:', message.toString());

            // Aquí puedes procesar los datos recibidos (asumiendo que es un JSON)
            const data = JSON.parse(message.toString());

            // Actualizar los elementos de la página con los datos recibidos
            document.getElementById('hora').innerText = 'Hora: ' + data.hora;
            document.getElementById('temperatura').innerText = 'Temperatura: ' + data.temperatura;
            document.getElementById('color').innerText = 'Estado del semáforo: ' + data.color;
        });
    }

    // Ejecutar la función cuando se cargue la página
    window.onload = function() {
        connectToBroker();
    };
};
