#include <AlfredoCRSF.h>
#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

#include "defines.h"
#include "esp_wifi.h"
#include "rx.h"

#if defined(C3_RX)
// Set up a new Serial object
HardwareSerial crsfHwSerial(1);
AlfredoCRSF crsf;
static unsigned long g_led_on = 0;

static void setup_crsf_uart() {
    crsfHwSerial.begin(CRSF_BAUDRATE, SERIAL_8N1, PIN_RX, PIN_TX);
    if (!crsfHwSerial)
        while (1) Serial.println("Invalid crsfSerial configuration");

    crsf.begin(crsfHwSerial);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);
}

void rx_setup() {
    Serial.println("rx_setup!");
    setup_crsf_uart();
}

void rx_loop() {
    crsf.update();
    if (millis() - g_led_on > 50) {
        digitalWrite(LED_PIN, HIGH);
        g_led_on = millis();
    }
    crsf_channels_t pkt = {0};
    crsf.queuePacket(CRSF_ADDRESS_CRSF_TRANSMITTER, CRSF_FRAMETYPE_RC_CHANNELS_PACKED, &pkt, sizeof(pkt));
}

void rx_turn_on_led() {
    digitalWrite(LED_PIN, LOW);
    g_led_on = millis();
}
void rx_data_received(const uint8_t *mac, const uint8_t *incomingData, int len) { Serial.printf("Got pkt: %s\n", incomingData); }
#else
void rx_setup() {}
void rx_loop() {}
void rx_turn_on_led() {}
void rx_data_received(const uint8_t *mac, const uint8_t *incomingData, int len) {}
#endif