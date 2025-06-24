#include <TM1638.h>

#define DATA 8
#define CLK 9
#define STROBE 7

TM1638 tm1638(DATA, CLK, STROBE);

// Estados para cada botón: 0=OFF, 1=30s, 2=60s, 3=90s
byte estados[8] = {0};
const int tiempos[4] = {0, 30, 60, 90};
const int pinesSalida[8] = {2, 3, 4, 5, 6, 10, 11, 12};

unsigned long tiempoActual = 0;
unsigned long ultimaActualizacion = 0;
byte botonesPrevios = 0;

void setup() {
  Serial.begin(9600);

  // Inicializar LEDs y salidas
  for (int i = 0; i < 8; i++) {
    tm1638.setLED(i, false);  // Apagar LED
    pinMode(pinesSalida[i], OUTPUT);
    digitalWrite(pinesSalida[i], LOW);
  }

  mostrarTexto("ALi OFF");
}

void loop() {
  // Actualizar cada segundo
  if (millis() - ultimaActualizacion >= 1000) {
    ultimaActualizacion = millis();
    tiempoActual++;
    actualizarSalidas();

    // Mostrar reloj (opcional)
    mostrarTiempo(tiempoActual);
  }

  byte botonesActuales = tm1638.getButtons();

  // Detectar botones presionados
  for (byte i = 0; i < 8; i++) {
    if ((botonesActuales & (1 << i)) && !(botonesPrevios & (1 << i))) {
      manejarPulsacion(i);
    }
  }

  botonesPrevios = botonesActuales;
}

// Maneja cada pulsación de botón
void manejarPulsacion(byte boton) {
  estados[boton] = (estados[boton] + 1) % 4;

  // Cambiar LED según estado
  tm1638.setLED(boton, estados[boton] > 0);  // ON si estado > 0

  // Mostrar texto temporal
  String texto = "L" + String(boton + 1);
  if (estados[boton] == 0)
    texto += " OFF";
  else
    texto += " " + String(tiempos[estados[boton]]) + "s";

  mostrarTexto(texto);
}

// Activa salidas según tiempo programado
void actualizarSalidas() {
  for (int i = 0; i < 8; i++) {
    int tiempoProg = tiempos[estados[i]];
    if (tiempoProg > 0 && tiempoActual % tiempoProg == 0) {
      digitalWrite(pinesSalida[i], HIGH);
      delay(50);
      digitalWrite(pinesSalida[i], LOW);

      Serial.print("Salida "); Serial.print(i + 1);
      Serial.print(" activada cada "); Serial.print(tiempoProg); Serial.println("s");
    }
  }

  // Reset diario
  if (tiempoActual >= 86400)
    tiempoActual = 0;
}

// Muestra texto de hasta 8 caracteres
void mostrarTexto(String texto) {
  while (texto.length() < 8) texto += " ";  // rellenar
  for (int i = 0; i < 8; i++) {
    tm1638.setDisplay(i, texto[i]);
  }
}

// Muestra temporizador en formato MM:SS
void mostrarTiempo(unsigned long segundos) {
  int m = (segundos / 60) % 100;
  int s = segundos % 60;
  char buffer[9];
  snprintf(buffer, 9, "%02d:%02d  ", m, s);
  mostrarTexto(String(buffer));
}
