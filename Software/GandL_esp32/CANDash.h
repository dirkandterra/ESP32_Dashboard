#ifndef CANDash_h
#define CANDash_h

#include "driver/twai.h"

#define CAN_TX_PIN   GPIO_NUM_5
#define CAN_RX_PIN   GPIO_NUM_4
#define CAN_BAUD_CFG TWAI_TIMING_CONFIG_250KBITS()

// Set to 1 to log every received frame over Serial for ID discovery.
#define CAN_SNIFF_MODE 1

#define NUM_CHANNELS 5

// Per-channel CAN config stored in EEPROM.
// pgn[0..2] + source form a big-endian 32-bit CAN ID:
//   targetId = (pgn[0]<<24)|(pgn[1]<<16)|(pgn[2]<<8)|source
// 11-bit example 0x1B0: pgn={0x00,0x00,0x01}, source=0xB0
// J1939 29-bit 0x18FEF100: pgn={0x18,0xFE,0xF1}, source=0x00
typedef struct {
    uint8_t  pgn[3];
    uint8_t  source;
    uint8_t  dataStart;   // byte index into CAN data[]
    uint8_t  dataLen;     // bytes to extract, 1-4, big-endian
    float    gain;
    float    offset;      // result = raw * gain + offset
} ChanConfig_t;

typedef struct {
    float   val;
    uint8_t updated;
    uint8_t valid;
} ChanData_t;

typedef struct {
    ChanData_t ch[NUM_CHANNELS];
} CANData_t;

extern CANData_t canData;

void CANInit(void);
void CANProcess(const ChanConfig_t *cfgs, uint8_t numCh);

#endif
