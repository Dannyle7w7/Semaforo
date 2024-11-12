# Usa una imagen base de Node.js
FROM node:18

# Crea y establece el directorio de trabajo en el contenedor
WORKDIR /usr/src/app

# Copia solo los archivos package.json y package-lock.json para instalar dependencias
COPY package*.json ./

# Instala las dependencias
RUN npm install

# Copia el resto de los archivos del proyecto al contenedor
COPY . .

# Expón los puertos que usa tu aplicación (Express, MQTT, WebSocket)
EXPOSE 3010 3011 3012

# Comando para ejecutar tu aplicación
CMD ["node", "server.js"]
