
#ifndef GSM_MANAGER_H
#define GSM_MANAGER_H

#include <Arduino.h>

class GSMManager {
private:
    HardwareSerial* serial;

    String readResponse(unsigned long timeout = 2000) {
        String response = "";
        unsigned long start = millis();

        while (millis() - start < timeout) {
            while (serial->available()) {
                response += (char)serial->read();
            }
        }

        return response;
    }

    bool sendCommand(String command, String expected,
                     unsigned long timeout = 2000) {
        while (serial->available()) {
            serial->read();
        }

        serial->println(command);

        String response = readResponse(timeout);

        Serial.println(response);

        return response.indexOf(expected) != -1;
    }

public:
    GSMManager() {
        serial = &Serial1;
    }

    void begin(int rxPin = 26, int txPin = 27,
               int baud = 9600) {
        serial->begin(baud, SERIAL_8N1, rxPin, txPin);
    }

    bool testConnection() {
        return sendCommand("AT", "OK");
    }

    bool checkSIM() {
        return sendCommand("AT+CPIN?", "READY");
    }

    bool checkNetwork() {
        String response;

        while (serial->available()) {
            serial->read();
        }

        serial->println("AT+CREG?");

        response = readResponse(3000);

        Serial.println(response);

        return response.indexOf("+CREG: 0,1") != -1 ||
               response.indexOf("+CREG: 0,5") != -1;
    }

    int getSignalStrength() {
        while (serial->available()) {
            serial->read();
        }

        serial->println("AT+CSQ");

        String response = readResponse(2000);

        int index = response.indexOf("+CSQ:");

        if (index == -1) {
            return -1;
        }

        return response.substring(index + 5).toInt();
    }

    bool sendSMS(String phoneNumber, String message) {
        if (!sendCommand("AT+CMGF=1", "OK")) {
            return false;
        }

        while (serial->available()) {
            serial->read();
        }

        serial->print("AT+CMGS=\"");
        serial->print(phoneNumber);
        serial->println("\"");

        String response = readResponse(5000);

        if (response.indexOf(">") == -1) {
            return false;
        }

        serial->print(message);
        serial->write(26);  // CTRL+Z

        response = readResponse(15000);

        Serial.println(response);

        return response.indexOf("+CMGS:") != -1 &&
               response.indexOf("OK") != -1;
    }
};

#endif
