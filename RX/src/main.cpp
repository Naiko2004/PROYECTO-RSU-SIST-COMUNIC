#include <Arduino.h>
#include "Config.h"
#include "DemodulatorFSK.h"

DemodulatorFSK demod;

// ==========================================
// CÁLCULO DE CRC-16 (Estándar CCITT-FALSE)
// ==========================================
uint16_t calculateCRC16(const uint8_t *data, uint16_t length) {
    uint16_t crc = 0xFFFF; 
    for (uint16_t i = 0; i < length; i++) {
        crc ^= (uint16_t)data[i] << 8; 
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ 0x1021; 
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

// ==========================================
// MÁQUINA DE ESTADOS Y BUFFERS
// ==========================================
enum RxState {
    WAIT_SYNC_1,
    WAIT_SYNC_2,
    WAIT_SOF,
    WAIT_LENGTH,
    READ_PAYLOAD,
    READ_CRC
};

RxState rx_state = WAIT_SYNC_1;

// Usamos un buffer de 64 bytes para el payload como indicaste
const int MAX_PAYLOAD = 64;
uint8_t trama_completa[MAX_PAYLOAD]; 
int rx_index = 0;
int expected_payload_length = 0;

// Buffer temporal exclusivo para los 2 bytes del CRC
uint8_t crc_buffer[2];
int crc_index = 0;

// ==========================================
// FUNCIÓN AUXILIAR: Calcular tamaño del payload
// ==========================================
int calculatePayloadSize(uint8_t mask) {
    int size = 0;
    if (mask & 0x01) size += 2; 
    if (mask & 0x02) size += 2; 
    if (mask & 0x04) size += 2; 
    if (mask & 0x08) size += 1; 
    if (mask & 0x10) size += 2; 
    if (mask & 0x20) size += 2; 
    if (mask & 0x40) size += 1; 
    if (mask & 0x80) size += 1; 
    return size;
}

// ==========================================
// FUNCIÓN PRINCIPAL: Parsear e Imprimir Datos
// ==========================================
void parsePacket() {
    uint8_t current_id = trama_completa[0];
    uint8_t current_type = trama_completa[1];
    uint8_t current_mask = trama_completa[2];

    Serial.println("\n===================================");
    Serial.printf(" PAQUETE RECIBIDO - ESTACIÓN ID: %d\n", current_id);
    
    Serial.print("TIPO: ");
    switch(current_type) {
        case 0x01: Serial.println("Evento (Cambio de estado)"); break;
        case 0x02: Serial.println("Lapso de Tiempo (Reporte periódico)"); break;
        case 0x03: Serial.println("Señal de Vida (Heartbeat)"); break;
        case 0x04: Serial.println("ALERTA CRÍTICA "); break;
        default: Serial.println("Desconocido"); break;
    }

    Serial.printf("MÁSCARA: 0x%02X\n", current_mask);
    Serial.println("--- DATOS DE SENSORES ---");

    // El Payload de sensores empieza en el índice 3
    int offset = 3; 
    
    if (current_mask & 0x01) {
        uint16_t lluvia_raw = (trama_completa[offset] << 8) | trama_completa[offset+1];
        Serial.printf(" Pluviómetro: %.1f mm\n", lluvia_raw / 10.0);
        offset += 2;
    }
    if (current_mask & 0x02) {
        uint16_t nivel_raw = (trama_completa[offset] << 8) | trama_completa[offset+1];
        Serial.printf(" Nivel de Río: %.2f m (%d cm)\n", nivel_raw / 100.0, nivel_raw);
        offset += 2;
    }
    if (current_mask & 0x04) {
        int16_t temp_raw = (trama_completa[offset] << 8) | trama_completa[offset+1];
        Serial.printf(" Temperatura: %.1f °C\n", temp_raw / 10.0);
        offset += 2;
    }
    if (current_mask & 0x08) {
        uint8_t humedad = trama_completa[offset];
        Serial.printf(" Humedad: %d %%\n", humedad);
        offset += 1;
    }
    if (current_mask & 0x10) {
        uint16_t rad = (trama_completa[offset] << 8) | trama_completa[offset+1];
        Serial.printf(" Rad. Solar: %d W/m2\n", rad);
        offset += 2;
    }
    if (current_mask & 0x20) {
        uint16_t presion = (trama_completa[offset] << 8) | trama_completa[offset+1];
        Serial.printf(" Presión: %d hPa\n", presion);
        offset += 2;
    }
    if (current_mask & 0x40) {
        uint8_t vel_viento = trama_completa[offset];
        Serial.printf(" Vel. Viento: %d km/h\n", vel_viento);
        offset += 1;
    }
    if (current_mask & 0x80) {
        uint8_t dir_viento = trama_completa[offset];
        Serial.printf(" Dir. Viento: %d°\n", (int)(dir_viento * 1.5)); 
        offset += 1;
    }

    if (offset == 3) {
        Serial.println("Ningún dato de sensor adjunto.");
    }
    Serial.println("===================================\n");
}

void setup() {
    Serial.begin(115200);
    demod.begin();
    
    Serial.println("Central INA-CIRSA Inicializada");
    Serial.println("Máquina de Estados Robusta lista. Escuchando canal VHF...");
}

void loop() {
    if (demod.available()) {
        uint8_t b = demod.read();
        digitalWrite(LED_PIN, HIGH);
        
        // Imprime el byte puro que entra para debug
        // Serial.printf("%02X ", b); 

        switch (rx_state) {
            
            case WAIT_SYNC_1:
                if (b == 0x55) rx_state = WAIT_SYNC_2;
                break;

            case WAIT_SYNC_2:
                if (b == 0x55) rx_state = WAIT_SOF;
                else rx_state = WAIT_SYNC_1; 
                break;

            case WAIT_SOF:
                if (b == 0x7E) {
                    // ¡Encontramos el Start of Frame!
                    rx_state = WAIT_LENGTH;
                } else if (b == 0x55) {
                    // Si hay ruido y entran tres "0x55" seguidos, no rompemos la máquina
                    rx_state = WAIT_SOF; 
                } else {
                    rx_state = WAIT_SYNC_1;
                }
                break;

            case WAIT_LENGTH:
                if (b > 0 && b <= MAX_PAYLOAD) {
                    expected_payload_length = b;
                    rx_index = 0; // Preparamos el buffer para los datos reales
                    rx_state = READ_PAYLOAD;
                } else {
                    // Longitud inválida (ruido). Abortar.
                    rx_state = WAIT_SYNC_1;
                }
                break;

            case READ_PAYLOAD:
                trama_completa[rx_index++] = b;
                
                // Si ya leímos la cantidad exacta de bytes declarados
                if (rx_index == expected_payload_length) {
                    rx_state = READ_CRC;
                    crc_index = 0; // Preparamos el buffer de CRC
                }
                break;

            case READ_CRC:
                crc_buffer[crc_index++] = b; 
                
                // Si ya recibimos los 2 bytes del CRC
                if (crc_index == 2) {
                    uint16_t crc_recibido = (crc_buffer[0] << 8) | crc_buffer[1];
                    uint16_t crc_calculado = calculateCRC16(trama_completa, expected_payload_length);
                    
                    if (crc_recibido == crc_calculado) {
                        Serial.println("\n✅ CRC OK: Trama intacta.");
                        parsePacket(); 
                    } else {
                        Serial.println("\n❌ ERROR: Trama corrupta (CRC Inválido). Descartada.");
                        Serial.printf("Esperado: 0x%04X | Recibido: 0x%04X\n", crc_calculado, crc_recibido);
                        
                        Serial.print("🔍 BYTES CRUDOS RECIBIDOS: ");
                        for(int i = 0; i < expected_payload_length; i++) {
                            Serial.printf("%02X ", trama_completa[i]);
                        }
                        Serial.printf("| CRC: %02X %02X\n", crc_buffer[0], crc_buffer[1]);
                    }
                    
                    // Fin del proceso, volvemos a buscar sincronismo para el próximo paquete
                    rx_state = WAIT_SYNC_1; 
                }
                break;
        }

        digitalWrite(LED_PIN, LOW);
    }
}