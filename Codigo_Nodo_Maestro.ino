// Este nodo implementa una arquitectura basada en Polling no bloqueante mediante la función millis(), actuando como el coordinador central de la red de telemetría. 
// Gestiona el envío de solicitudes estructuradas (SOF, ID, LEN, CHK) y cuenta con un mecanismo de Timeout de 500ms para evitar bloqueos del procesador ante la pérdida del enlace físico.
// Incorpora validación de integridad de datos entrantes mediante compuerta lógica XOR y reconstrucción de variables de 16 bits a partir de bytes individuales mediante desplazamiento (bit-shifting).

// Nodo Maestro (RX/TX) - Coordinador de Red Final

const int pinTemp  = A0; 
const int ledVerde = 13; 
const int ledRojo  = 12; 

unsigned long ultimoPolling = 0;
const unsigned long INTERVALO_POLLING = 2000; // Frecuencia de muestreo y solicitud en milisegundos
const byte SOF = 0xAA; // Byte de sincronización (10101010)

void setup() {
  Serial.begin(9600); // Configuración del hardware UART a 9600 baudios [cite: 40]
  pinMode(ledVerde, OUTPUT);
  pinMode(ledRojo, OUTPUT);
  
  digitalWrite(ledVerde, LOW);
  digitalWrite(ledRojo, LOW);
}

void loop() {
  // Polling no bloqueante: Permite ejecutar otras tareas en el procesador mientras se espera que transcurran los 2000ms.
  if (millis() - ultimoPolling >= INTERVALO_POLLING) {
    ultimoPolling = millis();
    ejecutarTelemetria();
  }
}

// Coordina la lectura local, solicita datos remotos y valida la integridad de la respuesta.
void ejecutarTelemetria() {
  // Lectura local del sensor LM35 (o similar) y conversión a grados Celsius.
  int lecturaADC = analogRead(pinTemp);
  float voltaje = lecturaADC * (5.0 / 1023.0);
  float temperaturaLocal = (voltaje - 0.5) * 100.0;

  byte idSolicitud = 0x01; 
  byte lenSolicitud = 0x00; 
  byte chkSolicitud = idSolicitud ^ lenSolicitud; 

  // Limpiar el buffer circular de hardware UART (Flush)
  // Previene que bytes huérfanos o ruido previo desfasen la lectura del nuevo paquete
  while(Serial.available() > 0) Serial.read();

  // Transmisión de la solicitud de telemetría byte a byte.
  Serial.write(SOF);
  Serial.write(idSolicitud);
  Serial.write(lenSolicitud);
  Serial.write(chkSolicitud);

  // Inicialización del mecanismo de Timeout (Recepción bloqueante controlada por tiempo)
  unsigned long tiempoInicio = millis();
  bool respuestaValida = false;
  byte bufferRX[6];
  int index = 0;

  // El ciclo se mantiene vivo mientras no superemos los 500ms de ventana de escucha.
  while (millis() - tiempoInicio < 500) {
    if (Serial.available() > 0) {
      byte byteEntrante = Serial.read();

      // Filtro de ruido: Ignoramos datos hasta interceptar el byte de sincronización.
      if (index == 0 && byteEntrante != SOF) continue; 
      
      bufferRX[index] = byteEntrante;
      index++;

      // Condición de salida: Se asume una trama fija de 6 bytes para el paquete de respuesta.
      if (index >= 6) {
        respuestaValida = true;
        break;
      }
    }
  }

  // Validacion de trama y diagnostico visual.
  if (respuestaValida) {
    byte idRx   = bufferRX[1];
    byte lenRx  = bufferRX[2];
    byte dataH  = bufferRX[3];
    byte dataL  = bufferRX[4];
    byte chkRx  = bufferRX[5];

    // Recálculo del Checksum local para contrastar con el recibido.
    byte chkCalculado = idRx ^ lenRx ^ dataH ^ dataL;

    // Aceptación de la trama: El cálculo matemático coincide y el ID es el esperado.
    if (chkCalculado == chkRx && idRx == 0x02) {
      // Trama integra
      digitalWrite(ledVerde, HIGH); 
      digitalWrite(ledRojo, LOW);   

      // Reconstrucción del entero de 16 bits usando bit-shifting a la izquierda (<< 8) y una compuerta lógica OR (|) para fusionar los bits bajos.
      int luzRemota = (dataH << 8) | dataL;

      // Impresión de resultados por consola serial.
      Serial.print("Temp Local: ");
      Serial.print(temperaturaLocal, 1);
      Serial.print(" C | Luz Remota (LDR): ");
      Serial.println(luzRemota);
    } else {
      // Error de Integridad: La trama llegó pero los datos fueron corrompidos en el canal.
      digitalWrite(ledVerde, LOW);
      digitalWrite(ledRojo, HIGH);
      Serial.println("ERROR: Trama corrupta.");
    }
  } else {
    // Error Físico: El tiempo de espera expiró sin recibir los bytes esperados.
    digitalWrite(ledVerde, LOW);  
    digitalWrite(ledRojo, HIGH); 
    Serial.println("ERROR: Timeout en la red.");
  }
}
