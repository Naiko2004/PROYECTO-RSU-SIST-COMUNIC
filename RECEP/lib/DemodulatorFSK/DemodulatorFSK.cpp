#include "DemodulatorFSK.h"
#include "Config.h"

// ==========================================
// VARIABLES VOLÁTILES PARA LA INTERRUPCIÓN
// ==========================================
hw_timer_t * adc_timer = NULL;

volatile uint8_t rx_buffer = 0;
volatile bool byte_ready = false;

// Variables del DSP (Procesamiento de Señal)
volatile int last_adc_value = 2048;
volatile int samples_since_cross = 0;
volatile int current_bit = 1; // 1 = Mark, 0 = Space

// Variables de recuperación de reloj (Digital PLL y UART)
volatile int sample_counter = 0;
volatile int bit_position = 0;
volatile uint8_t current_byte = 0;
volatile bool receiving = false;

// ==========================================
// RUTINA DE INTERRUPCIÓN (Se ejecuta 9600 veces por segundo)
// ==========================================
void IRAM_ATTR DemodulatorFSK::onADCInterrupt() {
    // 1. Leer ADC (0 a 4095)
    int adc_val = analogRead(ADC_PIN);
    
    // Filtro pasaaltos simple (elimina la continua o DC offset)
    // 2048 es nuestro cero virtual (1.65V)
    bool is_positive = (adc_val > 2048);
    bool was_positive = (last_adc_value > 2048);

    samples_since_cross++;

    // 2. DETECCIÓN DE CRUCE POR CERO
    if (is_positive != was_positive) {
        // Hubo un cruce. Medimos cuánto tiempo (muestras) pasó desde el anterior
        if (samples_since_cross > ZERO_CROSS_THRESHOLD) {
            current_bit = 1; // Baja frecuencia (1200 Hz -> Mark)
        } else {
            current_bit = 0; // Alta frecuencia (2200 Hz -> Space)
        }
        samples_since_cross = 0; // Reiniciar contador
    }
    last_adc_value = adc_val;

    // 3. RECUPERACIÓN DE DATOS (UART 8N1 por Software)
    // Estamos muestreando a 9600 Hz, por lo tanto hay 8 muestras por cada bit de 1200 bps.
    if (!receiving) {
        // Buscando el bit de Start (Space = 0)
        if (current_bit == 0) {
            receiving = true;
            sample_counter = 4; // Esperar media ventana (4 muestras) para leer en el centro del bit
            bit_position = 0;
            current_byte = 0;
        }
    } else {
        sample_counter--;
        if (sample_counter == 0) {
            sample_counter = 8; // Reiniciar para el centro del próximo bit
            
            if (bit_position == 0) {
                // Verificar que el bit de Start siga siendo 0 (Filtro de ruido)
                if (current_bit != 0) receiving = false; 
            } else if (bit_position >= 1 && bit_position <= 8) {
                // Leer bits de datos (LSB primero)
                if (current_bit == 1) {
                    current_byte |= (1 << (bit_position - 1));
                }
            } else if (bit_position == 9) {
                // Bit de Stop (Debe ser 1)
                if (current_bit == 1) {
                    rx_buffer = current_byte;
                    byte_ready = true;
                }
                receiving = false; // Fin de la trama, volvemos a buscar Start
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
    // Configurar resolución del ADC a 12 bits
    analogReadResolution(12);
    pinMode(ADC_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);

    // Iniciar el Timer a 9600 Hz
    adc_timer = timerBegin(1, 80, true); // Usamos el Timer 1 para no chocar con el modulador si están juntos
    timerAttachInterrupt(adc_timer, &DemodulatorFSK::onADCInterrupt, true);
    timerAlarmWrite(adc_timer, 1000000 / SAMPLE_RATE, true);
    timerAlarmEnable(adc_timer);
}

bool DemodulatorFSK::available() {
    return byte_ready;
}

uint8_t DemodulatorFSK::read() {
    byte_ready = false;
    return rx_buffer;
}