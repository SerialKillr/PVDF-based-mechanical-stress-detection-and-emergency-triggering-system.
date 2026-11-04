
#include <TinyGPSPlus.h>
#include <HardwareSerial.h>

// ===================== USER CONFIG =====================
const char* CONTACTS[] = {
  "+91XXXXXXXXXX",          
};
const bool ENABLE_CALL        = false;   // also ring first contact after SMS
const uint32_t LONG_PRESS_MS  = 2000;    // hold SOS this long to trigger
const uint32_t CANCEL_WINDOW_MS = 5000;  // time to cancel after trigger
const uint32_t GPS_WAIT_MS    = 20000;   // max wait for a fresh fix
const uint32_t FIX_MAX_AGE_MS = 3000;    // fix older than this isn't "live"
const uint8_t  SMS_RETRIES    = 3;       // attempts per contact
const uint32_t COOLDOWN_MS    = 30000;   // ignore button after an alert
const uint32_t GPS_BAUD       = 9600;
const bool     PRINT_GPS      = true;    // print GPS status to Serial Monitor
const uint32_t GPS_PRINT_MS   = 1000;    // how often to print
const uint32_t MODEM_BAUD     = 9600;

// Pins
const int PIN_GPS_RX   = 16;  // ESP32 RX  <- GPS TX
const int PIN_GPS_TX   = 17;  // ESP32 TX  -> GPS RX
const int PIN_MODEM_RX = 18;  // ESP32 RX  <- SIM TXD
const int PIN_MODEM_TX = 19;  // ESP32 TX  -> SIM RXD
const int PIN_SOS      = 4;
const int PIN_CANCEL   = 5;
const int PIN_LED      = 2;
const int PIN_BUZZER   = 13;
// =======================================================

const size_t NUM_CONTACTS = sizeof(CONTACTS) / sizeof(CONTACTS[0]);

TinyGPSPlus gps;
HardwareSerial gpsSerial(2);
HardwareSerial modem(1);

bool     modemReady = false;
bool     haveLast   = false;
double   lastLat = 0, lastLng = 0;
uint32_t lastFixMs = 0;
uint32_t cooldownUntil = 0;

// ---------- helpers ----------
void pumpGPS() {
  while (gpsSerial.available()) gps.encode(gpsSerial.read());
  if (gps.location.isValid() && gps.location.age() < FIX_MAX_AGE_MS) {
    lastLat = gps.location.lat();
    lastLng = gps.location.lng();
    lastFixMs = millis();
    haveLast = true;
  }
}

bool liveFix() {
  return gps.location.isValid() && gps.location.age() < FIX_MAX_AGE_MS;
}

void beep(uint32_t ms) {
  digitalWrite(PIN_BUZZER, HIGH);
  digitalWrite(PIN_LED, HIGH);
  uint32_t t = millis();
  while (millis() - t < ms) pumpGPS();
  digitalWrite(PIN_BUZZER, LOW);
  digitalWrite(PIN_LED, LOW);
}

void pause(uint32_t ms) {
  uint32_t t = millis();
  while (millis() - t < ms) pumpGPS();
}

void beepPattern(int count, uint32_t on, uint32_t off) {
  for (int i = 0; i < count; i++) { beep(on); pause(off); }
}

// Read modem until `token` appears, "ERROR" appears, or timeout.
bool modemWait(const char* token, uint32_t timeout, String* out = nullptr) {
  String buf;
  uint32_t start = millis();
  while (millis() - start < timeout) {
    pumpGPS();
    while (modem.available()) {
      buf += (char)modem.read();
      if (buf.indexOf(token) >= 0) { if (out) *out = buf; return true; }
      if (buf.indexOf("ERROR") >= 0) { if (out) *out = buf; return false; }
    }
  }
  if (out) *out = buf;
  return false;
}

void modemFlush() { while (modem.available()) modem.read(); }

bool sendAT(const char* cmd, const char* expect = "OK", uint32_t timeout = 2000, String* out = nullptr) {
  modemFlush();
  modem.println(cmd);
  return modemWait(expect, timeout, out);
}

// ---------- modem ----------
bool modemInit() {
  modemReady = false;
  Serial.println("[modem] init...");

  bool alive = false;
  for (int i = 0; i < 10 && !alive; i++) alive = sendAT("AT", "OK", 1000);
  if (!alive) { Serial.println("[modem] no response to AT (power/wiring/baud?)"); return false; }

  sendAT("ATE0");                      // echo off
  if (!sendAT("AT+CPIN?", "READY", 3000)) { Serial.println("[modem] SIM not ready"); return false; }

  // wait for network registration (home=,1 roaming=,5)
  bool reg = false;
  uint32_t t = millis();
  while (millis() - t < 60000 && !reg) {
    String r;
    sendAT("AT+CREG?", "OK", 2000, &r);
    if (r.indexOf(",1") >= 0 || r.indexOf(",5") >= 0) reg = true;
    else pause(1000);
  }
  if (!reg) { Serial.println("[modem] not registered on network (2G coverage?)"); return false; }

  String csq; sendAT("AT+CSQ", "OK", 2000, &csq);
  Serial.print("[modem] signal: "); csq.trim(); Serial.println(csq);

  if (!sendAT("AT+CMGF=1")) return false;       // SMS text mode
  sendAT("AT+CSCS=\"GSM\"");
  modemReady = true;
  Serial.println("[modem] ready");
  return true;
}

bool sendSMS(const char* number, const String& text) {
  modemFlush();
  modem.print("AT+CMGS=\"");
  modem.print(number);
  modem.println("\"");
  if (!modemWait(">", 6000)) {
    modem.write((uint8_t)27);   // ESC to abort
    return false;
  }
  modem.print(text);
  modem.write((uint8_t)26);     // Ctrl-Z
  bool ok = modemWait("+CMGS:", 40000);
  pause(300);
  modemFlush();
  return ok;
}

void callNumber(const char* number) {
  modemFlush();
  modem.print("ATD"); modem.print(number); modem.println(";");
  pause(20000);                 // let it ring ~20 s
  modem.println("ATH");
  pause(500);
  modemFlush();
}

// ---------- message ----------
String mapsLink(double lat, double lng) {
  return String("https://maps.google.com/?q=") + String(lat, 6) + "," + String(lng, 6);
}

String buildMessage(bool isTest) {
  String m = isTest ? "TEST - " : "";
  m += "SOS! Help needed.\n";

  if (liveFix()) {
    m += mapsLink(gps.location.lat(), gps.location.lng());
    m += "\nLive GPS fix";
  } else if (haveLast) {
    uint32_t ageMin = (millis() - lastFixMs) / 60000UL;
    m += "NO LIVE GPS. Last known (";
    m += String(ageMin);
    m += " min ago):\n";
    m += mapsLink(lastLat, lastLng);
  } else {
    m += "NO GPS FIX - location unavailable.";
  }

  if (gps.time.isValid() && gps.date.isValid()) {
    char t[24];
    snprintf(t, sizeof(t), "\n%02d:%02d UTC", gps.time.hour(), gps.time.minute());
    m += t;
  }
  return m;   // kept short: SIM800L text mode limit is 160 chars
}

// ---------- alert flow ----------
bool cancelPressed() { return digitalRead(PIN_CANCEL) == LOW; }

void triggerAlert(bool isTest) {
  Serial.println(isTest ? "[alert] TEST triggered" : "[alert] SOS triggered");

  // 1. cancel window (fast beeps)
  Serial.println("[alert] cancel window - press cancel button to abort");
  uint32_t start = millis();
  while (millis() - start < CANCEL_WINDOW_MS) {
    if (cancelPressed()) {
      Serial.println("[alert] CANCELLED");
      beepPattern(2, 80, 80);
      return;
    }
    beep(100);
    pause(150);
  }

  // 2. confirm trigger, wait for fresh GPS
  beep(600);
  digitalWrite(PIN_LED, HIGH);
  Serial.println("[alert] waiting for GPS fix...");
  start = millis();
  while (!liveFix() && millis() - start < GPS_WAIT_MS) pumpGPS();
  Serial.println(liveFix() ? "[alert] got live fix" : "[alert] no live fix, sending fallback");

  String msg = buildMessage(isTest);
  Serial.println("[alert] message:\n" + msg);

  // 3. make sure modem is alive
  if (!sendAT("AT", "OK", 1000) || !modemReady) modemInit();

  // 4. send to every contact with retries
  size_t delivered = 0;
  for (size_t i = 0; i < NUM_CONTACTS; i++) {
    bool ok = false;
    for (uint8_t a = 1; a <= SMS_RETRIES && !ok; a++) {
      Serial.printf("[sms] %s attempt %u/%u\n", CONTACTS[i], a, SMS_RETRIES);
      ok = sendSMS(CONTACTS[i], msg);
      if (!ok) {
        Serial.println("[sms] failed");
        pause(3000);
        if (a == 2) modemInit();         // last-ditch re-init before final try
      }
    }
    if (ok) { delivered++; Serial.println("[sms] accepted by modem"); }
  }

  if (ENABLE_CALL && delivered > 0) callNumber(CONTACTS[0]);

  digitalWrite(PIN_LED, LOW);

  // 5. report honestly
  if (delivered == NUM_CONTACTS)      { beepPattern(3, 150, 150); Serial.println("[alert] SMS submitted to all contacts (delivery not confirmed)"); }
  else if (delivered > 0)             { beepPattern(3, 150, 150); beepPattern(2, 700, 200); Serial.println("[alert] PARTIAL send"); }
  else                                { beepPattern(5, 700, 200); Serial.println("[alert] FAILED - no SMS sent"); }
}

// ---------- GPS serial printout ----------
void printGPS() {
  static uint32_t last = 0;
  if (!PRINT_GPS || millis() - last < GPS_PRINT_MS) return;
  last = millis();

  if (gps.charsProcessed() < 10) {
    Serial.println("[gps] no data from GPS - check wiring (TX->GPIO16), power, baud");
    return;
  }

  uint32_t sats = gps.satellites.isValid() ? gps.satellites.value() : 0;

  if (liveFix()) {
    Serial.printf("[gps] FIX  lat=%.6f  lng=%.6f  alt=%.1fm  speed=%.1fkm/h  sats=%u  hdop=%.1f\n",
                  gps.location.lat(), gps.location.lng(),
                  gps.altitude.isValid() ? gps.altitude.meters() : 0.0,
                  gps.speed.isValid() ? gps.speed.kmph() : 0.0,
                  (unsigned)sats,
                  gps.hdop.isValid() ? gps.hdop.hdop() : 0.0);
    if (gps.time.isValid()) {
      Serial.printf("[gps] time %02d:%02d:%02d UTC\n", gps.time.hour(), gps.time.minute(), gps.time.second());
    }
    Serial.println("[gps] " + mapsLink(gps.location.lat(), gps.location.lng()));
  } else {
    Serial.printf("[gps] searching... sats=%u  (NMEA chars received: %lu)\n",
                  (unsigned)sats, (unsigned long)gps.charsProcessed());
  }
}

// ---------- setup / loop ----------
void setup() {
  Serial.begin(115200);
  pinMode(PIN_SOS, INPUT_PULLUP);
  pinMode(PIN_CANCEL, INPUT_PULLUP);
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);

  gpsSerial.begin(GPS_BAUD, SERIAL_8N1, PIN_GPS_RX, PIN_GPS_TX);
  modem.begin(MODEM_BAUD, SERIAL_8N1, PIN_MODEM_RX, PIN_MODEM_TX);

  Serial.println("\nSOS device booting");
  bool ok = modemInit();
  if (ok) beepPattern(2, 100, 100); else beepPattern(4, 400, 150);   // startup self-test result
  Serial.println("Hold SOS for 2s to trigger. Send 't' for a test alert.");
}

void loop() {
  pumpGPS();
  printGPS();

  // status LED: slow blink = GPS fix, fast blink = searching
  static uint32_t lastBlink = 0;
  uint32_t period = liveFix() ? 1000 : 250;
  if (millis() - lastBlink >= period) {
    lastBlink = millis();
    digitalWrite(PIN_LED, !digitalRead(PIN_LED));
  }

  // SOS long-press (re-arms only after release)
  static bool pressing = false, fired = false;
  static uint32_t pressStart = 0;
  if (digitalRead(PIN_SOS) == LOW) {
    if (!pressing) { pressing = true; pressStart = millis(); }
    else if (!fired && millis() - pressStart >= LONG_PRESS_MS && (int32_t)(millis() - cooldownUntil) >= 0) {
      fired = true;
      triggerAlert(false);
      cooldownUntil = millis() + COOLDOWN_MS;
    }
  } else {
    pressing = false;
    fired = false;
  }

  // Serial test command
  if (Serial.available()) {
    char c = Serial.read();
    if (c == 't' || c == 'T') triggerAlert(true);
  }
}
