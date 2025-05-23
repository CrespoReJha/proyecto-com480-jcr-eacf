# Informe del Código de Arduino

## Integrantes
* **Jhamil Crespo Rejas -** Ingeniería en Ciencias de la Computación
* **Enrique Antonio Calderon -** Ingeniería en Ciencias de la Computación

## Resumen General

Este informe detalla minuciosamente el funcionamiento y la lógica del código Arduino proporcionado, diseñado para controlar dispositivos físicos de forma remota mediante una interfaz web. El sistema gestiona sensores ambientales de temperatura y humedad, así como actuadores que incluyen un LED, un relé y un motor paso a paso.

## Librerías Utilizadas

* **SPI.h y Ethernet.h:**

  * Gestionan la comunicación mediante Ethernet, permitiendo al Arduino actuar como servidor web, recibir comandos y enviar información actualizada constantemente a los clientes conectados.

* **AccelStepper.h:**

  * Se emplea para gestionar el motor paso a paso, proporcionando control preciso sobre movimientos, dirección, velocidad y aceleración.

* **DHT.h:**

  * Específica para leer datos del sensor DHT11, que mide temperatura y humedad ambiental.

## Herramientas y Componentes

### Hardware

* **Sensor DHT11:** Mide temperatura y humedad ambiental en tiempo real.
* **Motor Paso a Paso:** Permite movimientos precisos y controlados, facilitando ajustes mecánicos detallados.
* **LED y Relé:** Indicadores visuales y controladores de cargas eléctricas mayores, manejados mediante señales digitales.
* **Shield Ethernet:** Conectividad Ethernet que permite al Arduino actuar como servidor web.

### Software

* **Servidor Web:** Permite interacción con usuarios remotos mediante comandos HTTP.
* **JSON:** Envía información de sensores y actuadores en formato estructurado.
* **AJAX:** Actualiza dinámicamente la información en la interfaz web del cliente sin recargar la página completa.

## Explicación de la Lógica

### Setup Inicial

* **Comunicación Serial:** Se inicia con una velocidad de 9600 baudios para depuración mediante `Serial.begin(9600);`.
* **Configuración del DHT11:** Se inicializa el sensor para lecturas ambientales mediante `dht.begin();`.
* **Configuración de Pines:** Se establecen pines digitales como salida para el LED y el relé mediante `pinMode()`.
* **Motor Paso a Paso:** Configuración inicial mediante la función `stepper.setMaxSpeed()` y `stepper.setAcceleration()` para determinar su comportamiento.
* **Servidor Ethernet:** Se configura con dirección IP, gateway y subnet mediante `Ethernet.begin()` y se inicia el servidor con `server.begin();`.

### Bucle Principal (`loop()`)

#### 1. Movimiento del Motor:

* El motor se ejecuta continuamente hacia la posición objetivo configurada mediante `stepper.run();`, facilitando movimientos suaves y precisos.

#### 2. Lectura de Sensores:

* Si el motor no está en movimiento (`stepper.distanceToGo() == 0`) y han pasado más de 5 segundos desde la última lectura (`millis() - ultimaLecturaSensor > 5000`), se realiza una nueva lectura mediante `leerSensores();`.

#### 3. Gestión de Peticiones Web:

* El servidor escucha continuamente solicitudes mediante `server.available();`. Al recibir una solicitud:

  * Captura la petición HTTP en forma de cadena.
  * Si la petición es hacia la ruta `/data`, responde con información estructurada en JSON mediante `enviarDatosJSON()`.
  * Para otras rutas, procesa comandos mediante `procesarComandos()` y responde con una interfaz HTML generada por `enviarPaginaWeb()`.

### Funciones Clave Detalladas

#### **leerSensores():**

* Captura la temperatura y humedad mediante `dht.readTemperature()` y `dht.readHumidity()`.
* Registra el tiempo de la última lectura y actualiza el tiempo en formato legible.

#### **procesarComandos():**

* Analiza la petición HTTP recibida para ejecutar acciones específicas, activando o desactivando el LED, relé o configurando movimientos del motor paso a paso (abrir/cerrar).

#### **enviarDatosJSON():**

* Construye y envía un objeto JSON con datos actuales del sistema, incluyendo temperatura, humedad, estado del motor, LED, relé y tiempo de actualización.

#### **enviarPaginaWeb():**

* Genera una página HTML con controles interactivos para gestionar remotamente los actuadores.
* Incluye un script AJAX que actualiza dinámicamente la información mostrada, proporcionando una interfaz interactiva y actualizada continuamente.

## Comunicación y Formato de Datos

* **Protocolo HTTP:** Recibe comandos y envía respuestas estructuradas entre el servidor Arduino y clientes web.
* **Formato JSON:** Estructura la información enviada al cliente para simplificar la interpretación y el procesamiento en la interfaz.
