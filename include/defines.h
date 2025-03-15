#pragma once

#define BAUD_RATE (115200)
#define TX_ADDR \
    { 0x78, 0x42, 0x1c, 0x68, 0x24, 0xb0 }
#define RX_ADDR \
    { 0x28, 0x37, 0x2f, 0x57, 0x71, 0xe0 }
#define ENC_KEY \
    { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 }

typedef struct {
    unsigned frame_ctrl : 16;
    unsigned duration_id : 16;
    uint8_t addr1[6]; /* receiver address */
    uint8_t addr2[6]; /* sender address */
    uint8_t addr3[6]; /* filtering address */
    unsigned sequence_ctrl : 16;
    uint8_t addr4[6]; /* optional */
} wifi_ieee80211_mac_hdr_t;

typedef struct {
    wifi_ieee80211_mac_hdr_t hdr;
    uint8_t payload[0]; /* network data ended with 4 bytes csum (CRC32) */
} wifi_ieee80211_packet_t;

typedef struct PACKED {
    crsf_sensor_gps_t gps;
    crsf_sensor_vario_t vario;
    crsf_sensor_baro_altitude_t baro;
    crsf_sensor_battery_t battery;
} telemetry_t;

typedef struct PACKED {
    crsf_channels_t channels;
} rc_data_t;