#include <Arduino.h>
#include "Config.h"
#include <ModulatorFSK.h>

// Instanciamos nuestro módem
ModulatorFSK modem;

void setup() {
    Serial.begin(115200);
    
    // Inicializar el modulador
    modem.begin();
    
    Serial.println("Estación INA-CIRSA Inicializada");
    Serial.println("Módulo de Transmisión FSK OK.");
}

void loop() {
    // ----------------------------------------------------
    // EJEMPLO: Armado de Trama "Señal de Vida"
    // ----------------------------------------------------
    // Trama definida: [ID, Tipo, Máscara, CRC_H, CRC_L]
    uint8_t payload[5];
    payload[0] = 42;    // ID Estación
    payload[1] = 0x03;  // Tipo: Señal de vida
    payload[2] = 0x00;  // Máscara: Sin sensores activos
    
    // Aquí en el futuro llamaremos a una función calcularCRC16()
    payload[3] = 0xA1;  // CRC_H (Simulado)
    payload[4] = 0x3F;  // CRC_L (Simulado)

    // Transmitir al aire
    Serial.println("Transmitiendo paquete de Heartbeat...");
    modem.sendPacket(payload, sizeof(payload));

    // Esperar 10 segundos antes de enviar otro
    delay(10000);
}