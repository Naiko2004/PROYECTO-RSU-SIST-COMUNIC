#pragma once

// ==========================================
// CONFIGURACIÓN DE PINES
// ==========================================
#define ADC_PIN      34   // Entrada de audio desde la radio VHF (Solo entrada)
#define SQUELCH_PIN  35   // (Opcional) Pin para leer si la radio abrió el silenciador
#define LED_PIN      2    // LED indicador de datos recibidos

// ==========================================
// PARÁMETROS DEL MÓDEM FSK
// ==========================================
#define BAUD_RATE    600
#define SAMPLE_RATE  19200 // Oversampling 8x (9600 Hz para 1200 bps)

// Umbrales para cruce por cero en 9600 Hz
// 1200 Hz (Mark)  = 8 muestras por ciclo (4 por semiciclo)
// 2200 Hz (Space) = ~4.3 muestras por ciclo (2 por semiciclo)
#define ZERO_CROSS_THRESHOLD 3 // Si el semiciclo dura más de 3 muestras, es Mark (1), sino Space (0)