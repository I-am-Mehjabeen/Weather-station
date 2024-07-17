/*
****************************************************************************
* Copyright (C) 2008 - 2014 Bosch Sensortec GmbH
*
* bmp180.c
* Date: 2014/12/12
* Revision: 2.0.2 $
*
* Usage: Sensor Driver file for BMP180
*
****************************************************************************
* License:
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
#define DT_DRV_COMPAT bosch_bmp180


#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/kernel.h>
#include <zephyr/pm/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/__assert.h>
#include <zephyr/sys/byteorder.h>
#include "bmp180.h"


// LOG_MODULE_REGISTER(bmp180, LOG_LEVEL_INF);

// struct bmp180_cal_t {
//     int16_t ac1, ac2, ac3;
//     uint16_t ac4, ac5, ac6;
//     int16_t b1, b2;
//     int16_t mb, mc, md;
// };

// struct bmp180_data {
//     const struct device *i2c;
//     int32_t temperature;
//     int32_t pressure;
//     struct bmp180_cal_t calibrations;
//     uint8_t mode;
//     bool busy;
// };

// #define BMP180_I2C_ADDRESS 0x77
// #define BMP180_DEFAULT_CHIP_ID 0x55
// #define BMP180_REGISTER_ID 0xD0
// #define BMP180_REGISTER_CTRL_MEAS 0xF4
// #define BMP180_REGISTER_OUT_MSB 0xF6
// #define BMP180_REGISTER_CAL_AC1_MSB 0xAA
// #define BMP180_CMD_TEMP 0x2E
// #define BMP180_CMD_PRESS 0x34

LOG_MODULE_REGISTER(BMP180, CONFIG_SENSOR_LOG_LEVEL);

struct bmp180_data {
    const struct device *i2c;
    int16_t ac1, ac2, ac3, b1, b2, mb, mc, md;
    uint16_t ac4, ac5, ac6;
    int32_t b5;
    int32_t temperature;
    int32_t pressure;
};

static const struct sensor_driver_api bmp180_api_funcs = {
    .sample_fetch = bmp180_sample_fetch,
    .channel_get = bmp180_channel_get,
};

static struct bmp180_data bmp180_driver;

DEVICE_DT_INST_DEFINE(0, bmp180_init, device_pm_control_nop, &bmp180_driver, NULL, POST_KERNEL, CONFIG_SENSOR_INIT_PRIORITY, &bmp180_api_funcs);

static int bmp180_reg_read(const struct device *dev, uint8_t reg, int16_t *val)
{
    struct bmp180_data *data = dev->data;
    uint8_t reg_addr = reg;
    uint8_t tmp[2];
    int ret;

    ret = i2c_burst_read(data->i2c, DT_INST_REG_ADDR(0), reg_addr, tmp, 2);
    if (ret < 0) {
        return ret;
    }

    *val = (int16_t)sys_get_be16(tmp);

    return 0;
}

int bmp180_sample_fetch(const struct device *dev, enum sensor_channel chan)
{
    struct bmp180_data *data = dev->data;
    uint8_t reg_addr;
    uint8_t tmp[3];
    int ret;

    __ASSERT_NO_MSG(chan == SENSOR_CHAN_ALL || chan == SENSOR_CHAN_PRESS || chan == SENSOR_CHAN_AMBIENT_TEMP);

    /* Read calibration data */
    ret = bmp180_reg_read(dev, 0xAA, &data->ac1);
    if (ret < 0) {
        return ret;
    }
    /* Read other calibration values similarly */

    ret = bmp180_reg_read(dev, 0xAC, &data->ac2);
    if (ret < 0) {
        return ret;
    }

    ret = bmp180_reg_read(dev, 0xAE, &data->ac3);
    if (ret < 0) {
        return ret;
    }

    ret = bmp180_reg_read(dev, 0xB0, &data->ac4);
    if (ret < 0) {
        return ret;
    }

    ret = bmp180_reg_read(dev, 0xB2, &data->ac5);
    if (ret < 0) {
        return ret;
    }

    ret = bmp180_reg_read(dev, 0xB4, &data->ac6);
    if (ret < 0) {
        return ret;
    }

    ret = bmp180_reg_read(dev, 0xB6, &data->b1);
    if (ret < 0) {
        return ret;
    }

    ret = bmp180_reg_read(dev, 0xB8, &data->b2);
    if (ret < 0) {
        return ret;
    }

    ret = bmp180_reg_read(dev, 0xBa, &data->mb);
    if (ret < 0) {
        return ret;
    }

    ret = bmp180_reg_read(dev, 0xBC, &data->mc);
    if (ret < 0) {
        return ret;
    }

    ret = bmp180_reg_read(dev, 0xBE, &data->md);
    if (ret < 0) {
        return ret;
    }

    /* Trigger temperature measurement */
    reg_addr = 0xF4;
    tmp[0] = 0x2E;
    // ret = i2c_write(data->i2c, tmp, 1, DT_INST_REG_ADDR(0));
    ret = i2c_reg_write_byte(data->i2c, DT_INST_REG_ADDR(0), reg_addr, tmp[0]);
    if (ret < 0) {
        return ret;
    }

    k_msleep(5);

    /* Read raw temperature value */
    ret = i2c_burst_read(data->i2c, DT_INST_REG_ADDR(0), 0xF6, tmp, 2);

    if (ret < 0) {
        return ret;
    }

    int32_t ut = (int32_t)sys_get_be16(tmp);
    /* Trigger pressure measurement */
    tmp[0] = 0xF4;
    tmp[1] = 0x34; /* Ultra high resolution mode */
    ret = i2c_write(data->i2c, tmp, 2, DT_INST_REG_ADDR(0));
    // ret = i2c_reg_write_byte(data->i2c, DT_INST_REG_ADDR(0), tmp[0], tmp[1]);
    if (ret < 0) {
        return ret;
    }

    k_sleep(K_MSEC(26));

    /* Read raw pressure value */
    ret = i2c_burst_read(data->i2c, DT_INST_REG_ADDR(0), 0xF6, tmp, 3);

    if (ret < 0) {
        return ret;
    }

    int32_t up = (int32_t)(((uint32_t)tmp[0] << 16) | ((uint32_t)tmp[1] << 8) | (uint32_t)tmp[2]) >> 8;

    /* Calculate true temperature */
    int32_t x1 = (ut - data->ac6) * data->ac5 >> 15;
    int32_t x2 = ((int32_t)data->mc << 11) / (x1 + data->md);
    data->b5 = x1 + x2;
    data->temperature = (data->b5 + 8) >> 4;

    /* Calculate true pressure */
    int32_t b6 = data->b5 - 4000;
    x1 = (data->b2 * (b6 * b6 >> 12)) >> 11;
    x2 = data->ac2 * b6 >> 11;
    int32_t x3 = x1 + x2;
    int32_t b3 = ((((int32_t)data->ac1 * 4 + x3) << 3) + 2) >> 2;
    x1 = data->ac3 * b6 >> 13;
    x2 = (data->b1 * (b6 * b6 >> 12)) >> 16;
    x3 = ((x1 + x2) + 2) >> 2;
    uint32_t b4 = (data->ac4 * (uint32_t)(x3 + 32768)) >> 15;
    uint32_t b7 = ((uint32_t)up - b3) * (50000 >> 3);
    int32_t p;
    if (b7 < 0x80000000) {
        p = (b7 * 2) / b4;
    } else {
        p = (b7 / b4) * 2;
    }
    x1 = (p >> 8) * (p >> 8);
    x1 = (x1 * 3038) >> 16;
    x2 = (-7357 * p) >> 16;
    data->pressure = p + ((x1 + x2 + 3791) >> 4);

    return 0;
}

int bmp180_channel_get(const struct device *dev, enum sensor_channel chan, struct sensor_value *val)
{
    struct bmp180_data *data = dev->data;

    if (chan == SENSOR_CHAN_PRESS) {
        val->val1 = data->pressure / 100;
        val->val2 = (data->pressure % 100) * 10000;
    } else if (chan == SENSOR_CHAN_AMBIENT_TEMP) {
        val->val1 = data->temperature / 10;
        val->val2 = (data->temperature % 10) * 100000;
    } else {
        return -ENOTSUP;
    }

    return 0;
}

int bmp180_init(const struct device *dev)
{
    struct bmp180_data *data = dev->data;
    data->i2c = DEVICE_DT_GET(DT_NODELABEL(i2c1));
    if (!device_is_ready(data->i2c)) {
        LOG_ERR("I2C bus %s is not ready", data->i2c->name);
        printk("I2C bus %s is not ready", data->i2c->name);
        return -ENODEV;
    }

    return 0;
}