//Se incorpora la espera de 500ms, la compuerta XOR y el manejo de los estados de error.

// Nodo Maestro (RX/TX) - Coordinador de Red Final

const int pinTemp  = A0; 
const int ledVerde = 13; 
const int ledRojo  = 12; 

unsigned long ultimoPolling = 0;
const unsigned long INTERVALO_POLLING = 2000; 
const byte SOF = 0xAA; 

void setup() {
  Serial.begin(9600); 
  pinMode(ledVerde, OUTPUT);
  pinMode(ledRojo, OUTPUT);
  
  digitalWrite(ledVerde, LOW);
  digitalWrite(ledRojo, LOW);
}

void loop() {
  if (millis() - ultimoPolling >= INTERVALO_POLLING) {
    ultimoPolling = millis();
    ejecutarTelemetria();
  }
}

// Coordina la lectura local, solicita datos remotos y valida la integridad
void ejecutarTelemetria() {
  int lecturaADC = analogRead(pinTemp);
  float voltaje = lecturaADC * (5.0 / 1023.0);
  float temperaturaLocal = (voltaje - 0.5) * 100.0;

  byte idSolicitud = 0x01; 
  byte lenSolicitud = 0x00; 
  byte chkSolicitud = idSolicitud ^ lenSolicitud; 

  // Limpiar buffer de entrada
  while(Serial.available() > 0) Serial.read();

  Serial.write(SOF);
  Serial.write(idSolicitud);
  Serial.write(lenSolicitud);
  Serial.write(chkSolicitud);

  // Recepcion bloqueante con timeout de 500ms
  unsigned long tiempoInicio = millis();
  bool respuestaValida = false;
  byte bufferRX[6];
  int index = 0;

  while (millis() - tiempoInicio < 500) {
    if (Serial.available() > 0) {
      byte byteEntrante = Serial.read();
      
      if (index == 0 && byteEntrante != SOF) continue; 
      
      bufferRX[index] = byteEntrante;
      index++;
      
      if (index >= 6) {
        respuestaValida = true;
        break;
      }
    }
  }

  // Validacion de trama y diagnostico visual
  if (respuestaValida) {
    byte idRx   = bufferRX[1];
    byte lenRx  = bufferRX[2];
    byte dataH  = bufferRX[3];
    byte dataL  = bufferRX[4];
    byte chkRx  = bufferRX[5];

    byte chkCalculado = idRx ^ lenRx ^ dataH ^ dataL;

    if (chkCalculado == chkRx && idRx == 0x02) {
      // Trama integra
      digitalWrite(ledVerde, HIGH); 
      digitalWrite(ledRojo, LOW);   

      int luzRemota = (dataH << 8) | dataL;

      Serial.print("Temp Local: ");
      Serial.print(temperaturaLocal, 1);
      Serial.print(" C | Luz Remota (LDR): ");
      Serial.println(luzRemota);
    } else {
      // Error de Checksum
      digitalWrite(ledVerde, LOW);
      digitalWrite(ledRojo, HIGH);
      Serial.println("ERROR: Trama corrupta.");
    }
  } else {
    // Timeout
    digitalWrite(ledVerde, LOW);  
    digitalWrite(ledRojo, HIGH); 
    Serial.println("ERROR: Timeout en la red.");
  }
}
