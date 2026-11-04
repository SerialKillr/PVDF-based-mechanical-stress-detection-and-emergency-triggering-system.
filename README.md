# Portable GPS-Based Emergency Alert System

An ESP32-based emergency alert device that uses GPS positioning and cellular communication to send the user's location to predefined emergency contacts through SMS.

## Overview

The system is designed to provide a simple way to request assistance during an emergency. When the user presses the SOS button, the ESP32 retrieves the latest available GPS coordinates and instructs a cellular modem to send an emergency SMS containing a Google Maps location link.

The device is designed to operate independently of a smartphone or Wi-Fi connection.

## Features

- One-button SOS activation
- GPS-based location tracking
- SMS emergency notifications
- Google Maps location link
- Rechargeable lithium-ion battery
- LED and buzzer status indicators
- Modular hardware architecture

## Hardware Components

| Component | Purpose |
|---|---|
| ESP32-WROOM-32 | Main microcontroller |
| NEO-6M GPS | Location acquisition |
| SIM800L GSM module | SMS communication |
| 18650 Li-ion battery | Portable power source |
| Li-ion charging module | Battery charging |
| Push button | Emergency activation |
| LED | Status indication |
| Buzzer | Audio feedback |

*Replace components in this list if your final hardware differs.*

## System Architecture

1. The GPS module receives satellite signals and calculates coordinates.
2. The ESP32 continuously reads GPS data over UART.
3. The user presses the SOS button.
4. The ESP32 retrieves the latest valid GPS location.
5. The cellular modem sends an SMS to configured emergency contacts.
6. The LED and buzzer indicate the operation status.

## Communication

| Interface | Communication |
|---|---|
| GPS → ESP32 | UART / NMEA |
| ESP32 → GSM | UART / AT Commands |
| SOS Button → ESP32 | GPIO |
| LED / Buzzer → ESP32 | GPIO |

## Software Requirements

- Arduino IDE
- ESP32 board package
- TinyGPSPlus library
- ESP32 development board
- Compatible SIM card and cellular network

## Setup

1. Clone this repository.
2. Open `firmware/main/main.ino` in Arduino IDE.
3. Install the required libraries.
4. Copy `config.example.h` to `config.h`.
5. Configure your emergency contact number and hardware settings.
6. Select the correct ESP32 board and COM port.
7. Upload the firmware.

## Power System

The system uses a single-cell rechargeable lithium-ion battery.

The battery supply must be regulated appropriately for the ESP32 and GPS module. The GSM modem requires a separate supply capable of handling its high current peaks.

## Limitations

- GPS positioning may be unavailable indoors.
- SMS delivery depends on cellular network availability.
- SIM800L requires a compatible 2G GSM network.
- Battery depletion can disable the device.
- SMS submission does not guarantee successful delivery or that the recipient has read the message.

## Project Status

Development in progress.

## Future Improvements

- Fall detection using an accelerometer
- Battery-level monitoring
- Live location tracking
- Low-power operating modes
- Improved cellular connectivity

## License

This project is licensed under the MIT License.
