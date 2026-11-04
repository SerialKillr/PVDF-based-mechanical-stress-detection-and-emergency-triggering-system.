
#ifndef CONFIG_H
#define CONFIG_H

// ===============================
// ESP32 CONFIGURATION
// ===============================

#define ESP32_BOARD "ESP32-WROOM-32"

// ===============================
// GPS CONFIGURATION
// ===============================

#define GPS_RX_PIN 16
#define GPS_TX_PIN 17
#define GPS_BAUD 9600

// ===============================
// GSM CONFIGURATION
// ===============================

#define GSM_RX_PIN 26
#define GSM_TX_PIN 27
#define GSM_BAUD 9600

// ===============================
// SOS BUTTON
// ===============================

#define SOS_BUTTON_PIN 4

// ===============================
// STATUS INDICATORS
// ===============================

#define LED_PIN 2
#define BUZZER_PIN 13

// ===============================
// EMERGENCY CONTACTS
// ===============================

// Replace with your own number in config.h
#define EMERGENCY_CONTACT_1 "+91XXXXXXXXXX"

// ===============================
// TIMING CONFIGURATION
// ===============================

#define GPS_FIX_MAX_AGE 5000
#define SOS_DEBOUNCE_MS 50

#endif
