
#ifndef GPS_MANAGER_H
#define GPS_MANAGER_H

#include <Arduino.h>
#include <TinyGPSPlus.h>

class GPSManager {
private:
    TinyGPSPlus gps;
    HardwareSerial* serial;

public:
    GPSManager() {
        serial = &Serial2;
    }

    void begin(int rxPin = 16, int txPin = 17, int baud = 9600) {
        serial->begin(baud, SERIAL_8N1, rxPin, txPin);
    }

    void update() {
        while (serial->available()) {
            gps.encode(serial->read());
        }
    }

    bool hasFix() {
        return gps.location.isValid() &&
               gps.location.age() < 5000;
    }

    double getLatitude() {
        return gps.location.lat();
    }

    double getLongitude() {
        return gps.location.lng();
    }

    int getSatellites() {
        if (gps.satellites.isValid()) {
            return gps.satellites.value();
        }
        return 0;
    }

    unsigned long getLocationAge() {
        return gps.location.age();
    }

    bool isConnected() {
        return gps.charsProcessed() > 0;
    }
};

#endif
