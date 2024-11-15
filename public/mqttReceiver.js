// Intenta la conexión WebSocket primero
let client = mqtt.connect('ws://148.229.22.5:3012', {
    username: 'admin',
    password: 'admin'
});

function connectToBroker() {
    client.on('connect', () => {
        console.log('Conectado al broker MQTT (WebSocket)');
        client.subscribe('semaforo/data', (err) => {
            if (!err) {
                console.log('Suscrito al tema: semaforo/data');
            } else {
                console.error('Error al suscribirse:', err);
            }
        });
    });

    client.on('message', (topic, message) => {
        console.log(`Mensaje en ${topic}: ${message.toString()}`);
        try {
            const data = JSON.parse(message);
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

    client.on('error', (error) => {
        console.error('Error de conexión WebSocket:', error);
        // Si ocurre un error en WebSocket, intenta la conexión MQTT tradicional
        fallbackToMQTT();
    });
}

// Función de respaldo para intentar la conexión MQTT tradicional
function fallbackToMQTT() {
    client = mqtt.connect('mqtt://148.229.22.5:3011', {  // Puerto MQTT tradicional
        username: 'admin',
        password: 'admin'
    });

    client.on('connect', () => {
        console.log('Conectado al broker MQTT (Tradicional)');
        client.subscribe('semaforo/data', (err) => {
            if (!err) {
                console.log('Suscrito al tema: semaforo/data');
            } else {
                console.error('Error al suscribirse:', err);
            }
        });
    });

    client.on('message', (topic, message) => {
        console.log(`Mensaje en ${topic}: ${message.toString()}`);
        try {
            const data = JSON.parse(message);
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

    client.on('error', (error) => {
        console.error('Error de conexión MQTT tradicional:', error);
    });
}

// Inicia la conexión cuando la página esté cargada
window.onload = connectToBroker;
