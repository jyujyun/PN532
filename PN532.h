/*
PN532ライブラリ
PicoSDK向け

3shokudango
*/
#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <stdint.h>

/*
変更するべきゾーン
=================
*/
#define I2C i2c1

#define I2C_SCL 3
#define I2C_SDA 2
#define ADD 0x24
//===============

#ifdef __cplusplus
extern "C" {
#endif
#ifndef PN532_H
#define PN532_H
    uint32_t getVersion(void);
    void setCardWait(uint8_t waitTime);
    void SAMConfig();
    int8_t felicaRead(uint16_t systemCode,uint8_t requestCode,uint8_t cardID[8]);
    int8_t writeCommand(uint8_t *sendDat,uint8_t datLength);
    int8_t readDat(uint8_t *readDat,uint16_t timeOut);
    void PN532Init();
#endif
#ifdef __cplusplus
}
#endif