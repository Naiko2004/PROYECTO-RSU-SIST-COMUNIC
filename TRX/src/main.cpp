#include <Arduino.h>
#include "Config.h"
#include "DemodulatorFSK.h"
#include "ModulatorFSK.h"
// Toca unir los dos modulos y separarlos con una maquina de estados
// para ver si escucha o transmite. Hay que agregar el tema de pedir de vuelta
// tramas que fallen, poner bien los distintos modos (heartbeat, transmitir, emergencia)


// put function declarations here:
int myFunction(int, int);

void setup() {
  // put your setup code here, to run once:
  int result = myFunction(2, 3);
}

void loop() {
  // put your main code here, to run repeatedly:
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}