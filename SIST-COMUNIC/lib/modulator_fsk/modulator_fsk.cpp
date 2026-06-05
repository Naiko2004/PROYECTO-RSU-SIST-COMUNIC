#include "modulator_fsk.h"
#include "config.h"

// ==========================================
// VARIABLES GLOBALES OCULTAS (Para la Interrupción)
// ==========================================
volatile uint32_t phase_accumulator = 0;
volatile uint32_t phase_increment = 0;
uint32_t inc_mark;
uint32_t inc_space;
hw_timer_t * timer = NULL;

const uint8_t sine_table[256] = {
  128, 131, 134, 137, 140, 143, 146, 149, 152, 156, 159, 162, 165, 168, 171, 174,
  176, 179, 182, 185, 188, 191, 193, 196, 199, 201, 204, 206, 209, 211, 213, 216,
  218, 220, 222, 224, 226, 228, 230, 232, 234, 236, 237, 239, 240, 242, 243, 245,
  246, 247, 248, 249, 250, 251, 252, 253, 254, 254, 255, 255, 255, 255, 255, 255,
  255, 255, 255, 255, 255, 255, 254, 254, 253, 252, 251, 250, 249, 248, 247, 246,
  245, 243, 242, 240, 239, 237, 236, 234, 232, 230, 228, 226, 224, 222, 220, 218,
  216, 213, 211, 209, 206, 204, 201, 199, 196, 193, 191, 188, 185, 182, 179, 176,
  174, 171, 168, 165, 162, 159, 156, 152, 149, 146, 143, 140, 137, 134, 131, 128,
  124, 121, 118, 115, 112, 109, 106, 103, 100, 96,  93,  90,  87,  84,  81,  78,
  76,  73,  70,  67,  64,  61,  59,  56,  53,  51,  48,  46,  43,  41,  39,  36,
  34,  32,  30,  28,  26,  24,  22,  20,  18,  16,  15,  13,  12,  10,  9,   7,
  6,   5,   4,   3,   2,   1,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
  0,   0,   0,   0,   0,   0,   1,   1,   2,   3,   4,   5,   6,   7,   9,   10,
  12,  13,  15,  16,  18,  20,  22,  24,  26,  28,  30,  32,  34,  36,  39,  41,
  43,  46,  48,  51,  53,  56,  59,  61,  64,  67,  70,  73,  76,  78,  81,  84,
  87,  90,  93,  96,  100, 103, 106, 109, 112, 115, 118, 121, 124
};

// ==========================================
// INTERRUPCIÓN DE HARDWARE (Fuera de la clase)
// ==========================================
void IRAM_ATTR onTimer() {
    if (phase_increment > 0) {
        phase_accumulator += phase_increment;
        uint8_t index = phase_accumulator >> 24; 
        dacWrite(DAC_PIN, sine_table[index]);
    } else {
        dacWrite(DAC_PIN, 128); // Silencio
    }
}

// ==========================================
// IMPLEMENTACIÓN DE LA CLASE
// ==========================================
ModulatorFSK::ModulatorFSK() {}

void ModulatorFSK::begin() {
    pinMode(PTT_PIN, OUTPUT);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(PTT_PIN, LOW);

    // Matemáticas de DDS
    inc_mark = (uint32_t)(((uint64_t)FREQ_MARK * 4294967296ULL) / SAMPLE_RATE);
    inc_space = (uint32_t)(((uint64_t)FREQ_SPACE * 4294967296ULL) / SAMPLE_RATE);

    // Configuración del Timer del ESP32
    timer = timerBegin(0, 80, true);
    timerAttachInterrupt(timer, &onTimer, true);
    timerAlarmWrite(timer, 1000000 / SAMPLE_RATE, true);
    timerAlarmEnable(timer);
}

void ModulatorFSK::transmitBit(bool bitVal, uint64_t &next_time) {
    phase_increment = bitVal ? inc_mark : inc_space;
    while (esp_timer_get_time() < next_time) { /* yield */ }
    next_time += (1000000 / BAUD_RATE);
}

void ModulatorFSK::transmitByte(uint8_t data, uint64_t &next_time) {
    transmitBit(0, next_time); // Bit de inicio (Start)
    for (int i = 0; i < 8; i++) {
        transmitBit((data >> i) & 0x01, next_time); // LSB primero
    }
    transmitBit(1, next_time); // Bit de parada (Stop)
}

void ModulatorFSK::sendPacket(const uint8_t *payload, size_t length) {
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(PTT_PIN, HIGH); // Activar PTT
    delay(150); // Estabilizar RF

    uint64_t next_bit_time = esp_timer_get_time() + (1000000 / BAUD_RATE);

    // Preámbulo
    phase_increment = inc_mark;
    delay(50); 
    next_bit_time = esp_timer_get_time();

    // Sincronismo
    transmitByte(0x55, next_bit_time);
    transmitByte(0x55, next_bit_time);

    // Datos
    for (size_t i = 0; i < length; i++) {
        transmitByte(payload[i], next_bit_time);
    }

    // Fin
    phase_increment = 0; 
    delay(20); 
    digitalWrite(PTT_PIN, LOW); // Soltar PTT
    digitalWrite(LED_PIN, LOW);
}