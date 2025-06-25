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
String textoDisplay = "        ";  // Guarda el texto principal del display

// Estructura de parpadeo
struct ParpadeoLED {
  bool activo = false;
  byte colorOriginal = TM1638_COLOR_RED;
  byte fase = 0;
  unsigned long ultimoCambio = 0;
};

ParpadeoLED parpadeos[8];
const int PARPADEO_VECES = 3;
const int PARPADEO_INTERVALO = 200;

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

  actualizarParpadeos();  // ← Aquí se actualizan los parpadeos sin bloquear
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

void iniciarParpadeo(byte boton, byte color) {
  parpadeos[boton].activo = true;
  parpadeos[boton].colorOriginal = color;
  parpadeos[boton].fase = 0;
  parpadeos[boton].ultimoCambio = millis();
}

void actualizarParpadeos() {
  unsigned long ahora = millis();

  for (int i = 0; i < 8; i++) {
    if (parpadeos[i].activo) {
      if (ahora - parpadeos[i].ultimoCambio >= PARPADEO_INTERVALO) {
        if (parpadeos[i].fase % 2 == 0) {
          tm1638.setLED(TM1638_COLOR_NONE, i);
        } else {
          tm1638.setLED(parpadeos[i].colorOriginal, i);
        }

        parpadeos[i].fase++;
        parpadeos[i].ultimoCambio = ahora;

        if (parpadeos[i].fase >= PARPADEO_VECES * 2) {
          parpadeos[i].activo = false;
          tm1638.setLED(parpadeos[i].colorOriginal, i);
        }
      }
    }
  }
}

void actualizarCuentasAtras() {
  for (int i = 0; i < 8; i++) {
    if (estados[i] > 0) {
      cuentasAtras[i]--;
      if (cuentasAtras[i] <= 0) {
        digitalWrite(pinesSalida[i], HIGH);
        delay(50);
        digitalWrite(pinesSalida[i], LOW);

        cuentasAtras[i] = tiempos[estados[i]];

        Serial.print("Salida "); Serial.print(i + 1);
        Serial.print(" activada cada "); Serial.print(tiempos[estados[i]]); Serial.println("s"); 

        byte color = (estados[i] == 0) ? TM1638_COLOR_GREEN : TM1638_COLOR_RED;
        iniciarParpadeo(i, color);
      }
    }
  }

  mostrarCuentaAtras();

  if (tiempoActual >= 86400) tiempoActual = 0;
}

void mostrarTexto(String texto) {
  while (texto.length() < 6) texto += " ";
  textoDisplay = texto;
  texto += "  ";
  tm1638.setDisplayToString(texto);
}

void mostrarCuentaAtras() {
  String texto = textoDisplay;
  while (texto.length() < 6) texto += " ";

  if (ultimoBotonActivo >= 0 && estados[ultimoBotonActivo] > 0) {
    int segundos = cuentasAtras[ultimoBotonActivo];
    segundos = constrain(segundos, 0, 99);

    int decenas = segundos / 10;
    int unidades = segundos % 10;

    texto += (decenas == 0 ? ' ' : char('0' + decenas));
    texto += char('0' + unidades);
  } else {
    texto += "  ";
  }

  tm1638.setDisplayToString(texto);
}
