#include <SPI.h>
#include <Ethernet.h>
#include <AccelStepper.h>
#include <DHT.h>

// === CONFIGURACIÓN DHT11 ===
#define DHTPIN 8
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// === CONFIGURACIÓN ETHERNET ===+
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 0, 40);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);
EthernetServer server(80);

// === PINES ===
const int LED = 4;
const int RELE = 5;
#define STEP_PIN 3
#define DIR_PIN 2

// === MOTOR PASO A PASO ===
const long pasosApertura = 1500; // 5 vueltas si son 200 pasos/vuelta
AccelStepper stepper(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);

// === VARIABLES DE ESTADO ===
bool estadoLED = false;
bool estadoRELE = false;
bool ekranAbierto = false;
float temperatura = 0;
float humedad = 0;
unsigned long ultimaLecturaSensor = 0;
unsigned long ultimoAjax = 0;
const long intervaloAjax = 3000; // Actualizar cada 3 segundos

// === VARIABLES PARA TIEMPO ===
char tiempoString[20]; // Para almacenar la hora de actualización

void setup() {
  Serial.begin(9600);
  dht.begin();

  pinMode(LED, OUTPUT);
  pinMode(RELE, OUTPUT);
  digitalWrite(LED, LOW);
  digitalWrite(RELE, LOW);

  // Configuración optimizada del motor
  stepper.setMaxSpeed(300);      // Velocidad reducida para mayor torque
  stepper.setAcceleration(500);  // Aceleración en pasos/segundo²
  stepper.setCurrentPosition(0);

  // Iniciar servidor Ethernet
  Ethernet.begin(mac, ip, gateway, gateway, subnet);
  server.begin();
  Serial.print(F("Servidor iniciado en IP: "));
  Serial.println(Ethernet.localIP());
  
  // Leer sensores una vez al inicio
  leerSensores();
}

void actualizarTiempoString() {
  unsigned long tiempo = millis() / 1000; // Convertir a segundos
  unsigned long segundos = tiempo % 60;
  unsigned long minutos = (tiempo / 60) % 60;
  unsigned long horas = (tiempo / 3600) % 24;
  
  sprintf(tiempoString, "%02lu:%02lu:%02lu", horas, minutos, segundos);
}

void leerSensores() {
  temperatura = dht.readTemperature();
  humedad = dht.readHumidity();
  ultimaLecturaSensor = millis();
  actualizarTiempoString();
}

void loop() {
  // Prioridad 1: Mover el motor
  stepper.run();
  
  // Prioridad 2: Leer sensores cada 5 segundos si el motor no está en movimiento
  if (stepper.distanceToGo() == 0 && millis() - ultimaLecturaSensor > 5000) {
    leerSensores();
  }
  
  // Prioridad 3: Atender solicitudes web
  EthernetClient client = server.available();
  if (client) {
    procesarCliente(client);
  }
}

void procesarCliente(EthernetClient &client) {
  String peticion = "";
  boolean finCabecera = false;
  unsigned long tiempoInicio = millis();
  
  // Leer la petición HTTP mientras se sigue moviendo el motor
  while (client.connected() && !finCabecera && millis() - tiempoInicio < 500) {
    stepper.run(); // Mantener el motor en movimiento
    
    if (client.available()) {
      char c = client.read();
      peticion += c;
      
      if (peticion.endsWith("\r\n\r\n")) {
        finCabecera = true;
      }
    }
  }
  
  // Endpoint de datos para AJAX
  if (peticion.indexOf("GET /data") != -1) {
    enviarDatosJSON(client);
  }
  // Página web normal
  else {
    procesarComandos(peticion);
    enviarPaginaWeb(client);
  }
  
  // Cerrar conexión
  delay(1);
  client.stop();
}

void procesarComandos(String peticion) {
  if (peticion.indexOf("GET /led/on") != -1) {
    digitalWrite(LED, HIGH);
    estadoLED = true;
  } else if (peticion.indexOf("GET /led/off") != -1) {
    digitalWrite(LED, LOW);
    estadoLED = false;
  } else if (peticion.indexOf("GET /rele/on") != -1) {
    digitalWrite(RELE, HIGH);
    estadoRELE = true;
  } else if (peticion.indexOf("GET /rele/off") != -1) {
    digitalWrite(RELE, LOW);
    estadoRELE = false;
  } else if (peticion.indexOf("GET /sw3/on") != -1) {
    digitalWrite(LED, HIGH);
    digitalWrite(RELE, HIGH);
    estadoLED = true;
    estadoRELE = true;
  } else if (peticion.indexOf("GET /sw3/off") != -1) {
    digitalWrite(LED, LOW);
    digitalWrite(RELE, LOW);
    estadoLED = false;
    estadoRELE = false;
  } else if (peticion.indexOf("GET /ekran/abrir") != -1) {
    stepper.moveTo(pasosApertura);
    ekranAbierto = true;
  } else if (peticion.indexOf("GET /ekran/cerrar") != -1) {
    stepper.moveTo(0);
    ekranAbierto = false;
  }
}

void enviarDatosJSON(EthernetClient &client) {
  client.println(F("HTTP/1.1 200 OK"));
  client.println(F("Content-Type: application/json"));
  client.println(F("Connection: close"));
  client.println();
  client.print(F("{\"temp\":"));
  client.print(temperatura);
  client.print(F(",\"hum\":"));
  client.print(humedad);
  client.print(F(",\"motor\":"));
  client.print(stepper.isRunning() ? 1 : 0);
  client.print(F(",\"estado\":\""));
  client.print(ekranAbierto ? F("ABIERTO") : F("CERRADO"));
  client.print(F("\",\"tiempo\":\""));
  client.print(tiempoString);
  client.print(F("\",\"led\":\""));
  client.print(estadoLED ? F("ENCENDIDO") : F("APAGADO"));
  client.print(F("\",\"rele\":\""));
  client.print(estadoRELE ? F("ENCENDIDO") : F("APAGADO"));
  client.print(F("\"}"));
}

void enviarPaginaWeb(EthernetClient &client) {
  client.println(F("HTTP/1.1 200 OK"));
  client.println(F("Content-Type: text/html"));
  client.println(F("Connection: close"));
  client.println();
  
  // Inicio de la página web
  client.println(F("<!DOCTYPE html><html><head><title>Control Ekran</title>"));
  
  // Script AJAX para actualización automática
  client.println(F("<script>"));
  client.println(F("function actualizarSensor() {"));
  client.println(F("  var xhr = new XMLHttpRequest();"));
  client.println(F("  xhr.onreadystatechange = function() {"));
  client.println(F("    if (this.readyState == 4 && this.status == 200) {"));
  client.println(F("      var datos = JSON.parse(this.responseText);"));
  client.println(F("      document.getElementById('temp').innerHTML = datos.temp;"));
  client.println(F("      document.getElementById('hum').innerHTML = datos.hum;"));
  client.println(F("      document.getElementById('motorEstado').innerHTML = datos.motor ? 'GIRANDO' : 'DETENIDO';"));
  client.println(F("      document.getElementById('ekranEstado').innerHTML = datos.estado;"));
  client.println(F("      document.getElementById('ultimaAct').innerHTML = datos.tiempo;"));
  client.println(F("      document.getElementById('ledEstado').innerHTML = datos.led;"));
  client.println(F("      document.getElementById('releEstado').innerHTML = datos.rele;"));
  client.println(F("    }"));
  client.println(F("  };"));
  client.println(F("  xhr.open('GET', '/data', true);"));
  client.println(F("  xhr.send();"));
  client.println(F("}"));
  client.println(F("setInterval(actualizarSensor, 3000);"));
  client.println(F("</script></head><body>"));
  
  // Contenido de la página
  client.println(F("<h1>Panel de Control</h1>"));
  
  // Sección de sensores
  client.println(F("<div style='border: 1px solid #ccc; padding: 5px; margin-bottom: 10px;'>"));
  client.println(F("<h2>Sensores</h2>"));
  client.print(F("<p>Temperatura: <span id='temp'>"));
  client.print(temperatura);
  client.println(F("</span> C</p>"));
  client.print(F("<p>Humedad: <span id='hum'>"));
  client.print(humedad);
  client.println(F("</span> %</p>"));
  client.print(F("<p>Motor: <span id='motorEstado'>"));
  client.print(stepper.isRunning() ? F("GIRANDO") : F("DETENIDO"));
  client.println(F("</span></p>"));
  client.print(F("<p>Ekran: <span id='ekranEstado'>"));
  client.print(ekranAbierto ? F("ABIERTO") : F("CERRADO"));
  client.println(F("</span></p>"));
  client.print(F("<p>Led: <span id='ledEstado'>"));
  client.print(estadoLED ? F("ENCENDIDO") : F("APAGADO"));
  client.println(F("</span></p>"));
  client.print(F("<p>RELE: <span id='releEstado'>"));
  client.print(estadoRELE ? F("ENCENDIDO") : F("APAGADO"));
  client.println(F("</span></p>"));
  client.print(F("<p>Ultima actualizacion: <span id='ultimaAct'>"));
  client.print(tiempoString);
  client.println(F("</span></p>"));
  client.println(F("</div>"));
  
  // Controles
  client.println(F("<p><a href='/led/on'><button>LED ON</button></a> | <a href='/led/off'><button>LED OFF</button></a></p>"));
  client.println(F("<p><a href='/rele/on'><button>RELE ON</button></a> | <a href='/rele/off'><button>RELE OFF</button></a></p>"));
  client.println(F("<p><a href='/sw3/on'><button>SWITCH 3 ON</button></a> | <a href='/sw3/off'><button>SWITCH 3 OFF</button></a></p>"));
  client.println(F("<p><a href='/ekran/abrir'><button>Abrir Ekran</button></a> | <a href='/ekran/cerrar'><button>Cerrar Ekran</button></a></p>"));
  
  client.println(F("</body></html>"));
}