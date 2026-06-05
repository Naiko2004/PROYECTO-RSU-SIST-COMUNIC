#include "DemodulatorFSK.h"
#include "Config.h"
#include <driver/adc.h>

hw_timer_t * adc_timer = NULL;

volatile uint8_t rx_buffer = 0;
volatile bool byte_ready = false;

// ==========================================
// VARIABLES DSP MEJORADAS
// ==========================================
// Filtro EWMA para centrar la onda sin importar el voltaje
volatile int32_t dc_offset_sum = 2048 * 256; 
volatile int current_zero = 2048;

// Variables de Histéresis
volatile bool is_positive = true;
volatile bool was_positive = true;

volatile int samples_since_cross = 0;
volatile int current_bit = 1; // 1 = Mark (Reposo)

// Variables de recuperación de reloj UART
volatile int sample_counter = 0;
volatile int bit_position = 0;
volatile uint8_t current_byte = 0;
volatile bool receiving = false;

// ==========================================
// RUTINA DE INTERRUPCIÓN (9600 Hz)
// ==========================================
void IRAM_ATTR DemodulatorFSK::onADCInterrupt() {
    int adc_val = adc1_get_raw(ADC1_CHANNEL_6);

    // 1. HISTÉRESIS FIJA Y SENSIBLE (Ideal para cable directo)
    // El centro exacto es 2048. Usamos un margen de solo ±50 para no perder el tono alto.
    if (adc_val > 2100) {
        is_positive = true;
    } else if (adc_val < 1996) {
        is_positive = false;
    }

    samples_since_cross++;

    // 2. DETECCIÓN DE CRUCE POR CERO
    if (is_positive != was_positive) {
        // En 19200 Hz: 1200Hz cruza cada 8 muestras, 2200Hz cada 4.3 muestras. 
        if (samples_since_cross > ZERO_CROSS_THRESHOLD) {
            current_bit = 1; // 1200 Hz (Mark)
        } else {
            current_bit = 0; // 2200 Hz (Space)
        }
        samples_since_cross = 0; 
    }
    was_positive = is_positive;

    // 3. SILENCIADOR MÍNIMO (Evita leer basura si desconectan el cable)
    if (samples_since_cross > 20) {
        current_bit = 1; 
    }

    // 4. RECUPERACIÓN DE DATOS UART
    if (!receiving) {
        if (current_bit == 0) { // Detectamos el Bit de Start
            receiving = true;
            sample_counter = 8; // Esperar al centro del bit (Oversampling 16x)
            bit_position = 0;
            current_byte = 0;
        }
    } else {
        sample_counter--;
        if (sample_counter == 0) {
            sample_counter = 16; 
            
            if (bit_position == 0) {
                if (current_bit != 0) receiving = false; 
            } else if (bit_position >= 1 && bit_position <= 8) {
                if (current_bit == 1) {
                    current_byte |= (1 << (bit_position - 1));
                }
            } else if (bit_position == 9) {
                if (current_bit == 1) { 
                    rx_buffer = current_byte;
                    byte_ready = true;
                }
                receiving = false; 
            }
            bit_position++;
        }
    }
}

// ==========================================
// MÉTODOS DE LA CLASE
// ==========================================
DemodulatorFSK::DemodulatorFSK() {}

void DemodulatorFSK::begin() {
    pinMode(LED_PIN, OUTPUT);

    // Atenuación DB_11 para leer de 0 a 3.3V
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_11);

    adc_timer = timerBegin(1, 80, true);
    timerAttachInterrupt(adc_timer, &DemodulatorFSK::onADCInterrupt, true);
    timerAlarmWrite(adc_timer, 1000000 / SAMPLE_RATE, true);
    timerAlarmEnable(adc_timer);
}

bool DemodulatorFSK::available() { return byte_ready; }
uint8_t DemodulatorFSK::read() { byte_ready = false; return rx_buffer; }