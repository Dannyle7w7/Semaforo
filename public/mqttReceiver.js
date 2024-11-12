// Conexión con el broker MQTT mediante WebSockets
const client = mqtt.connect('ws://148.229.22.5:3012', {
    username: 'admin',
    password: 'admin'
});

// Función para conectarse al broker
function connectToBroker() {
    client.on('connect', () => {
        console.log('Conectado al broker MQTT');
        client.subscribe('semaforo/data', (err) => {
            if (!err) {
                console.log('Suscrito al tema: semaforo/data');
            } else {
                console.error('Error al suscribirse:', err);
            }
        });
    });

    // Manejo de los mensajes recibidos
    client.on('message', (topic, message) => {
        console.log(`Mensaje en ${topic}: ${message.toString()}`);
        
        // Intentar parsear el mensaje
        try {
            const data = JSON.parse(message);
            
            // Aquí actualizas el HTML con los datos del mensaje
            document.getElementById('hora').innerText = data.hora || 'No disponible';
            document.getElementById('temperatura').innerText = data.temperatura || 'No disponible';
            document.getElementById('color').innerText = data.color || 'No disponible';
            document.getElementById('humedad').innerText = data.humedad || 'No disponible';
            document.getElementById('ip').innerText = data.ip || 'No disponible';
            actualizarSemaforo(data.color);
        } catch (error) {
            console.error('Error al parsear JSON:', error);
        }
    });

    // Manejo de errores
    client.on('error', (error) => {
        console.error('Error de conexión:', error);
    });
}

// Llamar a la función para conectar cuando la página esté cargada
window.onload = connectToBroker;
