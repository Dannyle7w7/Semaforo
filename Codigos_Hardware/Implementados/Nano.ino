// Documentación de código para semáforo con Arduino
// Este programa controla un sistema de semáforo utilizando un Arduino Nano. Su funcionalidad incluye:
// - Gestión del encendido de luces del semáforo en función de horarios configurados.
// - Manejo de un botón peatonal que modifica el flujo normal del semáforo.
// - Monitoreo y visualización de la hora actual, temperatura y humedad.
// - Manejo de un modo especial de horario y errores en el sistema.
// - Uso de un watchdog para reiniciar el sistema en caso de fallos.
//
// Componentes utilizados:
// - Módulo RTC DS3231 para sincronización de tiempo.
// - Sensor DHT22 para capturar temperatura y humedad.
// - Pantalla LCD RGB para mostrar datos.
// - Sistema de botón peatonal con lógica de interrupción.


#include <Wire.h>
#include <RTClib.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <rgb_lcd.h>
#include <avr/wdt.h>

#define DHTPIN 12
#define DHTTYPE DHT22
#define BUTTON_PIN 2
#define BUTTON_PIN_EXIT 3

#define RED1_PIN 11
#define YELLOW1_PIN 10
#define GREEN1_PIN 9

#define RED2_PIN 8
#define YELLOW2_PIN 7
#define GREEN2_PIN 6

RTC_DS3231 rtc;
DHT dht(DHTPIN, DHTTYPE);
rgb_lcd lcd;

//Variables de cambio
float Tiemporojo    =   30000;         //Este tiempo es un acumulado del verde amarillo siendo en total rojo 20 segundos 5 verde 5 amarillo
float Tiempoverde   =   50000;         //Es ese tiempo poruqe es un acumlado de tiempo que es 5 de verde parpadenate 10 de amarillo y 20 de rojo dejando 20 segundo de verde
int Iniciosemaforo  =   6;             //Hora que empieza a trabajar con regularidad el semaforo 
int Finalsemaforo   =   23;            //Hora que finaliza y se pone en amarillo 
int Inicioestacionamiento  =7;         //Hora que empieza funcionar como uno nomral para el paso de administrativo
int FinalEstacionamiento   =9;         //Hora que finaliza el funcionamiento normal y regresa a verde hasta que alguien presiona el boton

bool Specialhour = false;
bool pedestrianButtonPressed = false;
unsigned long lastMillis = 0;
unsigned long blinkMillis = 0;
bool greenBlinkState = false;
bool yellowBlinkState = false;
bool redBlinkState = false;
unsigned long lastPrintMillis = 0;
bool errorState = false;
bool rtcConnected = true;
unsigned long lastErrorCheckMillis = 0;

void setup() {
  Serial.begin(9600);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUTTON_PIN_EXIT, OUTPUT);

  pinMode(RED1_PIN, OUTPUT);
  pinMode(YELLOW1_PIN, OUTPUT);
  pinMode(GREEN1_PIN, OUTPUT);
  pinMode(RED2_PIN, OUTPUT);
  pinMode(YELLOW2_PIN, OUTPUT);
  pinMode(GREEN2_PIN, OUTPUT);

  lcd.begin(16, 2);
  lcd.setRGB(100, 0, 100);  // Set backlight color to RGB(100, 0, 100)

  if (!rtc.begin()) {
    lcd.print("RTC error!");
    errorState = true;
    rtcConnected = false;
  } else {
    rtcConnected = true;
  }

  if (rtc.lostPower()) {
    rtc.adjust(DateTime(__DATE__, __TIME__));

  }

  dht.begin();

  // Inicializar semáforos en verde
  digitalWrite(RED1_PIN, LOW);
  digitalWrite(YELLOW1_PIN, LOW);
  digitalWrite(GREEN1_PIN, HIGH);
  digitalWrite(RED2_PIN, LOW);
  digitalWrite(YELLOW2_PIN, LOW);
  digitalWrite(GREEN2_PIN, HIGH);

  //Boton entrada salida siempre en alto
  digitalWrite(BUTTON_PIN_EXIT,LOW);

  // Configurar el watchdog para un reinicio en 8 segundos si no se resetea
  wdt_enable(WDTO_8S);

  // Verificar si el reinicio fue causado por el watchdog
  if (MCUSR & _BV(WDRF)) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Watchdog Reset");
    lcd.setCursor(0, 1);
    lcd.print("System Error!");
    delay(5000);
    MCUSR &= ~_BV(WDRF);  // Limpiar el flag de reinicio del watchdog
    errorState = true;
  }
}

void loop() {
  // Verificar la conexión con el RTC cada segundo
  if (millis() - lastPrintMillis >= 1000) {
    if (!verifyRTC() || rtc.lostPower()) {
      errorState = true;
      rtcConnected = false;
    } else {
      rtcConnected = true;
      errorState = false;
    }
    DateTime now = rtc.now();
    updateLCD(now);
    lastPrintMillis = millis();
  }

  // Restablecer el watchdog timer
  wdt_reset();

  if (errorState) {
    handleErrorState();
  } else {
    handleNormalOperation();
  }
}

void handleNormalOperation() {
  DateTime now = rtc.now();
  int hour = now.hour();

//Recordatorio el boton esta siempre en abierto
  if(hour >= Inicioestacionamiento && hour < FinalEstacionamiento && !Specialhour && !pedestrianButtonPressed){
    Specialhour = true;
    lastMillis = millis();
    blinkMillis = millis();
    //                                                                                                     6                         7                               9                           23
  }else if (digitalRead(BUTTON_PIN) == LOW && !pedestrianButtonPressed && !Specialhour && ((hour >= Iniciosemaforo && hour < Inicioestacionamiento) || (hour>= FinalEstacionamiento && hour<Finalsemaforo))) {
    pedestrianButtonPressed = true;
    lastMillis = millis();
    blinkMillis = millis();
  }
  if(Specialhour){
    handleSpecialSequence();
  }else if (pedestrianButtonPressed) {
    handlePedestrianSequence();
  } else if (hour >= Iniciosemaforo && hour < Finalsemaforo) {
    handleRegularSequence();
  } else {
    handleOffHoursSequence();
  }
}

bool verifyRTC() {
  Wire.beginTransmission(0x68);  // Dirección I2C del RTC DS3231
  if (Wire.endTransmission() != 0) {
    return false;
  }
  return true;
}

void handlePedestrianSequence() {
  unsigned long currentMillis = millis();

  if (currentMillis - lastMillis < 5000) {
    if (currentMillis - blinkMillis > 500) {
      greenBlinkState = !greenBlinkState;
      digitalWrite(GREEN1_PIN, greenBlinkState);
      digitalWrite(GREEN2_PIN, greenBlinkState);
      blinkMillis = currentMillis;
    }
  } else if (currentMillis - lastMillis < 10000) {
    digitalWrite(GREEN1_PIN, LOW);
    digitalWrite(GREEN2_PIN, LOW);
    digitalWrite(YELLOW1_PIN, HIGH);
    digitalWrite(YELLOW2_PIN, HIGH);
  } else if (currentMillis - lastMillis < Tiemporojo) {  // 20 segundos en rojo
    digitalWrite(YELLOW1_PIN, LOW);
    digitalWrite(YELLOW2_PIN, LOW);
    digitalWrite(RED1_PIN, HIGH);
    digitalWrite(RED2_PIN, HIGH);
  } else  {
    pedestrianButtonPressed = false;
    digitalWrite(RED1_PIN, LOW);
    digitalWrite(RED2_PIN, LOW);
  }
}

void handleSpecialSequence() {
  unsigned long currentMillis = millis();

  if (currentMillis - lastMillis < 5000) {
    if (currentMillis - blinkMillis > 500) {
      greenBlinkState = !greenBlinkState;
      digitalWrite(GREEN1_PIN, greenBlinkState);
      digitalWrite(GREEN2_PIN, greenBlinkState);
      blinkMillis = currentMillis;
    }
  } else if (currentMillis - lastMillis < 10000) {
    digitalWrite(GREEN1_PIN, LOW);
    digitalWrite(GREEN2_PIN, LOW);
    digitalWrite(YELLOW1_PIN, HIGH);
    digitalWrite(YELLOW2_PIN, HIGH);
  } else if (currentMillis - lastMillis < Tiemporojo) {  // 20 segundos en rojo
    digitalWrite(YELLOW1_PIN, LOW);
    digitalWrite(YELLOW2_PIN, LOW);
    digitalWrite(RED1_PIN, HIGH);
    digitalWrite(RED2_PIN, HIGH);
  } else if(currentMillis - lastMillis < Tiempoverde) {
    digitalWrite(RED1_PIN, LOW);
    digitalWrite(RED2_PIN, LOW);
    digitalWrite(GREEN1_PIN, HIGH);
    digitalWrite(GREEN2_PIN, HIGH);
    
  }else {
    Specialhour = false;
  }
}

void handleRegularSequence() {
  digitalWrite(RED1_PIN, LOW);
  digitalWrite(YELLOW1_PIN, LOW);
  digitalWrite(GREEN1_PIN, HIGH);
  digitalWrite(RED2_PIN, LOW);
  digitalWrite(YELLOW2_PIN, LOW);
  digitalWrite(GREEN2_PIN, HIGH);
}

void handleOffHoursSequence() {
  unsigned long currentMillis = millis();

  if (currentMillis - blinkMillis > 250) {  // Parpadeo más rápido
    yellowBlinkState = !yellowBlinkState;
    digitalWrite(YELLOW1_PIN, yellowBlinkState);
    digitalWrite(YELLOW2_PIN, yellowBlinkState);
    blinkMillis = currentMillis;
  }

  digitalWrite(GREEN1_PIN, LOW);
  digitalWrite(RED1_PIN, LOW);
  digitalWrite(GREEN2_PIN, LOW);
  digitalWrite(RED2_PIN, LOW);
}

void handleErrorState() {
  unsigned long currentMillis = millis();
  if (currentMillis - blinkMillis > 500) {
    redBlinkState = !redBlinkState;
    digitalWrite(RED1_PIN, redBlinkState);
    digitalWrite(RED2_PIN, redBlinkState);
    blinkMillis = currentMillis;
  }
}

void updateLCD(DateTime now) {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Determinar el estado del semáforo
  String semaphoreState = "Verde"; // Estado por defecto
  if(Specialhour){
    unsigned long currentMillis = millis();
    if (currentMillis - lastMillis < 5000) {
      semaphoreState = "Verde";
    } else if (currentMillis - lastMillis < 10000) {
      semaphoreState = "Amarillo";
    } else if (currentMillis - lastMillis < Tiemporojo) {
      semaphoreState = "Rojo";
    }else if(currentMillis - lastMillis < Tiempoverde){
      semaphoreState = "Verde";
    }
  }else if (pedestrianButtonPressed) {
    unsigned long currentMillis = millis();
    if (currentMillis - lastMillis < 5000) {
      semaphoreState = "Verde";
    } else if (currentMillis - lastMillis < 10000) {
      semaphoreState = "Amarillo";
    } else if (currentMillis - lastMillis < Tiemporojo) {
      semaphoreState = "Rojo";
    }
  } else if (errorState) {
    semaphoreState = "Error";
  } else {
    int hour = now.hour();
    if (hour < Iniciosemaforo || hour >= Finalsemaforo) {
      semaphoreState = "Amarillo";
    }
  }
/*
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(now.hour());
  lcd.print(':');
  lcd.print(now.minute());
  lcd.print(':');
  lcd.print(now.second());
  lcd.print(" ");
  lcd.print(semaphoreState);*/

Serial.print(now.hour());
Serial.print(":");
Serial.print(now.minute());
Serial.print(":");
Serial.print(now.second());
Serial.print(":");
Serial.print(temperature, 1);
Serial.print(":");
Serial.print(semaphoreState);
Serial.print(":");
Serial.println(humidity);
/*
  lcd.setCursor(0, 1);
  lcd.print("Tex:");
  lcd.print(temperature,0);
  lcd.print("C ");
  lcd.print("Hum:");
  lcd.print(humidity,0);
  lcd.print("%");
  */
}

void forceRestart() {
  wdt_enable(WDTO_15MS);
  while (1) {
    // Esperar a que el watchdog reinicie el sistema
    }
} 