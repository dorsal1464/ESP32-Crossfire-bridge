#include <AlfredoCRSF.h>
#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

#include "defines.h"
#include "esp_wifi.h"
#include "rx.h"
#include "tx.h"

// #define WROOM_TX

#if defined(WROOM_TX)
#define PEER_MAC RX_ADDR
#else
#define PEER_MAC TX_ADDR
#endif

static int g_rssi = 0;

esp_now_peer_info_t g_peer_info = {
    .peer_addr = PEER_MAC,
    .lmk = ENC_KEY,
    .channel = 0,
    .encrypt = true,
};

uint8_t peer_addr[] = PEER_MAC;

static void readMacAddress() {
    Serial.print("[DEFAULT] ESP32 Board MAC Address: ");
    uint8_t baseMac[6];
    esp_err_t ret = esp_wifi_get_mac(WIFI_IF_STA, baseMac);
    if (ret == ESP_OK) {
        Serial.printf("%02x:%02x:%02x:%02x:%02x:%02x\n", baseMac[0], baseMac[1], baseMac[2], baseMac[3], baseMac[4], baseMac[5]);
    } else {
        Serial.println("Failed to read MAC address");
    }
}

static void OnWifiDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    Serial.print("Packet Send Status: ");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}

static void OnWifiDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
    rx_data_received(mac, incomingData, len);
    tx_data_received(mac, incomingData, len);
}

static void promiscuous_rx_cb(void *buf, wifi_promiscuous_pkt_type_t type) {
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
        rx_turn_on_led();
    }
}

void setup() {
    Serial.begin(BAUD_RATE);
    delay(1000);
    Serial.println("Starting...");
    WiFi.mode(WIFI_MODE_STA);
    
    ESP_ERROR_CHECK(esp_wifi_set_bandwidth(WIFI_IF_STA, WIFI_BW_HT20));
    ESP_ERROR_CHECK(esp_wifi_set_max_tx_power(80));
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
    ESP_ERROR_CHECK(
        esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N | WIFI_PROTOCOL_LR));
    ESP_ERROR_CHECK(esp_wifi_config_80211_tx_rate(WIFI_IF_STA, WIFI_PHY_RATE_LORA_250K));
    WiFi.begin();

    readMacAddress();
    delay(1000);

    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_promiscuous_rx_cb(promiscuous_rx_cb);
    // esp_now_register_send_cb(OnWifiDataSent);
    esp_now_register_recv_cb(OnWifiDataRecv);

    if (esp_now_add_peer(&g_peer_info) != ESP_OK) {
        Serial.println("Failed to add peer");
        return;
    }
    rx_setup();
    tx_setup();
}

void loop() {
    rx_loop();
    tx_loop();
    // 250Hz
    delay(1000 / 250);
}
