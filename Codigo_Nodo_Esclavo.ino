// Nodo Esclavo - V2: Respuesta en el loop
const int pinLDR = A1;
const byte SOF = 0xAA;

void setup() {
  Serial.begin(9600); 
}

void loop() {
  if (Serial.available() >= 2) {
    if (Serial.read() == SOF) {
      byte idRecepcionado = Serial.read();
      
      if (idRecepcionado == 0x01) {
        int valorCrudo = analogRead(pinLDR); 
        Serial.write(SOF);
        Serial.write(0x02);
        Serial.write(highByte(valorCrudo));
        Serial.write(lowByte(valorCrudo));
      }
    }
  }
}
