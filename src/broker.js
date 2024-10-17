const aedes = require('aedes')();
const server = require('net').createServer(aedes.handle);
const port = process.env.MQTT_PORT || 1883;  // Configuración del puerto con variable de entorno

const USERNAME = 'admin';
const PASSWORD = 'admin';

server.listen(port, function () {
  console.log('Aedes MQTT broker is up and running on port', port);
}).on('error', function (err) {
  console.error('Error starting MQTT broker:', err.message);
  process.exit(1); // Salir si no puede iniciar el servidor
});

// Configurar autenticación con usuario y contraseña
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

// Agregar más eventos para logging
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

// Manejo de excepciones no controladas
process.on('uncaughtException', function (err) {
  console.error('Excepción no controlada:', err.message);
});
