window.onload = function() {
    // Aquí puedes añadir código para actualizar los valores de los sensores y la hora
    const tempValue = document.getElementById('temp-value');
    const humValue = document.getElementById('hum-value');
    const dateValue = document.getElementById('date-value');
    const timeValue = document.getElementById('time-value');

    // Simulación de datos
    tempValue.innerText = "20.5°C";
    humValue.innerText = "10.50%";

    const updateTime = () => {
        const now = new Date();
        const options = { weekday: 'long', year: 'numeric', month: 'long', day: 'numeric' };
        dateValue.innerText = now.toLocaleDateString('es-ES', options);
        timeValue.innerText = now.toLocaleTimeString('es-ES');
    };

    setInterval(updateTime, 1000);
    updateTime();
};
