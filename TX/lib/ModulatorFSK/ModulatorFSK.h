#pragma once
#include <Arduino.h>

class ModulatorFSK {
public:
    // Constructor
    ModulatorFSK();

    // Inicializa hardware y timers
    void begin();

    // Envía una trama completa manejando el PTT automáticamente
    void sendPacket(const uint8_t *payload, size_t length);

private:
    // Métodos internos de transmisión
    void transmitByte(uint8_t data, uint64_t &next_time);
    void transmitBit(bool bitVal, uint64_t &next_time);
};