#pragma once

// ==========================================
// CONFIGURACIÓN DE PINES
// ==========================================
#define DAC_PIN      25   // Pin de salida de audio (DAC1)
#define PTT_PIN      4    // Pin para activar el PTT de la radio
#define LED_PIN      2    // LED de estado de la placa
#define ADC_PIN      34   // Entrada de audio desde la radio VHF (Solo entrada)
#define SQUELCH_PIN  35   // (Opcional) Pin para leer si la radio abrió el silenciador

// ==========================================
// PARÁMETROS DEL MOD FSK
// ==========================================
#define FREQ_MARK    1200 // Frecuencia para el "1" lógico (Hz)
#define FREQ_SPACE   2200 // Frecuencia para el "0" lógico (Hz)
#define BAUD_RATE    600 // Velocidad en bits por segundo
#define SAMPLE_RATE  40000 // Frecuencia de muestreo del DDS (40 kHz)

// ==========================================
// PARÁMETROS DEL DEMOD FSK
// ==========================================
#define BAUD_RATE    600
#define SAMPLE_RATE  19200 // Oversampling 8x (9600 Hz para 1200 bps)

// Umbrales para cruce por cero en 9600 Hz
// 1200 Hz (Mark)  = 8 muestras por ciclo (4 por semiciclo)
// 2200 Hz (Space) = ~4.3 muestras por ciclo (2 por semiciclo)
#define ZERO_CROSS_THRESHOLD 6 // Si el semiciclo dura más de 3 muestras, es Mark (1), sino Space (0)