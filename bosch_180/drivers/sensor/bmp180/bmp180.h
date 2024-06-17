/** \mainpage
*
****************************************************************************
* Copyright (C) 2008 - 2014 Bosch Sensortec GmbH
*
* File : bmp180.h
*
* Date : 2014/12/12
*
* Revision : 2.2.1
*
* Usage: Sensor Driver for BMP180 sensor
*
****************************************************************************
*
* \section License
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*   Redistributions of source code must retain the above copyright
*   notice, this list of conditions and the following disclaimer.
*
*   Redistributions in binary form must reproduce the above copyright
*   notice, this list of conditions and the following disclaimer in the
*   documentation and/or other materials provided with the distribution.
*
*   Neither the name of the copyright holder nor the names of the
*   contributors may be used to endorse or promote products derived from
*   this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDER
* OR CONTRIBUTORS BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
* OR CONSEQUENTIAL DAMAGES(INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
* ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE
*
* The information provided is believed to be accurate and reliable.
* The copyright holder assumes no responsibility
* for the consequences of use
* of such information nor for any infringement of patents or
* other rights of third parties which may result from its use.
* No license is granted by implication or otherwise under any patent or
* patent rights of the copyright holder.
**************************************************************************/
 /** \file bmp180.h
    \brief Header file for all define constants and function prototypes
*/
#ifndef BMP180_H
#define BMP180_H

#ifdef __cplusplus
extern "C" {
#endif

#include <zephyr/kernel.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/sensor.h>

#define BMP180_I2C_ADDRESS 0x77
#define BMP180_DEFAULT_CHIP_ID 0x55

// BMP180 Registers
typedef enum
{
    BMP180_REGISTER_CAL_AC1_MSB = 0xAA,
    BMP180_REGISTER_CAL_AC1_LSB = 0xAB,
    BMP180_REGISTER_CAL_AC2_MSB = 0xAC,
    BMP180_REGISTER_CAL_AC2_LSB = 0xAD,
    BMP180_REGISTER_CAL_AC3_MSB = 0xAE,
    BMP180_REGISTER_CAL_AC3_LSB = 0xAF,
    BMP180_REGISTER_CAL_AC4_MSB = 0xB0,
    BMP180_REGISTER_CAL_AC4_LSB = 0xB1,
    BMP180_REGISTER_CAL_AC5_MSB = 0xB2,
    BMP180_REGISTER_CAL_AC5_LSB = 0xB3,
    BMP180_REGISTER_CAL_AC6_MSB = 0xB4,
    BMP180_REGISTER_CAL_AC6_LSB = 0xB5,
    BMP180_REGISTER_CAL_B1_MSB = 0xB6,
    BMP180_REGISTER_CAL_B1_LSB = 0xB7,
    BMP180_REGISTER_CAL_B2_MSB = 0xB8,
    BMP180_REGISTER_CAL_B2_LSB = 0xB9,
    BMP180_REGISTER_CAL_MB_MSB = 0xBA,
    BMP180_REGISTER_CAL_MB_LSB = 0xBB,
    BMP180_REGISTER_CAL_MC_MSB = 0xBC,
    BMP180_REGISTER_CAL_MC_LSB = 0xBD,
    BMP180_REGISTER_CAL_MD_MSB = 0xBE,
    BMP180_REGISTER_CAL_MD_LSB = 0xBF,
    BMP180_REGISTER_ID = 0xD0,
    BMP180_REGISTER_SOFT_RESET = 0xE0,
    BMP180_REGISTER_CTRL_MEAS = 0xF4,
    BMP180_REGISTER_OUT_MSB = 0xF6,
    BMP180_REGISTER_OUT_LSB = 0xF7,
    BMP180_REGISTER_OUT_XLSB = 0xF8
} bmp180_register_t;

#define BMP180_OUT_XLSB_ADC_OUT_XLSB_MASK 0xF8
#define BMP180_CTRL_MEAS_MEASUREMENT_CONTROL_MASK 0x1F
#define BMP180_CTRL_MEAS_SCO_MASK 0x20
#define BMP180_CTRL_MEAS_OSS_MASK 0xC0

// Control register commands
typedef enum
{
    BMP180_CMD_TEMP = 0x2E,
    BMP180_CMD_PRESS = 0x34,
} bmp180_ctrl_cmd_t;

// BMP180 communication modes
typedef enum
{
    BMP180_MODE_ULTRA_LOW_POWER = 0,
    BMP180_MODE_STANDARD = 1,
    BMP180_MODE_HIGH_RESOLUTION = 2,
    BMP180_MODE_ULTRA_HIGH_RESOLUTION = 3
} bmp180_mode_t;

// BMP180 calibration values
typedef struct
{
    int16_t ac1;
    int16_t ac2;
    int16_t ac3;
    uint16_t ac4;
    uint16_t ac5;
    uint16_t ac6;
    int16_t b1;
    int16_t b2;
    int16_t mb;
    int16_t mc;
    int16_t md;
} bmp180_cal_t;

// BMP180 driver instance data
typedef struct
{
    const struct device *i2c;
    bmp180_cal_t calibrations;
    int32_t temperature;
    int32_t pressure;
    bmp180_mode_t mode;
} bmp180_data_t;

int bmp180_init(const struct device *dev);
int bmp180_sample_fetch(const struct device *dev, enum sensor_channel chan);
int bmp180_channel_get(const struct device *dev, enum sensor_channel chan, struct sensor_value *val);

#ifdef __cplusplus
}
#endif

#endif // BMP180_H