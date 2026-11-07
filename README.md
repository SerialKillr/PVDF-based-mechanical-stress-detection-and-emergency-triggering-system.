# PVDF Stress-Triggered GPS Emergency Alert System

A portable, ESP32-based safety device that detects significant mechanical stress using a **PVDF piezoelectric sensor**, then automatically sends an SMS containing the wearer's GPS location (as a Google Maps link) to predefined emergency contacts. A manual SOS button provides a backup trigger.

> **Status:** Work in progress. Hardware integration, signal conditioning, threshold calibration and event validation are still being tested.

---

## Table of Contents

- [How It Works](#how-it-works)
- [Features](#features)
- [Hardware](#hardware)
- [Pin Mapping](#pin-mapping)
- [System Architecture](#system-architecture)
- [Getting Started](#getting-started)
- [Calibration and Testing](#calibration-and-testing)
- [Power Design](#power-design)
- [Known Limitations](#known-limitations)
- [Roadmap](#roadmap)
- [License](#license)

---

## How It Works

PVDF (polyvinylidene fluoride) is a piezoelectric polymer with the repeating unit:

$$
(\mathrm{CH_2{-}CF_2})_n
$$

When it bends or is compressed, it generates a small electrical charge. This project conditions that signal, lets the ESP32 decide whether it represents a genuine high-stress event, and, if so, fires off an alert.

```
PVDF film → Signal conditioning → ESP32 ADC → Event validation
                                                    │
                      SOS button ───────────────────┤
                                                    ▼
                                  Latest GPS fix (NEO-6M, UART/NMEA)
                                                    ▼
                                  SMS via GSM modem (UART/AT commands)
                                                    ▼
                         Emergency contacts get a Google Maps link
```

**Event flow**

1. Mechanical deformation produces a charge/voltage response in the PVDF film.
2. A high-impedance conditioning stage scales and buffers the signal for the ESP32.
3. The ESP32 checks for threshold crossings and validates the event to reject noise.
4. The device retrieves the most recent valid GPS fix.
5. The GSM modem sends an SMS to every configured contact.
6. The LED and buzzer report the system state.

The SOS button bypasses the sensor and triggers the same alert path directly.

---

## Features

- PVDF-based mechanical stress detection with threshold triggering
- Event validation to reduce false alarms
- GPS positioning with Google Maps link in the SMS
- GSM SMS alerts to multiple predefined contacts
- Manual SOS button
- LED and buzzer status feedback
- Rechargeable single-cell Li-ion power
- Modular hardware and firmware design

---

## Hardware

| Component | Role |
|---|---|
| ESP32-WROOM-32 | Main controller |
| PVDF piezo film | Mechanical deformation sensor |
| Op-amp signal-conditioning circuit | Buffers and scales the PVDF output |
| NEO-6M GPS module | Location data (UART, NMEA) |
| SIM800L GSM module | SMS transmission (UART, AT commands) |
| 18650 Li-ion cell + charging module | Portable power |
| Push button | Manual SOS |
| LED + buzzer | Status indication |

*Components and conditioning circuitry may change as development continues.*

---

## Pin Mapping

*Suggested defaults. Update these to match your wiring.*

| Signal | ESP32 pin | Interface |
|---|---|---|
| PVDF conditioned output | GPIO 34 (ADC1) | Analog in |
| GPS TX → ESP32 RX | GPIO 16 | UART2 |
| GPS RX ← ESP32 TX | GPIO 17 | UART2 |
| GSM TX → ESP32 RX | GPIO 26 | UART1 (remapped) |
| GSM RX ← ESP32 TX | GPIO 25 | UART1 (remapped) |
| SOS button | GPIO 27 | Digital in (pull-up) |
| Status LED | GPIO 2 | Digital out |
| Buzzer | GPIO 4 | Digital out |

---

## System Architecture

### 1. Piezoelectric stress detection
PVDF has very high output impedance, so it needs a high-impedance buffer or charge amplifier before it reaches the ADC. The conditioned signal is biased to mid-rail so both positive and negative deformation can be read.

### 2. ESP32 processing
The ESP32 samples the sensor, filters noise, validates events, handles the SOS button, reads GPS data, drives the GSM modem and controls the indicators.

### 3. GPS positioning
The NEO-6M streams NMEA sentences over UART. The firmware parses them with TinyGPSPlus and keeps the latest valid fix along with its age.

### 4. GSM communication
The SIM800L is driven with AT commands. On a confirmed event the ESP32 sends:

```
EMERGENCY ALERT! Possible distress detected.
Location: https://maps.google.com/?q=LATITUDE,LONGITUDE
```

---

## Getting Started

**Requirements**

- Arduino IDE with the ESP32 board package
- [TinyGPSPlus](https://github.com/mikalhart/TinyGPSPlus) library
- An active SIM card on a network the GSM module supports

**Steps**

1. Wire the hardware according to the pin table above.
2. Open the firmware sketch in Arduino IDE and select **ESP32 Dev Module**.
3. Set your emergency contact numbers and detection threshold in the config section of the sketch.
4. Upload, then open the Serial Monitor at 115200 baud to confirm sensor readings, GPS fix and GSM registration.

---

## Calibration and Testing

- Log raw ADC values over serial while flexing the sensor in its final mounting.
- Set the threshold above normal walking and handling noise but below a genuine high-stress event.
- Test with the GSM SMS function disabled first, using only the buzzer and serial log, to count false triggers.
- Test GPS outdoors with clear sky before relying on location data.

---

## Power Design

- A single 18650 cell powers the system through a charging/protection module and suitable regulation.
- The GSM modem needs a dedicated supply path that handles transmit current peaks of up to about 2 A, with a large bulk capacitor close to the module.
- Energy from the PVDF film is used only as a **sensing signal**. It is not enough to power the ESP32 or GSM modem.

---

## Known Limitations

- PVDF output depends on deformation, film geometry, mounting and material properties.
- Vibration and electrical noise can cause false triggers.
- GPS may not get a fix indoors, and a cold start can take minutes.
- SMS delivery depends on network coverage and is not guaranteed or acknowledged.
- The SIM800L is a **2G-only** module, which many networks have shut down or are phasing out.
- A flat battery disables the device.
- Mechanical stress alone cannot identify every kind of emergency.

---

## Roadmap

- [ ] Improved PVDF signal conditioning (charge amplifier, input protection)
- [ ] Adaptive thresholding and better noise filtering
- [ ] Cancel window with buzzer countdown before sending
- [ ] Accelerometer-based fall detection
- [ ] Multiple PVDF sensors
- [ ] Battery monitoring and low-power / deep-sleep mode
- [ ] LTE module (replacing 2G GSM)
- [ ] Live location tracking
- [ ] Machine-learning event classification
- [ ] Custom PCB and 3D-printed enclosure

---

## License

Released under the [MIT License](LICENSE).
