#pragma once

// ==========================================
// CONFIGURACIÓN DE PINES
// ==========================================
#define DAC_PIN      25   // Pin de salida de audio (DAC1)
#define PTT_PIN      4    // Pin para activar el PTT de la radio
#define LED_PIN      2    // LED de estado de la placa

// ==========================================
// PARÁMETROS DEL MÓDEM FSK
// ==========================================
#define FREQ_MARK    1200 // Frecuencia para el "1" lógico (Hz)
#define FREQ_SPACE   2200 // Frecuencia para el "0" lógico (Hz)
#define BAUD_RATE    600 // Velocidad en bits por segundo
#define SAMPLE_RATE  40000 // Frecuencia de muestreo del DDS (40 kHz)