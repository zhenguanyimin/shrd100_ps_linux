/*****************************************************************************
 * Copyright (c) 2023
 * Skyfend Technology Co., Ltd
 *
 * All rights reserved. Any unauthorized disclosure or publication of the
 * confidential and proprietary information to any other party will constitute
 * an infringement of copyright laws.
 *
 * $Id$
 * Author  : JiaWei, jiawei@autel.com
 * Created : 25 September 2023
 * Purpose : Provide ADS1115 driver
 *
 * $Rev$
 * $URL$
 *
 * Notes:
 *
 *****************************************************************************
 */

#include "ads1115.hpp"

#include "eap_pub.h"
#include <unistd.h>

namespace hal
{
namespace driver
{

// NTC 表中第一个电阻值代表的温度
#define FIRST_NTC_TABLE_TEMP (-55)

#define VCC_3V3        (3.3F)
#define RESISTANCE_1    (4990)
#define RESISTANCE_2    (4990)

#define NTC_TABLE_LEN    (167) // 电阻值---温度对应关系长度

// 电压分辨率  FUL-SCALE RANGE(FSR) (2.048v)
#define VOLTAGE_RESOLUTION_62_5_UV    (62.5F)

// 单次转换开始
#define ADS1115_REG_CONFIG_OS_START                     (0x1U << 7)//设备单词转换开启 高字节的最高位
#define ADS1115_REG_CONFIG_OS_NULL                      (0x0U << 7)

//输入引脚选择和输入方式选择
#define ADS1115_REG_CONFIG_MUX_Diff_01                  (0x0U << 4)  // 差分输入0引脚和1引脚 (默认)
#define ADS1115_REG_CONFIG_MUX_Diff_03                  (0x1U << 4)  // 差分输入0引脚和3引脚
#define ADS1115_REG_CONFIG_MUX_Diff_13                  (0x2U << 4)  // 差分输入1引脚和3引脚
#define ADS1115_REG_CONFIG_MUX_Diff_23                  (0x3U << 4)  // 差分输入2引脚和3引脚
#define ADS1115_REG_CONFIG_MUX_SINGLE_0                 (0x4U << 4)  //单端输入 0
#define ADS1115_REG_CONFIG_MUX_SINGLE_1                 (0x5U << 4)  //单端输入 1
#define ADS1115_REG_CONFIG_MUX_SINGLE_2                 (0x6U << 4)  //单端输入 2
#define ADS1115_REG_CONFIG_MUX_SINGLE_3                 (0x7U << 4)  //单端输入 3

//量程选择
#define ADS1115_REG_CONFIG_PGA_6                        (0x0U << 1) // +/- 6.1144
#define ADS1115_REG_CONFIG_PGA_4                        (0x1U << 1) // +/- 4.096
#define ADS1115_REG_CONFIG_PGA_2                        (0x2U << 1) // +/- 2.048 (默认)
#define ADS1115_REG_CONFIG_PGA_1                        (0x3U << 1) // +/- 1.024
#define ADS1115_REG_CONFIG_PGA_05                       (0x4U << 1) // +/- 0.512
#define ADS1115_REG_CONFIG_PGA_02                       (0x5U << 1) // +/- 0.256

//运行方式
#define ADS1115_REG_CONFIG_MODE_SINGLE              (0x1U << 0)  //  单次  (默认）
#define ADS1115_REG_CONFIG_MODE_CONTIN              (0x0U << 0)  //连续转换

//转换速率
#define ADS1115_REG_CONFIG_DR_8                     (0x0U << 5)
#define ADS1115_REG_CONFIG_DR_16                    (0x1U << 5)
#define ADS1115_REG_CONFIG_DR_32                    (0x2U << 5)
#define ADS1115_REG_CONFIG_DR_64                    (0x3U << 5)
#define ADS1115_REG_CONFIG_DR_128                   (0x4U << 5)  // (默认）
#define ADS1115_REG_CONFIG_DR_250                   (0x5U << 5)
#define ADS1115_REG_CONFIG_DR_475                   (0x6U << 5)
#define ADS1115_REG_CONFIG_DR_860                   (0x7U << 5)

//比较器模式
#define ADS1115_REG_CONFIG_COMP_MODE_TRADITIONAL    (0x0U << 4)//默认
#define ADS1115_REG_CONFIG_COMP_MODE_WINDOW         (0x1U << 4)

//比较器输出有效电平
#define ADS1115_REG_CONFIG_COMP_POL_LOW         (0x0U << 3)//默认
#define ADS1115_REG_CONFIG_COMP_POL_HIG         (0x1U << 3)

#define ADS1115_REG_CONFIG_COMP_LAT_NONLATCH        (0x0U << 2) // default
#define ADS1115_REG_CONFIG_COMP_LAT_LATCH           (0x1U << 2)

#define ADS1115_REG_CONFIG_COMP_QUE_ONE         (0x0U << 0)
#define ADS1115_REG_CONFIG_COMP_QUE_TWO         (0x1U << 0)
#define ADS1115_REG_CONFIG_COMP_QUE_THR         (0x2U << 0)
#define ADS1115_REG_CONFIG_COMP_QUE_DIS         (0x3U << 0) // default


// 寄存器配置 AIN0 单端输入AIN0、满量程2.048V、单次转换、转换速率128
#define CONFIG_REG_AIN0_H     (ADS1115_REG_CONFIG_OS_START|\
                              ADS1115_REG_CONFIG_MUX_SINGLE_0|\
                             ADS1115_REG_CONFIG_PGA_2|\
                             ADS1115_REG_CONFIG_MODE_SINGLE)


// 寄存器配置 AIN1 单端输入AIN1、满量程2.048V、单次转换、转换速率128
#define CONFIG_REG_AIN1_H     (ADS1115_REG_CONFIG_OS_START|\
                              ADS1115_REG_CONFIG_MUX_SINGLE_1|\
                             ADS1115_REG_CONFIG_PGA_2|\
                             ADS1115_REG_CONFIG_MODE_SINGLE)

// 寄存器配置 AIN2 单端输入AIN2、满量程2.048V、单次转换、转换速率128
#define CONFIG_REG_AIN2_H     (ADS1115_REG_CONFIG_OS_START|\
                              ADS1115_REG_CONFIG_MUX_SINGLE_2|\
                             ADS1115_REG_CONFIG_PGA_2|\
                             ADS1115_REG_CONFIG_MODE_SINGLE)

// 寄存器配置 低8bit，所有channel都相同
#define CONFIG_REG_AIN_L    (ADS1115_REG_CONFIG_DR_128|\
                            ADS1115_REG_CONFIG_COMP_MODE_TRADITIONAL|\
                            ADS1115_REG_CONFIG_COMP_POL_LOW|\
                            ADS1115_REG_CONFIG_COMP_LAT_NONLATCH|\
                            ADS1115_REG_CONFIG_COMP_QUE_DIS)

/***************/
//配置寄存器说明

//config register
/*CRH[15:8](R/W)
   BIT      15      14      13      12      11      10      9       8
   NAME     OS      MUX2    MUX1    MUX0    PGA2    PGA1    PGA0    MODE
CRL[7:0] (R/W)
   BIT      7       6       5       4       3       2       1       0
   NAME    DR0     DR1     DR0   COM_MODE COM_POL COM_LAT COM_QUE1 COM_QUE0


   -----------------------------------------------------------------------------------
 * 15    | OS             |  运行状态会单词转换开始
 *       |                | 写时:
 *       |                | 0   : 无效
 *       |                | 1   : 开始单次转换处于掉电状态时
 *       |                | 读时:
 *       |                | 0   : 正在转换
 *       |                | 1   : 未执行转换
 * -----------------------------------------------------------------------------------
 * 14:12 | MUX [2:0]      | 输入复用多路配置
 *       |                | 000 : AINP = AIN0 and AINN = AIN1 (default)
 *       |                | 001 : AINP = AIN0 and AINN = AIN3
 *       |                | 010 : AINP = AIN1 and AINN = AIN3
 *       |                | 011 : AINP = AIN2 and AINN = AIN3
 *       |                | 100 : AINP = AIN0 and AINN = GND
 *       |                | 101 : AINP = AIN1 and AINN = GND
 *       |                | 110 : AINP = AIN2 and AINN = GND
 *       |                | 111 : AINP = AIN3 and AINN = GND
 * -----------------------------------------------------------------------------------
 * 11:9  | PGA [2:0]      | 可编程增益放大器配置(FSR  full scale range)
 *       |                | 000 : FSR = В±6.144 V
 *       |                | 001 : FSR = В±4.096 V
 *       |                | 010 : FSR = В±2.048 V (默认)
 *       |                | 011 : FSR = В±1.024 V
 *       |                | 100 : FSR = В±0.512 V
 *       |                | 101 : FSR = В±0.256 V
 *       |                | 110 : FSR = В±0.256 V
 *       |                | 111 : FSR = В±0.256 V
 * -----------------------------------------------------------------------------------
 * 8     | MODE           | 工作模式
 *       |                | 0   : 连续转换
 *       |                | 1   : 单词转换
 * -----------------------------------------------------------------------------------
 * 7:5   | DR [2:0]       | 采样频率
 *       |                | 000 : 8 SPS
 *       |                | 001 : 16 SPS
 *       |                | 010 : 32 SPS
 *       |                | 011 : 64 SPS
 *       |                | 100 : 128 SPS (默认)
 *       |                | 101 : 250 SPS
 *       |                | 110 : 475 SPS
 *       |                | 111 : 860 SPS
 * -----------------------------------------------------------------------------------
 * 4     | COMP_MODE      | 比较器模式
 *       |                | 0   : 传统比较器 (default)
 *       |                | 1   : 窗口比较器
 * -----------------------------------------------------------------------------------
 * 3     | COMP_POL       | Comparator polarity
 *       |                | 0   : 低电平有效 (default)
 *       |                | 1   : 高电平有效
 * -----------------------------------------------------------------------------------
 * 2     | COMP_LAT       | Latching comparator
 *       |                | 0   : 非锁存比较器. (default)
 *       |                | 1   : 锁存比较器.
 * -----------------------------------------------------------------------------------
 * 1:0   | COMP_QUE [1:0] | Comparator queue and disable
 *       |                | 00  : Assert after one conversion
 *       |                | 01  : Assert after two conversions
 *       |                | 10  : Assert after four conversions
 *       |                | 11  : 禁用比较器并将ALERT/RDY设置为高阻抗 (default)
 * -----------------------------------------------------------------------------------
*/

typedef enum ads1115RegisterAddr {
    ADS1115_CONVERSION_REGISTER = 0X00,
    ADS1115_CONFIG_REGISTER = 0X01,
    ADS1115_LO_THRESH_REGISTER = 0X02,
    ADS1115_HO_THRESH_REGISTER = 0X03,
} asd1115RegisterAddr_t;

//电阻值 对应的温度
uint32_t NTC_TABLE[NTC_TABLE_LEN] = {
    0x6F7A6, // -55
    0x68047, // -54
    0x60EA9, // -53
    0x5A35F, // -52
    0x53EE9, // -51
    0x4E181, // -50
    0x48B19, // -49
    0x43B6A, // -48
    0x3F207, // -47
    0x3AE72, // -46
    0x37026, // -45
    0x336A2, // -44
    0x30171, // -43
    0x2D027, // -42
    0x2A266, // -41
    0x277DD, // -40
    0x25046, // -39
    0x22B62, // -38
    0x208FC, // -37
    0x1E8E4, // -36
    0x1CAF0, // -35
    0x1AEF9, // -34
    0x194DD, // -33
    0x17C7A, // -32
    0x165B3, // -31
    0x1506B, // -30
    0x13C88, // -29
    0x129F2, // -28
    0x11891, // -27
    0x1084E, // -26
    0xF917,  // -25
    0xEAD9,  // -24
    0xDD80,  // -23
    0xD0FE,  // -22
    0xC544,  // -21
    0xBA42,  // -20
    0xAFEE,  // -19
    0xA63A,  // -18
    0x9D1C,  // -17
    0x9489,  // -16
    0x8C7A,  // -15
    0x84E4,  // -14
    0x7DC1,  // -13
    0x7709,  // -12
    0x70B5,  // -11
    0x6ABF,  // -10
    0x6522,  // -9
    0x5FD7,  // -8
    0x5ADA,  // -7
    0x5627,  // -6
    0x51B8,  // -5
    0x4D8A,  // -4
    0x4999,  // -3
    0x45E1,  // -2
    0x425F,  // -1
    0x3F0F,  // 0
    0x3BEF,  // 1
    0x38FC,  // 2
    0x3632,  // 3
    0x3391,  // 4
    0x3114,  // 5
    0x2EBA,  // 6
    0x2C81,  // 7
    0x2A67,  // 8
    0x286A,  // 9
    0x2687,  // 10
    0x24BF,  // 11
    0x230F,  // 12
    0x2175,  // 13
    0x1FF0,  // 14
    0x1E80,  // 15
    0x1D22,  // 16
    0x1BD7,  // 17
    0x1A9B,  // 18
    0x1970,  // 19
    0x1853,  // 20
    0x1745,  // 21
    0x1643,  // 22
    0x154F,  // 23
    0x1465,  // 24
    0x1388,  // 25
    0x12B4,  // 26
    0x11EB,  // 27
    0x112B,  // 28
    0x1074,  // 29
    0xFC5,   // 30
    0xF1E,   // 31
    0xE80,   // 32
    0xDE8,   // 33
    0xD57,   // 34
    0xCCD,   // 35
    0xC49,   // 36
    0xBCB,   // 37
    0xB52,   // 38
    0xADF,   // 39
    0xA71,   // 40
    0xA08,   // 41
    0x9A4,   // 42
    0x944,   // 43
    0x8E8,   // 44
    0x890,   // 45
    0x83C,   // 46
    0x7EC,   // 47
    0x79F,   // 48
    0x755,   // 49
    0x70F,   // 50
    0x6CB,   // 51
    0x68B,   // 52
    0x64D,   // 53
    0x611,   // 54
    0x5D9,   // 55
    0x5A2,   // 56
    0x56E,   // 57
    0x53C,   // 58
    0x50D,   // 59
    0x4DF,   // 60
    0x4B3,   // 61
    0x489,   // 62
    0x460,   // 63
    0x439,   // 64
    0x414,   // 65
    0x3F0,   // 66
    0x3CE,   // 67
    0x3AD,   // 68
    0x38D,   // 69
    0x36E,   // 70
    0x351,   // 71
    0x335,   // 72
    0x31A,   // 73
    0x300,   // 74
    0x2E7,   // 75
    0x2CE,   // 76
    0x2B7,   // 77
    0x2A1,   // 78
    0x28B,   // 79
    0x276,   // 80
    0x262,   // 81
    0x24F,   // 82
    0x23D,   // 83
    0x22B,   // 84
    0x219,   // 85
    0x209,   // 86
    0x1F9,   // 87
    0x1E9,   // 88
    0x1DA,   // 89
    0x1CC,   // 90
    0x1BE,   // 91
    0x1B0,   // 92
    0x1A3,   // 93
    0x196,   // 94
    0x18A,   // 95
    0x17F,   // 96
    0x173,   // 97
    0x168,   // 98
    0x15E,   // 99
    0x153,   // 100
    0x14A,   // 101
    0x140,   // 102
    0x137,   // 103
    0x12E,   // 104
    0x125,   // 105
    0x11D,   // 106
    0x115,   // 107
    0x10D,   // 108
    0x105,   // 109
    0xFE,      // 110
    0xF7,     // 111
};

/*
 * function:
 *             查表获取获取当前阻值的index号
 * paramer:
 *         table: NTC电阻数组
 *         table: 数组长度
 *         data:  需要查找的电阻
 * return:
 *         返回输入电阻值在表中的index号
 */
// Copy from RTOS
uint8_t LookUpNtcTable(const uint32_t* const table, uint8_t tableLen, uint32_t data)
{
    uint32_t begin, end, middle = 0;
    uint8_t i = 0;

    begin = 0;
    end = tableLen - 1;

    if (table == NULL)
    {
        return 0;
    }

    if (data > table[0])
    {
        return 0;
    }
    else if (data <= table[end])
    {
        return end;
    }

    while (begin < end)
    {
        i++;
        middle = (begin + end) / 2;
        if (data == table[middle])
        {
            break;
        }
        else if ((data < table[middle]) && (data > table[middle + 1]))
        {
            break;
        }

        if (data > table[middle])
        {
            end = middle;
        }
        else
        {
            begin = middle;
        }

        if (i > tableLen)
        {
            break;
        }
    }

    if(begin > end)
    {
        return 0;
    }

    return middle;
}


// example, hal::driver::Ads1115 temperMonitor("/dev/i2c-1", 0x49);

Ads1115::Ads1115(std::shared_ptr<I2CBus> ic2Bus,
                 const std::uint16_t slaveAddr,
                 const std::uint16_t bitMode,
                 const std::uint16_t maxRetry,
                 const std::uint16_t timeOut)
    : I2CDriver(ic2Bus, slaveAddr, bitMode, maxRetry, timeOut)
{
}

Ads1115::~Ads1115()
{
}

int Ads1115::read(const std::uint8_t regAddr, std::uint8_t* buf, const std::uint16_t bufLen)
{
    return i2c_read(regAddr, buf, bufLen);
}

int Ads1115::write(const std::uint8_t regAddr, const std::uint8_t* buf, const std::uint16_t bufLen)
{
    return i2c_write(regAddr, buf, bufLen);
}

std::int16_t Ads1115::getTemperature(const Ads1115Channel& channel)
{
    const auto converValue = readConversion(channel);
    const float resistValue = convertToResistance(converValue);
    auto resist = (std::uint32_t)resistValue;

    const auto tempIndex = (uint16_t)LookUpNtcTable(NTC_TABLE, NTC_TABLE_LEN, (std::uint32_t)resist);
    const auto temp = (int16_t)(tempIndex + FIRST_NTC_TABLE_TEMP);

    /*
    std::cout << "converValue = 0X" << std::hex << converValue 
            << ", resistValue = " << std::dec << resistValue
            << ", (uint32_t)resistValue = " << std::dec << (uint32_t)resistValue
            << ", resist = " << resist
            << ", tempIndex = " << tempIndex
            << ", temp = " << temp
            << std::endl;
    */
    return temp;
}

std::uint16_t Ads1115::readConversion(const Ads1115Channel& channel)
{
    uint16_t configData = 0;
    //std::uint8_t configReg[2] = {0};
    if (Ads1115Channel::ADS1115_CHANNEL_0 == channel)
    {
        configData = (CONFIG_REG_AIN_L << 8) | CONFIG_REG_AIN0_H;
    }
    else if(Ads1115Channel::ADS1115_CHANNEL_1 == channel) // Test
    {
        configData = (CONFIG_REG_AIN_L << 8) | CONFIG_REG_AIN1_H;
    }
    else if(Ads1115Channel::ADS1115_CHANNEL_2 == channel) // Test
    {
        configData = (CONFIG_REG_AIN_L << 8) | CONFIG_REG_AIN2_H;
    }

    // std::cout << std:: hex << "configData=0X" << configData << std:: dec << std::endl;

    auto ret = write(ADS1115_CONFIG_REGISTER, (uint8_t*)&configData, sizeof(configData));
    (void)ret;

    std::uint8_t readV[2] = {0, 0};
    ret = read(ADS1115_CONFIG_REGISTER, readV, 2);
    // std::cout << "chann: " << (std::uint16_t)channel << ", readV = " << std:: hex << (std::uint16_t)readV[0] << ", " << (std::uint16_t)readV[1] << std::endl;

    usleep(10000); // 128 SPS means each sample need 7.8125ms, sleep 10ms

    std::uint8_t readV1[2] = {0, 0};
    ret = read(ADS1115_CONFIG_REGISTER, readV1, 2);
    // std::cout << "chann: " << (std::uint16_t)channel << ", readV1 = " << std:: hex << (std::uint16_t)readV1[0] << ", " << (std::uint16_t)readV[1] << std::endl;

    std::uint8_t conversionReg[2] = {0, 0};
    ret = read(ADS1115_CONVERSION_REGISTER, conversionReg, 2);
    // std::cout << "chann: " << (std::uint16_t)channel << ", conversionReg = " << std:: hex << (std::uint16_t)conversionReg[0] << ", " << (std::uint16_t)conversionReg[1] << std::endl;

    std::uint16_t converValue = ((static_cast<std::uint16_t>(conversionReg[0]) << 8) | (conversionReg[1]));
    if ((converValue == 0x7FFF) || (converValue == 0x8FFF))
    {
        converValue = 1;  // TBD... ???
        std::cout << "###Error, converValue=0x" << std::hex << converValue << std::endl;
    }
    // std::cout << "converValue=0x" << std::hex << converValue << std::dec << std::endl;
    return converValue;
}

float Ads1115::convertToResistance(const std::uint16_t value)
{
    auto valtage_V = ((double)value * VOLTAGE_RESOLUTION_62_5_UV) / 1000000; // Convert uV to V
    auto resistance = (((double)VCC_3V3 - 2 *valtage_V) * RESISTANCE_1) / valtage_V; //Rntc = ((VDD - 2dValue) * R)/dValue
    resistance = abs(resistance);
    //std::cout << std::dec << "valtage_V: " << valtage_V
    //    << ", resistance: " << resistance << std::endl;
    return resistance;
}

}  // ~namespace driver
}  // ~namespace hal

