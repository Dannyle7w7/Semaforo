const express = require('express');
const app = express();
const port = 3000;

// Sirve archivos estáticos desde la carpeta 'public'
app.use(express.static('public'));

app.listen(port, () => {
    console.log(`Servidor corriendo en http://localhost:${port}`);
});
