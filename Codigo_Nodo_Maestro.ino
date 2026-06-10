// Nodo Maestro - V1: Estructura base
const int pinTemp = A0; 
const int ledVerde = 13; 
const int ledRojo = 12; 

void setup() {
  Serial.begin(9600); 
  pinMode(ledVerde, OUTPUT);
  pinMode(ledRojo, OUTPUT);
  
  digitalWrite(ledVerde, LOW);
  digitalWrite(ledRojo, LOW);
}

void loop() {
  // Pendiente: Implementar temporizador y telemetría
}
