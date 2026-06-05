#include <Arduino.h>
#include "Config.h"
#include "ModulatorFSK.h"

ModulatorFSK modem;

// ==========================================
// FUNCIÓN DE SEGURIDAD: Cálculo de CRC-16 
// (Estándar CCITT-FALSE, muy usado en telemetría)
// ==========================================
uint16_t calculateCRC16(const uint8_t *data, uint16_t length) {
    uint16_t crc = 0xFFFF; // Valor inicial
    for (uint16_t i = 0; i < length; i++) {
        crc ^= (uint16_t)data[i] << 8;
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ 0x1021; // Polinomio
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

void setup() {
    Serial.begin(115200);
    modem.begin();
    Serial.println("Estación de Campo INA-CIRSA Inicializada");
}

void loop() {
    Serial.println("\n[+] Leyendo sensores y armando trama...");

    // ----------------------------------------------------
    // SIMULACIÓN DE LECTURA DE SENSORES
    // ----------------------------------------------------
    float sensor_nivel = 4.50;      // 4.50 metros
    float sensor_temp = 20.5;       // 20.5 °C
    uint8_t sensor_humedad = 75;    // 75%

    // ----------------------------------------------------
    // ARMADO DINÁMICO DEL PAQUETE
    // ----------------------------------------------------
    uint8_t payload[64]; // Buffer lo suficientemente grande
    int index = 0;

    // 1. CABECERA
    payload[index++] = 5;       // ID Estación: 5
    payload[index++] = 0x02;    // Tipo: Lapso de tiempo (Reporte periódico)
    
    // Máscara 0x0E (Binario: 0000 1110) -> Bits 1 (Nivel), 2 (Temp) y 3 (Humedad) encendidos
    uint8_t mask = 0x0E;        
    payload[index++] = mask;

    // 2. PAYLOAD (DATOS ESCALADOS)
    // El receptor espera Big Endian (Byte Alto primero, Byte Bajo después)

    // Nivel (Multiplicado por 100 para pasarlo a cm) -> 450
    uint16_t nivel_int = (uint16_t)(sensor_nivel * 100);
    payload[index++] = (nivel_int >> 8) & 0xFF; // MSB
    payload[index++] = nivel_int & 0xFF;        // LSB

    // Temperatura (Multiplicado por 10 para pasar a décimas) -> 205
    int16_t temp_int = (int16_t)(sensor_temp * 10);
    payload[index++] = (temp_int >> 8) & 0xFF; // MSB
    payload[index++] = temp_int & 0xFF;        // LSB

    // Humedad (1 solo byte directo)
    payload[index++] = sensor_humedad;

    // 3. CÁLCULO DE CRC (Sobre todos los bytes agregados hasta ahora)
    uint16_t crc = calculateCRC16(payload, index);
    
    // Adjuntar el CRC al final del paquete
    payload[index++] = (crc >> 8) & 0xFF; // CRC High
    payload[index++] = crc & 0xFF;        // CRC Low

    // ----------------------------------------------------
    // TRANSMISIÓN POR RADIO VHF
    // ----------------------------------------------------
    Serial.printf("Transmitiendo trama de %d bytes...\n", index);
    modem.sendPacket(payload, index);

    // Esperar 15 segundos antes del próximo envío de prueba
    delay(1000);
}