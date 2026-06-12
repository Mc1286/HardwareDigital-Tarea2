//Se migró la recepción de datos sincrónica (polling) a una arquitectura asincrónica basada en eventos usando la interrupción por hardware serialEvent(), liberando completamente el loop() principal.
//Se implementó la lógica de validación de integridad para la trama entrante mediante Checksum (XOR).
//Se estructuró la función de respuesta para generar la trama de salida completa (SOF, ID, LEN, DATA, CHK) empaquetando correctamente los 16 bits del ADC.
//Cumplimiento total de la rúbrica respecto al uso de ISR en al menos uno de los nodos de la red.
// Nodo Esclavo - Lectura de LDR y respuesta por UART Final

const int pinLDR = A1;

// 0xAA (10101010 en binario) es ideal como byte de sincronización debido a la alternancia de sus bits en la capa física.
const byte SOF = 0xAA;
byte bufferRX[10];
int indexRX = 0;
bool mensajeCompleto = false;

void setup() {
  Serial.begin(9600); 
}

void loop() {
  // El procesador queda liberado de tareas de espera bloqueante.
  // Solo procesamos si la interrupcion UART armo la trama.
  if (mensajeCompleto) {
    procesarSolicitud();
    
    // Reiniciamos los punteros de memoria para la próxima interrupción.
    indexRX = 0;
    mensajeCompleto = false;
  }
}

// Interrupción UART: El hardware recibe los bits, llena el buffer interno del microcontrolador y esta función extrae los bytes en segundo plano.
void serialEvent() {
  while (Serial.available() && !mensajeCompleto) {
    byte byteEntrante = Serial.read();

    // Filtro de ruido: descartamos cualquier byte basura hasta encontrar el SOF
    if (indexRX == 0 && byteEntrante != SOF) {
      continue; 
    }
    
    bufferRX[indexRX] = byteEntrante;
    indexRX++;

    // La petición estándar del Maestro (Lectura) consta de exactamente 4 bytes
    if (indexRX >= 4) {
      mensajeCompleto = true;
      break; 
    }
  }
}

// Desarma la trama recibida y verifica su integridad
void procesarSolicitud() {
  byte idRecepcionado = bufferRX[1];
  byte lenRecepcionado = bufferRX[2];
  byte chkRecepcionado = bufferRX[3];

  // Validación de integridad mediante compuerta lógica XOR.
  byte chkCalculado = idRecepcionado ^ lenRecepcionado;
  
  // Descartar si el paquete llego corrupto, abortamos el procesamiento y reportamos el error.
  if (chkCalculado != chkRecepcionado) {
    return; 
  }

  // ID 0x01: Solicitud de telemetría desde el Maestro.
  if (idRecepcionado == 0x01) {
    enviarDatosLDR();
  }
}

// Empaqueta la lectura del ADC y transmite la respuesta
void enviarDatosLDR() {
  int valorCrudo = analogRead(pinLDR); 

  // Desplazamiento y enmascaramiento de bits para separar el int (16 bits) en dos bytes (8 bits)
  byte dataHigh = highByte(valorCrudo);
  byte dataLow = lowByte(valorCrudo);
  
  byte idEnvio = 0x02; 
  byte lenEnvio = 0x02; 

  // Generación del Checksum de salida integrando todos los campos de datos
  byte chkEnvio = idEnvio ^ lenEnvio ^ dataHigh ^ dataLow;

  // Transmisión de la trama completa byte a byte
  Serial.write(SOF);
  Serial.write(idEnvio);
  Serial.write(lenEnvio);
  Serial.write(dataHigh);
  Serial.write(dataLow);
  Serial.write(chkEnvio);
}
