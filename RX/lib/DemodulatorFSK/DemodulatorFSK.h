#pragma once
#include <Arduino.h>

class DemodulatorFSK {
public:
    DemodulatorFSK();
    void begin();
    
    // Función para consultar si hay un byte nuevo disponible
    bool available();
    
    // Lee el byte decodificado
    uint8_t read();

private:
    static void IRAM_ATTR onADCInterrupt();
};