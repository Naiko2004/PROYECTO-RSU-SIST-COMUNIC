#include <Arduino.h>
#include "Config.h"
#include "ModulatorFSK.h"

ModulatorFSK modem;

// ==========================================
// FUNCIÓN DE SEGURIDAD: Cálculo de CRC-16 
// (Estándar CCITT-FALSE)
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
    float sensor_nivel = (rand() % 1000) / 100.0;      // 4.50 metros
    float sensor_temp = (rand () % 400) / 10.0;       // 20.5 °C
    uint8_t sensor_humedad = random(0, 81);    // 75%
    


    // ----------------------------------------------------
    // ARMADO DINÁMICO DEL PAQUETE
    // ----------------------------------------------------
    uint8_t payload[64]; 
    int index = 0;

    // 1. CABECERA
    payload[index++] = 5;       // ID Estación: 5
    payload[index++] = 0x02;    // Tipo: Lapso de tiempo (Reporte periódico)
    
    // Máscara 0x0E (Binario: 0000 1110)
    uint8_t mask = 0x0E;        
    payload[index++] = mask;

    // 2. PAYLOAD (DATOS ESCALADOS)
    uint16_t nivel_int = (uint16_t)(sensor_nivel * 100);
    payload[index++] = (nivel_int >> 8) & 0xFF; // MSB
    payload[index++] = nivel_int & 0xFF;        // LSB

    int16_t temp_int = (int16_t)(sensor_temp * 10);
    payload[index++] = (temp_int >> 8) & 0xFF; // MSB
    payload[index++] = temp_int & 0xFF;        // LSB

    payload[index++] = sensor_humedad;

    // 3. CÁLCULO DE CRC (Solo sobre los datos útiles, sin preámbulos)
    uint16_t crc = calculateCRC16(payload, index);
    
    // Adjuntar el CRC al final del paquete
    payload[index++] = (crc >> 8) & 0xFF; // CRC High
    payload[index++] = crc & 0xFF;        // CRC Low

    // ----------------------------------------------------
    // TRANSMISIÓN POR RADIO VHF
    // ----------------------------------------------------
    // 'index' ahora contiene el largo de los datos + 2 bytes del CRC
    Serial.printf("Transmitiendo trama de %d bytes...\n", index);
    modem.sendPacket(payload, index);

    // Esperar antes del próximo envío
    delay(1000); // 1 segundo para pruebas
}