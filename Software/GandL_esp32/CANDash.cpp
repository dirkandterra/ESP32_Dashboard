#include "CANDash.h"
#include "Arduino.h"

CANData_t canData = {0};

void CANInit(void) {
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(CAN_TX_PIN, CAN_RX_PIN, TWAI_MODE_NORMAL);
    twai_timing_config_t  t_config = CAN_BAUD_CFG;
    twai_filter_config_t  f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    if (twai_driver_install(&g_config, &t_config, &f_config) != ESP_OK) {
        Serial.println("CAN: driver install failed");
        return;
    }
    if (twai_start() != ESP_OK) {
        Serial.println("CAN: start failed");
        return;
    }
    Serial.println("CAN: started");
}

void CANProcess(const ChanConfig_t *cfgs, uint8_t numCh) {
    twai_message_t msg;

    while (twai_receive(&msg, 0) == ESP_OK) {

#if CAN_SNIFF_MODE
        Serial.printf("CAN 0x%08X [%d]:", msg.identifier, msg.data_length_code);
        for (int i = 0; i < msg.data_length_code; i++) {
            Serial.printf(" %02X", msg.data[i]);
        }
        Serial.println();
#endif

        for (uint8_t ch = 0; ch < numCh; ch++) {
            const ChanConfig_t *cfg = &cfgs[ch];
            if (cfg->dataLen == 0) continue;

            uint32_t targetId = ((uint32_t)cfg->pgn[0] << 24) |
                                ((uint32_t)cfg->pgn[1] << 16) |
                                ((uint32_t)cfg->pgn[2] <<  8) |
                                 (uint32_t)cfg->source;
            uint32_t mask = ((cfg->pgn[0] != 0xFF ? 0xFFu : 0x00u) << 24) |
                            ((cfg->pgn[1] != 0xFF ? 0xFFu : 0x00u) << 16) |
                            ((cfg->pgn[2] != 0xFF ? 0xFFu : 0x00u) <<  8) |
                             (cfg->source != 0xFF ? 0xFFu : 0x00u);
            if ((msg.identifier & mask) != (targetId & mask)) continue;
            if ((uint8_t)(cfg->dataStart + cfg->dataLen) > msg.data_length_code) continue;

            uint32_t raw = 0;
            for (uint8_t b = 0; b < cfg->dataLen; b++) {
                raw |= (uint32_t)msg.data[cfg->dataStart + b] << (8 * b);
            }

            canData.ch[ch].val     = (float)raw * cfg->gain + cfg->offset;
            canData.ch[ch].updated = 1;
            canData.ch[ch].valid   = 1;
        }
    }
}
