#include <Adafruit_NeoPixel.h>
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
static unsigned long g_last_stats = 0;
static const uint8_t peer_addr[] = TX_ADDR;
static Adafruit_NeoPixel led_pixel(1, LED_PIN, NEO_GRB + NEO_KHZ800);

static void turn_off_led() {
#if defined(SIMPLE_LED)
    digitalWrite(LED_PIN, HIGH);
#else
    led_pixel.setPixelColor(0, 0, 0, 0);
    led_pixel.show();
#endif
}

static void setup_crsf_uart() {
    crsfHwSerial.begin(CRSF_BAUDRATE, SERIAL_8N1, PIN_RX, PIN_TX);
    if (!crsfHwSerial)
        while (1) Serial.println("Invalid crsfSerial configuration");

    crsf.begin(crsfHwSerial);
}

void rx_setup() {
#if defined(SIMPLE_LED)
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);
#else
    led_pixel.begin();
#endif
    Serial.println("rx_setup!");
    setup_crsf_uart();
}

void rx_loop() {
    crsf.update();

    if (millis() - g_led_on > 1000 / 20) {
        turn_off_led();
        g_led_on = millis();
    }

    // crsf_channels_t pkt = {0};
    // crsf.queuePacket(CRSF_ADDRESS_CRSF_TRANSMITTER, CRSF_FRAMETYPE_RC_CHANNELS_PACKED, &pkt, sizeof(pkt));

    if (millis() - g_last_stats > 1000 / 10) {
        telemetry_t telemetry = {0};
        g_last_stats = millis();

        memcpy(&telemetry.gps, crsf.getGpsSensor(), sizeof(crsf_sensor_gps_t));
        memcpy(&telemetry.vario, crsf.getVarioSensor(), sizeof(crsf_sensor_vario_t));
        memcpy(&telemetry.baro, crsf.getBaroAltitudeSensor(), sizeof(crsf_sensor_baro_altitude_t));
        // memcpy(&telemetry.battery, crsf.getBatterySensor(), sizeof(crsf_sensor_battery_t));
        esp_now_send(peer_addr, (uint8_t *)&telemetry, sizeof(telemetry));
    }
}

void rx_turn_on_led() {
#if defined(SIMPLE_LED)
    digitalWrite(LED_PIN, LOW);
#else
    led_pixel.setPixelColor(0, 0, 255, 0);
    led_pixel.show();
#endif
    g_led_on = millis();
}
void rx_data_received(const uint8_t *mac, const uint8_t *incomingData, int len) {
    const rc_data_t *rc_data = (rc_data_t *)incomingData;
    Serial.printf("Got pkt: %s\n", incomingData);
    crsf.queuePacket(CRSF_ADDRESS_CRSF_TRANSMITTER, CRSF_FRAMETYPE_RC_CHANNELS_PACKED, &rc_data->channels, len);
}
#else
void rx_setup() {}
void rx_loop() {}
void rx_turn_on_led() {}
void rx_data_received(const uint8_t *mac, const uint8_t *incomingData, int len) {}
#endif