const express = require('express');
const path = require('path');
const aedes = require('aedes')();
const net = require('net');
const WebSocket = require('ws');
const http = require('http');

// Inicia el servidor Express
const app = express();
const port = 3010;

// Sirve los archivos estáticos desde la carpeta 'public'
app.use(express.static(path.join(__dirname, 'public')));

// Inicia el servidor web en el puerto 3000
app.listen(port, () => {
    console.log(`Servidor corriendo en http://localhost:${port}`);
});

// Configuración del broker MQTT
const mqttPort = 3011;  // Puerto donde estará escuchando el broker MQTT tradicional
const websocketPort = 3012; // Puerto para WebSocket
const USERNAME = 'admin';
const PASSWORD = 'admin';

// Crea el servidor del broker MQTT en el puerto MQTT tradicional (1883)
const mqttServer = net.createServer(aedes.handle);

// Inicia el broker MQTT
mqttServer.listen(mqttPort, function () {
    console.log(`Broker MQTT corriendo en el puerto ${mqttPort}`);
}).on('error', function (err) {
    console.error('Error al iniciar el broker MQTT:', err.message);
    process.exit(1);  // Sale si hay un error crítico
});

// Configuración del servidor WebSocket con HTTP
const server = http.createServer();
const wsServer = new WebSocket.Server({ server });

// Maneja las conexiones WebSocket
wsServer.on('connection', function (ws) {
    const stream = WebSocket.createWebSocketStream(ws);
    aedes.handle(stream);  // Maneja la conexión de WebSocket como MQTT
    console.log('Cliente WebSocket conectado');
});

// Inicia el servidor WebSocket en el puerto 3012   
server.listen(websocketPort, function () {
    console.log(`Broker MQTT corriendo en WebSocket puerto ${websocketPort}`);
});

// Configura la autenticación del broker MQTT
aedes.authenticate = function (client, username, password, callback) {
    const authorized = (username === USERNAME && password.toString() === PASSWORD);
    if (authorized) {
        console.log('Cliente autenticado:', client.id);
        callback(null, true);
    } else {
        console.log('Autenticación fallida para el cliente:', client.id);
        callback(new Error('Credenciales incorrectas'), false);
    }
};

// Maneja eventos del broker MQTT
aedes.on('client', function (client) {
    console.log('Cliente conectado:', client.id);
});

aedes.on('clientDisconnect', function (client) {
    console.log('Cliente desconectado:', client.id);
});

aedes.on('publish', function (packet, client) {
    if (client) {
        console.log('Mensaje publicado por', client.id, 'en el tema', packet.topic);
    }
});
