#include "PN532.h"

#define ADD 0x24

#define I2C i2c1

#define I2C_SCL 3
#define I2C_SDA 2

const uint8_t ACK[6] = 
{
    0x00,0x00,0xFF,0x00,0xFF,0x00,
};
const uint8_t NACK[6] = 
{
    0x00,0x00,0xFF,0xFF,0x00,0x00,
};
int16_t getReadLength(uint16_t timeOut)
{
    sleep_ms(1);//printf("\n長さの読み取り開始\n");
    
    uint8_t NACKBuf[6];
    for (int i = 0;i < 6;i ++)
    {
        NACKBuf[i] = 0;
    }
    uint16_t time = 0;

    do
    {
        if (i2c_read_blocking(I2C,ADD,NACKBuf,6,false) != -1)
        {
            if (NACKBuf[0] & 1)
            {
                break;
            }
        }
        sleep_ms(1);
        time ++;
        if (time > timeOut)
        {
            //printf("長さ読み取り:タイムアウト 多分つながってない\n");
            return -1;
        }
    } while (1);
    sleep_ms(1);/*printf("\n帰ってきたNACKはこれです\n");
    for (int i = 0;i < 6;i ++)
    {
        printf("0x%02x ",NACKBuf[i]);
    }
    printf("\n");*/
    i2c_write_blocking(I2C,ADD,NACK,6,false);
    return NACKBuf[4];
}
int8_t writeCommand(uint8_t *sendDat,uint8_t datLength)
{
    uint8_t sendBuf[64];
    sendBuf[0] = 0x00;
    sendBuf[1] = 0x00;
    sendBuf[2] = 0xFF;
    uint8_t length = datLength + 1;
    sendBuf[3] = length;
    sendBuf[4] = ~length + 1;
    sendBuf[5] = 0xD4;
    uint8_t sum = 0xD4;
    for (int i = 0;i < datLength;i ++)
    {
        sendBuf[i + 6]  = sendDat[i];
        sum += sendDat[i];
    }
    sendBuf[6 + datLength] = ~sum + 1;
    sendBuf[7 + datLength] = 0x00;
    sleep_ms(1);
    /*
    printf("===========\n送ったデータ:\n");
    for (int i = 0;i < 8 + datLength;i ++)
    {
        printf("0x%02x ",sendBuf[i]);
    }*/
    i2c_write_blocking(I2C,ADD,sendBuf,8 + datLength,false);
sleep_ms(1);
    uint8_t ACKBuf[7];
    for (int i = 0;i < 7;i ++)
    {
        ACKBuf[i] = 0;
    }
    uint16_t time = 0;
    do
    {
        if (i2c_read_blocking(I2C,ADD,ACKBuf,7,false) != -1)
        {
            if (ACKBuf[0] & 1)
            {
                break;
            }
        }
        sleep_ms(1);
        time ++;
        if (time > 5000)
        {
            //printf("データ書き込み:タイムアウト 多分つながってない\n");
            return -1;
        }
    } while (1);
    sleep_ms(1);
    /*
    printf("\nPN532の準備完了 所要時間は%dミリ秒\n",time);
    printf("帰ってきたACKはこれです\n");
    for (int i = 1;i < 7;i ++)
    {
        printf("0x%02x ",ACKBuf[i]);
    }
    printf("\n");*/
    if (ACKBuf[1] != 0x00 || ACKBuf[2] != 0x00 || ACKBuf[3] != 0xFF)
    {
        //printf("ACKが一致しません\n");
        return -1;
    }

    return 0;
}
int8_t readDat(uint8_t *readDat,uint16_t timeOut)
{
    uint8_t readBuf[64];
    for (int i = 0;i < 64;i ++)
    {
        readBuf[i] = 0;
    }
    uint16_t time = 0;
    int length = getReadLength(timeOut);
        if (length == -1)
    {
        return -1;
    }
    sleep_ms(1);
    //printf("length=%d",length);
    do
    {
        if (i2c_read_blocking(I2C,ADD,readBuf,6 + length + 2,false) != -1)
        {
            if (readBuf[0] & 1)
            {
                break;
            }
        }
        sleep_ms(1);
        time ++;
        if (time > timeOut)
        {
            //printf("データ読み込み:タイムアウト PN532からの応答なし\n");
            return -1;
        }
    } while (1);
    sleep_ms(1);//printf("バッファーの[1][2][3]はこの通りです。確認してください。%d %d %d\n",readBuf[1],readBuf[2],readBuf[3]);
    length = readBuf[4];
    //printf("全長:%d\n",readBuf[4]);
    for (int i = 8;i < 8+length;i ++)
    {
        //printf("%d番目 0x%02x\n",i,readBuf[i]);
        readDat[i - 8] = readBuf[i];
    }
    return 0;
}
uint32_t getVersion(void)
{
    uint8_t sendBuf[64];
    sendBuf[0] = 0x02;
    if (writeCommand(sendBuf,1) == -1)
    {
        //printf("ファームウェアバージョンの取得に失敗 PN532が認識されませんでした。");
        return 0;
    }
    
    uint8_t readBuf[64];

    readDat(readBuf,5000);
    uint32_t resp = 0;

    resp |= readBuf[0];
    resp <<= 8;
    resp |= readBuf[1];
    resp <<= 8;
    resp |= readBuf[2];
    resp <<= 8;
    resp |= readBuf[3];
    //printf("受信データ(10進数)=%d (16進数)=%x\n",resp,resp);
    return resp;
}
void setCardWait(uint8_t waitTime)
{
    uint8_t sendBuf[64];
    sendBuf[0] = 0x32;
    sendBuf[1] = 5;
    sendBuf[2] = 0xFF;
    sendBuf[3] = 0x01;
    sendBuf[4] = waitTime;
    writeCommand(sendBuf,5);
}
void SAMConfig()
{
    uint8_t sendBuf[64];
    sendBuf[0] = 0x14;
    sendBuf[1] = 0x01;
    sendBuf[2] = 0x14;
    sendBuf[3] = 0x01;
    writeCommand(sendBuf,4);
}
int8_t felicaRead(uint16_t systemCode,uint8_t requestCode,uint8_t cardID[8])
{
    uint8_t sendBuf[64];
    sendBuf[0] = 0x4A;
    sendBuf[1] = 1;
    sendBuf[2] = 1;
    sendBuf[3] = 0x00;
    sendBuf[4] = (systemCode >> 8) & 0xFF;
    sendBuf[5] = systemCode & 0xFF;
    sendBuf[6] = requestCode;
    sendBuf[7] = 0;
    if (writeCommand(sendBuf,8) == -1)
    {
        return -1;
    }
    uint8_t readBuf[64];
    if (readDat(readBuf,5000) == -1)
    {
        return -1;
    }
    for (int i = 0;i < 8;i ++)
    {
        cardID[i] =  readBuf[4 + i];
    }
    //printf("\nステータス:%d\n",readBuf[0]);
    return 0;
}
void PN532Init()
{
        
    i2c_init(I2C, 100000);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SCL);
    gpio_pull_up(I2C_SDA);

    sleep_ms(500);
    for (int i = 0;i < 9;i ++)
    {
        gpio_put(I2C_SCL,0);
        sleep_ms(5);
        gpio_put(I2C_SCL,1);
        sleep_ms(5);   
    }
    sleep_ms(500);
}