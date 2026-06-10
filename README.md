# Tarea 2: Sistema de Comunicación Serial (Red de dos sensores)

## Descripción del Sistema
Este proyecto implementa una red de telemetría maestro-esclavo utilizando dos placas Arduino UNO comunicadas vía UART. El sistema consolida la información de dos sensores distintos:
* **Nodo Maestro:** Lee la temperatura local mediante un sensor TMP36 y actúa como coordinador de la red.
* **Nodo Esclavo:** Lee el nivel de luz mediante un sensor LDR (divisor de voltaje) y espera las solicitudes del maestro para enviar su información.
El maestro realiza *polling* a la red cada 2 segundos. El esclavo utiliza lectura no bloqueante para recibir las tramas, procesarlas y responder con sus datos, los cuales son consolidados y mostrados por el maestro a través del Monitor Serie.

## Parámetros de la Capa Física (UART)
* **Velocidad (Baud rate):** 9600 bps
* **Bits de datos:** 8 bits
* **Paridad:** Ninguna (None)
* **Bits de parada:** 1 bit

## Estructura de la Trama de Mensaje
El protocolo de comunicación define tramas con la siguiente estructura y un mecanismo de detección de errores mediante Checksum (XOR):

| Campo | Tamaño | Descripción | Ejemplo MSG Solicitud (Maestro) | Ejemplo MSG Respuesta (Esclavo) |
| :--- | :--- | :--- | :--- | :--- |
| **SOF** | 1 byte | Byte de inicio (Sincronización). | `0xAA` | `0xAA` |
| **ID** | 1 byte | Identificador del mensaje. | `0x01` (Solicitud) | `0x02` (Respuesta) |
| **LEN** | 1 byte | Longitud de la carga útil (Datos). | `0x00` (0 bytes) | `0x02` (2 bytes) |
| **DATA** | N bytes | Carga útil (Valor crudo LDR dividido en High/Low). | N/A | `0x03` `0xFF` (Para valor 1023) |
| **CHK** | 1 byte | Checksum (XOR de ID + LEN + DATA). | `0x01` | `0xFC` |

## Instrucciones de Compilación y Ejecución
1. Clonar este repositorio en la máquina local.
2. Cargar el código `maestro.ino` en la primera placa Arduino UNO.
3. Cargar el código `esclavo.ino` en la segunda placa Arduino UNO.
4. Conectar los pines TX del Maestro al RX del Esclavo, y viceversa. Unir las tierras (GND) de ambas placas.
5. Abrir el Monitor Serie conectado al Nodo Maestro configurado a 9600 baudios para visualizar la telemetría consolidada y los diagnósticos de red.

