# Informe Técnico del Código Arduino

## Resumen General

El código proporcionado está diseñado para controlar remotamente dispositivos físicos utilizando Arduino, ofreciendo interacción a través de una interfaz web y comunicación continua con sensores ambientales (temperatura y humedad) y actuadores (LED, relé y motor paso a paso).

## Librerías Utilizadas

* **SPI.h y Ethernet.h:**

  * Gestión de comunicaciones mediante Ethernet, habilitando el dispositivo Arduino para funcionar como servidor web.

* **AccelStepper.h:**

  * Manejo del motor paso a paso, permitiendo movimientos controlados en dirección y velocidad.

* **DHT.h:**

  * Lectura del sensor DHT11, utilizado para medir temperatura y humedad del entorno.

## Herramientas y Componentes

### Hardware

* **Sensor DHT11:** Para medir temperatura y humedad ambiente.
* **Motor Paso a Paso:** Controlado por el driver AccelStepper.
* **LED y Relé:** Dispositivos básicos controlados digitalmente para indicar estados o activar cargas mayores.
* **Shield Ethernet:** Para conectividad y configuración del servidor.

### Software

* **Servidor Web:** Implementado para ofrecer interfaz visual y recibir comandos HTTP desde un cliente remoto.
* **JSON:** Utilizado para enviar actualizaciones periódicas del estado de sensores y dispositivos mediante AJAX.
* **AJAX:** Para actualizar información del cliente de forma dinámica sin refrescar toda la página web.

## Explicación de la Lógica

### Setup Inicial

* Inicialización de comunicación serial para depuración.
* Configuración de sensores, actuadores y parámetros del motor paso a paso.
* Arranque del servidor Ethernet.
* Primera lectura de sensores para establecer valores iniciales.

### Bucle Principal (`loop()`)

La lógica se basa en prioridades claramente definidas:

1. **Movimiento del Motor:**

   * El motor se mueve constantemente mientras exista una posición objetivo configurada.

2. **Lectura de Sensores:**

   * Se realiza cada 5 segundos si el motor no está en movimiento para evitar interferencias o lecturas incorrectas.

3. **Gestión de Peticiones Web:**

   * El servidor escucha continuamente solicitudes HTTP. Al recibir una solicitud:

     * Si es una petición para datos (`GET /data`), se responde con un JSON con información actualizada de sensores y estados de dispositivos.
     * Si es cualquier otra solicitud, interpreta comandos para cambiar estados de actuadores (LED, relé, motor).

### Control de Actuadores

* **LED y Relé:** Se controlan digitalmente (ON/OFF).
* **Motor Paso a Paso:** Controlado mediante comandos específicos (`abrir` y `cerrar`) que ajustan su posición objetivo utilizando la biblioteca AccelStepper.

### Interfaz Web

* Página HTML simple que permite controlar actuadores.
* JavaScript utilizando AJAX para actualizar dinámicamente estados de dispositivos y sensores.

## Comunicación y Formato de Datos

* **Protocolo HTTP:** Utilizado para recibir comandos y enviar respuestas.
* **JSON:** Utilizado para estructurar datos enviados a la página web.
