#include <TM1638.h>

#define DATA 10
#define CLK 9
#define STROBE 8

#define TM1638_COLOR_NONE   0
#define TM1638_COLOR_RED    1
#define TM1638_COLOR_GREEN  2

TM1638 tm1638(DATA, CLK, STROBE);

// Estados: 0=OFF, 1=30s, 2=60s, 3=90s
byte estados[8] = {0};
int cuentasAtras[8] = {0};
const int tiempos[4] = {0, 30, 60, 90};
const int pinesSalida[8] = {2, 3, 4, 5, 6, 10, 11, 12};

unsigned long tiempoActual = 0;
unsigned long ultimaActualizacion = 0;
byte botonesPrevios = 0;
int ultimoBotonActivo = -1;
String textoDisplay = "        ";  // Guarda el texto principal del display (6 primeros dígitos)


void setup() {
  Serial.begin(115200);
  tm1638.setupDisplay(true, 1);
  mostrarTexto("ALL OFF");

  for (int i = 0; i < 8; i++) {
    tm1638.setLED(TM1638_COLOR_NONE, i);
    pinMode(pinesSalida[i], OUTPUT);
    digitalWrite(pinesSalida[i], LOW);
  }
}

void loop() {
  if (millis() - ultimaActualizacion >= 1000) {
    ultimaActualizacion = millis();
    tiempoActual++;
    actualizarCuentasAtras();
  }

  byte botonesActuales = tm1638.getButtons();

  for (byte i = 0; i < 8; i++) {
    if ((botonesActuales & (1 << i)) && !(botonesPrevios & (1 << i))) {
      manejarPulsacion(i);
    }
  }

  botonesPrevios = botonesActuales;
}

void manejarPulsacion(byte boton) {
  estados[boton] = (estados[boton] + 1) % 4;

  if (estados[boton] == 0) {
    tm1638.setLED(TM1638_COLOR_GREEN, boton);
    cuentasAtras[boton] = 0;
  } else {
    tm1638.setLED(TM1638_COLOR_RED, boton);
    cuentasAtras[boton] = tiempos[estados[boton]];
    ultimoBotonActivo = boton;
  }

  String texto = "L" + String(boton + 1);
  if (estados[boton] == 0) {
    texto += " OFF";
  } else {
    texto += " " + String(tiempos[estados[boton]]);
  }

  mostrarTexto(texto);
}

void actualizarCuentasAtras() {
  for (int i = 0; i < 8; i++) {
    if (estados[i] > 0) {
      cuentasAtras[i]--;
      if (cuentasAtras[i] <= 0) {
        //digitalWrite(pinesSalida[i], HIGH);
        //delay(50);
        //digitalWrite(pinesSalida[i], LOW);
        cuentasAtras[i] = tiempos[estados[i]];

        Serial.print("Salida "); Serial.print(i + 1);
        Serial.print(" activada cada "); Serial.print(tiempos[estados[i]]); Serial.println("s");
      }
    }
  }

  mostrarCuentaAtras();

  if (tiempoActual >= 86400) tiempoActual = 0;
}

void mostrarTexto(String texto) {
  while (texto.length() < 6) texto += " ";  // Rellenar a 6 caracteres
  textoDisplay = texto;                    // Guardar texto principal
  texto += "  ";                           // Rellenar los últimos 2 con espacios
  tm1638.setDisplayToString(texto);
}

void mostrarCuentaAtras() {
  // Copiamos los primeros 6 caracteres del texto principal
  String texto = textoDisplay;
  while (texto.length() < 6) texto += " "; // Asegurarse de tener 6 caracteres

  if (ultimoBotonActivo >= 0 && estados[ultimoBotonActivo] > 0) {
    int segundos = cuentasAtras[ultimoBotonActivo];
    segundos = constrain(segundos, 0, 99);

    int decenas = segundos / 10;
    int unidades = segundos % 10;

    texto += (decenas == 0 ? ' ' : char('0' + decenas));
    texto += char('0' + unidades);
  } else {
    texto += "  "; // Borrar cuenta atrás si no hay botón activo
  }

  tm1638.setDisplayToString(texto);
}


