# PVDF-Based Mechanical Stress Detection and GPS Emergency Alert System

An ESP32-based emergency alert system integrating a PVDF piezoelectric sensor, GPS positioning, and GSM communication to detect significant mechanical stress and transmit location-based emergency notifications.

## Overview

This project aims to develop a portable emergency alert system capable of detecting significant mechanical deformation using a piezoelectric polymer and automatically initiating an emergency notification.

The system uses **Polyvinylidene Fluoride (PVDF)**, a piezoelectric polymer with the repeating chemical unit:

\[
(CH_2-CF_2)_n
\]

When subjected to mechanical deformation, PVDF generates an electrical charge. A signal-conditioning and detection circuit processes this electrical response and provides a trigger signal to the ESP32.

The ESP32 then obtains the latest available geographical coordinates from a GPS module and instructs a GSM modem to send an emergency SMS containing a Google Maps location link to predefined emergency contacts.

## Key Features

- PVDF-based mechanical stress detection
- Electrical signal detection and threshold-based triggering
- ESP32-based embedded processing
- GPS-based geographical positioning
- GSM-based emergency SMS notifications
- Google Maps location sharing
- Rechargeable lithium-ion battery power supply
- LED and buzzer status indicators
- Modular hardware and software architecture

## Hardware Components

| Component | Purpose |
|---|---|
| ESP32-WROOM-32 | Main microcontroller |
| PVDF piezoelectric polymer | Mechanical deformation sensing |
| Signal-conditioning circuit | Processes the PVDF electrical output |
| Current/voltage detection circuit | Monitors the generated electrical signal, depending on the sensing design |
| NEO-6M GPS module | Obtains geographical coordinates |
| SIM800L GSM module | Sends emergency SMS notifications |
| 18650 Li-ion battery | Portable power supply |
| Li-ion charging module | Battery charging |
| Push button | Manual SOS activation |
| LED | Visual status indication |
| Buzzer | Audio feedback |

*Component selection and signal-conditioning circuitry may change during development.*

## System Architecture

The system consists of four major subsystems:

### 1. Piezoelectric Stress Detection

The PVDF polymer undergoes mechanical deformation when external stress is applied.

Due to its piezoelectric properties, this deformation produces an electrical charge and corresponding voltage response.

The generated signal is processed by a suitable high-impedance signal-conditioning circuit. The ESP32 monitors the resulting signal and identifies significant threshold crossings.

### 2. ESP32 Processing

The ESP32 acts as the central controller.

Its responsibilities include:

- Monitoring the piezoelectric sensor signal.
- Detecting significant mechanical events.
- Filtering noise and rejecting insignificant signals.
- Handling manual SOS activation.
- Retrieving GPS coordinates.
- Controlling the GSM modem.
- Managing system status indicators.

### 3. GPS Positioning

The NEO-6M GPS module receives satellite signals and provides geographical coordinates through UART communication using NMEA data.

The ESP32 retrieves the latest valid location whenever an emergency event is detected.

### 4. GSM Emergency Communication

The SIM800L modem communicates with the ESP32 through UART using AT commands.

When an emergency event is confirmed, the ESP32 sends an SMS containing the emergency notification and geographical coordinates.

Example location link:

`https://maps.google.com/?q=LATITUDE,LONGITUDE`

## Working Principle

1. The PVDF piezoelectric polymer is subjected to mechanical deformation.
2. Mechanical stress produces an electrical response in the polymer.
3. The signal-conditioning circuit processes the generated electrical signal.
4. The ESP32 detects whether the signal exceeds a predefined threshold.
5. The system applies event validation to reduce false triggering.
6. The ESP32 retrieves the latest valid GPS coordinates.
7. The GSM modem sends an emergency SMS to predefined contacts.
8. The LED and buzzer indicate the system response.

A manual SOS button can also activate the emergency notification independently of the piezoelectric sensor.

## Communication Interfaces

| Connection | Interface |
|---|---|
| PVDF detection circuit → ESP32 | ADC / GPIO |
| GPS → ESP32 | UART / NMEA |
| ESP32 → GSM | UART / AT Commands |
| SOS Button → ESP32 | GPIO |
| LED / Buzzer → ESP32 | GPIO |

## Software Requirements

- Arduino IDE
- ESP32 board package
- TinyGPSPlus library
- ESP32-WROOM development board
- Compatible SIM card and cellular network

## Power Management

The system uses a rechargeable single-cell lithium-ion battery.

The battery supplies the ESP32 and peripheral modules through appropriate voltage regulation.

The GSM modem requires a dedicated supply capable of handling its high transmission-current peaks.

The PVDF-generated electrical energy is treated as a sensing signal or potential harvested energy. It is not assumed to power the ESP32 or GSM modem directly.

## Limitations

- PVDF output depends on mechanical deformation, sensor geometry, mounting, and material properties.
- Electrical noise and environmental vibrations can cause false triggers.
- GPS positioning may be unavailable indoors.
- SMS transmission depends on cellular network availability.
- SIM800L requires a compatible 2G GSM network.
- Battery depletion can disable the system.
- SMS submission does not guarantee successful delivery or recipient acknowledgement.
- Mechanical stress detection alone cannot reliably identify every type of emergency.

## Future Improvements

- Improved PVDF signal-conditioning circuitry.
- Adaptive stress detection and noise filtering.
- Multiple PVDF sensors for improved event detection.
- Machine-learning-based mechanical event classification.
- Accelerometer-based fall detection.
- Battery monitoring and low-power operation.
- Live location tracking.
- Improved cellular connectivity using LTE modules.
- Compact custom PCB and 3D-printed enclosure.

## Project Status

Development in progress.

Hardware integration, signal conditioning, threshold calibration, and emergency event validation are subject to testing.

## License

MIT License.
