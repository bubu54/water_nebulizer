/* ##################   Nebulizador de agua con display LM1638  ####################
   Descripción: Envía por puerto serie texto al display JY-LKM1638
   Autor:       Alvaro Ruiz Diez
   Revisión:    19-06-2020
   Licencia:    Creative Commons Share-Alike 3.0
  ##############################################################
*/

#include <TM1638.h>

#define DATA 8
#define CLK 9
#define STROBE 7

TM1638 tm1638(DATA, CLK, STROBE);

int estado[8] = {0};         // 0=OFF, 1=30s, 2=60s, 3=90s
unsigned long lastPressTime[8] = {0};
const int duracion[4] = {0, 30, 60, 90}; // en segundos

void setup() {
  tm1638.setupDisplay(true, 7);
  tm1638.setDisplayToString("READY");

  for (int i = 0; i < 8; i++) {
    tm1638.setLED(TM1638_COLOR_RED, i, false);  // todos apagados al inicio
  }
}

void loop() {
  byte botones = tm1638.getButtons();

  for (int i = 0; i < 8; i++) {
    if (bitRead(botones, i)) {
      // Evitar rebote
      if (millis() - lastPressTime[i] > 250) {
        estado[i] = (estado[i] + 1) % 4;

        // Mostrar estado en display
        String texto = "L" + String(i + 1) + " ";
        texto += (estado[i] == 0) ? "OFF" : String(duracion[estado[i]]);
        tm1638.setDisplayToString(texto);

        // LED ON si no está en estado OFF
        tm1638.setLED(TM1638_COLOR_GREEN, i, estado[i] != 0);

        lastPressTime[i] = millis();
      }
    }
  }
}
