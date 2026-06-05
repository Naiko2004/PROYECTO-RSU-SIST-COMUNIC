#include <Arduino.h>
#include "Config.h"
#include "DemodulatorFSK.h"

DemodulatorFSK demod;

// Buffer para ir guardando la trama que entra
uint8_t buffer_recepcion[256];
int indice_rx = 0;

void setup() {
    Serial.begin(115200);
    demod.begin();
    
    Serial.println("Central INA-CIRSA Inicializada");
    Serial.println("Escuchando canal VHF...");
}

void loop() {
    // Si la interrupción DSP decodificó un byte válido...
    if (demod.available()) {
        uint8_t dato_entrante = demod.read();
        
        digitalWrite(LED_PIN, HIGH); // Parpadeo al recibir
        
        // Guardamos el byte
        buffer_recepcion[indice_rx] = dato_entrante;
        indice_rx++;
        
        // --- AQUÍ SE IMPLEMENTARÁ LA MÁQUINA DE ESTADOS DEL PROTOCOLO ---
        // Por ahora, solo lo imprimimos en hexadecimal
        Serial.printf("Byte Rx: %02X\n", dato_entrante);
        
        // Si detectamos el preámbulo o la bandera (ej. 0x55), 
        // reiniciamos el índice o validamos el paquete.
        // ...
        
        delay(5); // Pequeña espera para apagar el LED visualmente
        digitalWrite(LED_PIN, LOW);
    }
}