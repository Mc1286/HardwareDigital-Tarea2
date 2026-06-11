Se añade el temporizador de 2 segundos y el envío inicial del SOF.
// Nodo Maestro - V2: Lectura local y polling
const int pinTemp = A0; 
const int ledVerde = 13; 
const int ledRojo = 12; 

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

    int lecturaADC = analogRead(pinTemp);
    float voltaje = lecturaADC * (5.0 / 1023.0);
    float temperaturaLocal = (voltaje - 0.5) * 100.0;

    // Envio de trama basica (Sin Checksum aun)
    Serial.write(SOF);
    Serial.write(0x01); 
  }
}
