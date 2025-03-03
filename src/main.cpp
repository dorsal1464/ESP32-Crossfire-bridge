#include <AlfredoCRSF.h>
#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

#include "defines.h"
#include "esp_wifi.h"


// #define WROOM_TX

#if defined(WROOM_TX)
#define PEER_MAC RX_ADDR
#else
#define PEER_MAC TX_ADDR
#endif

int g_rssi = 0;
int g_led_on = 0;

esp_now_peer_info_t g_peer_info = {
    .peer_addr = PEER_MAC,
    .lmk = ENC_KEY,
    .channel = 0,
    .encrypt = true,
};

uint8_t peer_addr[] = PEER_MAC;

#if defined(C3_RX)
#define PIN_RX 20
#define PIN_TX 21
#define LED_PIN 8

// Set up a new Serial object
HardwareSerial crsfHwSerial(1);
AlfredoCRSF crsf;
#endif

void readMacAddress() {
    Serial.print("[DEFAULT] ESP32 Board MAC Address: ");
    uint8_t baseMac[6];
    esp_err_t ret = esp_wifi_get_mac(WIFI_IF_STA, baseMac);
    if (ret == ESP_OK) {
        Serial.printf("%02x:%02x:%02x:%02x:%02x:%02x\n", baseMac[0], baseMac[1], baseMac[2], baseMac[3], baseMac[4], baseMac[5]);
    } else {
        Serial.println("Failed to read MAC address");
    }
}

void OnWifiDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    Serial.print("Packet Send Status: ");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}

void OnWifiDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) { Serial.printf("Got pkt: %s\n", incomingData); }

void promiscuous_rx_cb(void *buf, wifi_promiscuous_pkt_type_t type) {
    // All espnow traffic uses action frames which are a subtype of the mgmnt
    // frames so filter out everything else.
    if (type != WIFI_PKT_MGMT) return;

    static const uint8_t ACTION_SUBTYPE = 0xd0;
    static const uint8_t ESPRESSIF_OUI[] = {0x18, 0xfe, 0x34};

    const wifi_promiscuous_pkt_t *ppkt = (wifi_promiscuous_pkt_t *)buf;
    const wifi_ieee80211_packet_t *ipkt = (wifi_ieee80211_packet_t *)ppkt->payload;
    const wifi_ieee80211_mac_hdr_t *hdr = &ipkt->hdr;

    // Only continue processing if this is an action frame containing the
    // Espressif OUI.
    if ((ACTION_SUBTYPE == (hdr->frame_ctrl & 0xFF)) && (memcmp(ipkt->payload + 1, ESPRESSIF_OUI, 3) == 0)) {
        g_rssi = ppkt->rx_ctrl.rssi;
        Serial.printf("\tRSSI: %ddbm\n", g_rssi);
        digitalWrite(LED_PIN, HIGH);
        g_led_on = millis();
    }
}

void setupCsrfUart() {
#if defined(C3_RX)
    crsfHwSerial.begin(CRSF_BAUDRATE, SERIAL_8N1, PIN_RX, PIN_TX);
    if (!crsfHwSerial)
        while (1) Serial.println("Invalid crsfSerial configuration");

    crsf.begin(crsfHwSerial);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
#endif
}

void setup() {
    Serial.begin(BAUD_RATE);
    delay(1000);

    setupCsrfUart();

    WiFi.mode(WIFI_STA);
    WiFi.begin();

    readMacAddress();
    delay(1000);

    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_promiscuous_rx_cb(promiscuous_rx_cb);
    esp_now_register_send_cb(OnWifiDataSent);
    esp_now_register_recv_cb(OnWifiDataRecv);

    if (esp_now_add_peer(&g_peer_info) != ESP_OK) {
        Serial.println("Failed to add peer");
        return;
    }
}

void loop() {
#if defined(C3_RX)
    crsf.update();
    if (millis() - g_led_on > 50) {
        digitalWrite(LED_PIN, LOW);
    }
#endif
    uint8_t myData[] = "ALIVE!\x00";
    esp_err_t result = esp_now_send(g_peer_info.peer_addr, (uint8_t *)&myData, sizeof(myData));

    if (result == ESP_OK) {
    } else {
        Serial.println("Error sending the data");
    }
    delay(2000);
#if defined(C3_RX)
    crsf_channels_t pkt = {0};
    crsf.queuePacket(CRSF_ADDRESS_CRSF_TRANSMITTER, CRSF_FRAMETYPE_RC_CHANNELS_PACKED, &pkt, sizeof(pkt));
#endif
}
