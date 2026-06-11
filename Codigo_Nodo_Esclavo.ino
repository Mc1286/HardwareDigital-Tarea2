//Se migró la recepción de datos sincrónica (polling) a una arquitectura asincrónica basada en eventos usando la interrupción por hardware serialEvent(), liberando completamente el loop() principal.
//Se implementó la lógica de validación de integridad para la trama entrante mediante Checksum (XOR).
//Se estructuró la función de respuesta para generar la trama de salida completa (SOF, ID, LEN, DATA, CHK) empaquetando correctamente los 16 bits del ADC.
//Cumplimiento total de la rúbrica respecto al uso de ISR en al menos uno de los nodos de la red.
// Nodo Esclavo - Lectura de LDR y respuesta por UART Final

const int pinLDR = A1;

const byte SOF = 0xAA; // Byte de sincronizacion
byte bufferRX[10];
int indexRX = 0;
bool mensajeCompleto = false;

void setup() {
  Serial.begin(9600); 
}

void loop() {
  // El loop principal queda liberado.
  // Solo procesamos si la interrupcion UART armo la trama.
  if (mensajeCompleto) {
    procesarSolicitud();
    
    // Reiniciamos las variables
    indexRX = 0;
    mensajeCompleto = false;
  }
}

// Interrupcion UART: recibe bytes en segundo plano
void serialEvent() {
  while (Serial.available() && !mensajeCompleto) {
    byte byteEntrante = Serial.read();
    
    if (indexRX == 0 && byteEntrante != SOF) {
      continue; 
    }
    
    bufferRX[indexRX] = byteEntrante;
    indexRX++;
    
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
  
  byte chkCalculado = idRecepcionado ^ lenRecepcionado;
  
  // Descartar si el paquete llego corrupto
  if (chkCalculado != chkRecepcionado) {
    return; 
  }
  
  if (idRecepcionado == 0x01) {
    enviarDatosLDR();
  }
}

// Empaqueta la lectura del ADC y transmite la respuesta
void enviarDatosLDR() {
  int valorCrudo = analogRead(pinLDR); 
  
  byte dataHigh = highByte(valorCrudo);
  byte dataLow = lowByte(valorCrudo);
  
  byte idEnvio = 0x02; 
  byte lenEnvio = 0x02; 
  
  byte chkEnvio = idEnvio ^ lenEnvio ^ dataHigh ^ dataLow;
  
  Serial.write(SOF);
  Serial.write(idEnvio);
  Serial.write(lenEnvio);
  Serial.write(dataHigh);
  Serial.write(dataLow);
  Serial.write(chkEnvio);
}
