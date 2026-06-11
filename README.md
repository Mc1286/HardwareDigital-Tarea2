# Tarea 2: Sistema de Comunicación Serial (Red de dos sensores)

## Equipo de Desarrollo y Roles
Para dar cumplimiento a la pauta de evaluación, se documenta la siguiente distribución de roles técnicos para el proyecto:
* **Mathias Salinas:** *Documentador*
* **Matias Cabrera:** *desarrollo de firmware*
* **Juan Cortez:** *desarrollo de firmware*
* **Juan Pablo Villarroel:** *jefe*
* **Fabián Mamani:** *Arquitecto del Protocolo*

---

## Descripción del Sistema
Este proyecto implementa una red de telemetría maestro-esclavo utilizando dos placas Arduino UNO comunicadas vía UART. El sistema consolida la información de dos sensores distintos:
* **Nodo Maestro:** Lee la temperatura local mediante un sensor TMP36 y actúa como coordinador de la red.
* **Nodo Esclavo:** Lee el nivel de luz mediante un sensor LDR (divisor de voltaje) y espera las solicitudes del maestro para enviar su información.

> **Detalle Técnico:** El maestro realiza *polling* asíncrono a la red cada 2 segundos. El esclavo utiliza lectura no bloqueante vía interrupción (`serialEvent()`) para recibir las tramas, procesarlas y responder, optimizando el uso del procesador según los requerimientos de la asignatura.

---

## Parámetros de la Capa Física (UART)
* **Velocidad (Baud rate):** 9600 bps
* **Bits de datos:** 8 bits
* **Paridad:** Ninguna (None)
* **Bits de parada:** 1 bit

---

## Estructura de la Trama de Mensaje
El protocolo de comunicación define tramas con la siguiente estructura y un mecanismo de detección de errores mediante Checksum (XOR):

| Campo | Tamaño | Descripción | Ejemplo MSG Solicitud (Maestro) | Ejemplo MSG Respuesta (Esclavo) |
| :--- | :---: | :--- | :---: | :---: |
| **SOF** | 1 byte | Byte de inicio (Sincronización). | `0xAA` | `0xAA` |
| **ID** | 1 byte | Identificador del mensaje. | `0x01` (Solicitud) | `0x02` (Respuesta) |
| **LEN** | 1 byte | Longitud de la carga útil (Datos). | `0x00` (0 bytes) | `0x02` (2 bytes) |
| **DATA** | N bytes| Carga útil (Valor crudo LDR dividido en High/Low). | N/A | `0x03` `0xFF` *(Ej. valor 1023)* |
| **CHK** | 1 byte | Checksum (XOR de ID + LEN + DATA). | `0x01` | `0xFC` |

---

## Diagrama del Montaje (Tinkercad)
A continuación, se presenta el esquema de conexiones y la disposición de hardware utilizada para la simulación y validación del sistema.

![Diagrama del Circuito](montaje_tinkercad.png)

---

## Instrucciones de Compilación y Ejecución
1. Clonar este repositorio en la máquina local.
2. Abrir y cargar el código localizado en la carpeta `/Maestro/Maestro.ino` en la primera placa Arduino UNO.
3. Abrir y cargar el código localizado en la carpeta `/Esclavo/Esclavo.ino` en la segunda placa Arduino UNO.
4. Conectar físicamente (o en el simulador) el pin **TX (1)** del Maestro al pin **RX (0)** del Esclavo, y viceversa (TX Esclavo a RX Maestro). Unir las tierras (**GND**) de ambas placas.
5. Abrir el Monitor Serie conectado exclusivamente al Nodo Maestro, configurado a 9600 baudios, para visualizar la telemetría consolidada y los diagnósticos de red.
