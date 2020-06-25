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

TM1638 tm1638(DATA, CLK, STROBE);  // Define modulo y pines

String txt = "";       // variable con el texto a imprimir
int segundos[8];
byte botones;
byte apagar = 0;
int sec_act = 0;
int rele[8];

void setup() {

  pinMode(rele[0], OUTPUT); //modo salida

  //module1.setDisplayToString("ALL OFF");
  tm1638.setupDisplay(true, 1);

  for (int i = 0; i < 8; i++)
  {
    segundos[i] = 60;
    tm1638.setLED(TM1638_COLOR_GREEN, i);
  }

}
// =============================================================

void loop() {
  
  for (int boton = 1; boton < 9; boton++) {
    // Let the TM1638 process the button inputs
    botones = tm1638.getButtons();
    if (isButtonBeingPressed(boton)) {

      delay(200);

      if (segundos[boton - 1] == 30)
      {
        segundos[boton - 1] = 60;
      }

      if (segundos[boton - 1] > 120)
      {
        segundos[boton - 1] = 0;
      }

      EncenderLed(boton, segundos[boton - 1]);

      txt = "L" + String(boton) + " " + String(segundos[boton - 1]) + "         ";
      tm1638.setDisplayToString(txt);
      segundos[boton - 1] += 30;
    }
  }

  //CheckLines();

  AbrirLineas();
  

}

void EncenderLed(int led, int color)
{
  if (color > 0)
  {
    tm1638.setLED(TM1638_COLOR_RED, led - 1);
  }
  else
  {
    tm1638.setLED(TM1638_COLOR_GREEN, led - 1);
  }
}

void AbrirLineas()
{

int ajustar = 20; //83;

sec_act++;

tm1638.setDisplayToString(String(sec_act / ajustar));
  
  for (int i = 0; i < 8; i++)
  {
    if(segundos[i] == sec_act / ajustar)
    {
     tm1638.setDisplayToString("________");
    }
  }

  if(sec_act / ajustar > 120)
  {
    tm1638.clearDisplay();
    sec_act = 1;
  }
}

boolean isButtonBeingPressed(int n) {
  // Button 1 status shown by bit0 of the byte buttons returned by module.getButtons()
  // Button 2 status shown by bit1 or the byte buttons ...
  // Button 3 status shown by bit2...etc

  // n - the number of the button to be tested) should be an integer from 1 to 8
  if (n < 1 or n > 8) return false;

  // Read in the value of getButtons from the TM1638 module.
  botones = tm1638.getButtons();

  // Which bit must we test for this button?
  int bitToLookAt = n - 1;

  // Read the value of the bit - either a 1 for button pressed, or 0 for not pressed.
  byte theValueOfTheBit = bitRead(botones, bitToLookAt);

  // If the button is pressed, return true, otherwise return false.
  if (theValueOfTheBit == 1)
    return true;
  else
    return false;
}

void CheckLines()
{
  apagar = 0;

  for (int i = 0; i < 8; i++)
  {
    if (segundos[i] > 30)
    {
      apagar++;
    }
  }

  if (apagar == 0)
  {
    tm1638.setDisplayToString("ALL OFF ");
  }
}

void disparar(int linea)
{
  digitalWrite(linea, HIGH);   // Enciende el pin
  delay(500);                   // Espera medio segundo
  digitalWrite(linea, LOW);   // Apaga el pin
  delay(500);                   // Espera medio segundo
}
