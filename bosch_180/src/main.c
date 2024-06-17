#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/sys/printk.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/sensor.h>
#include <sensor/bmp180/bmp180.h>
#include <math.h>



LOG_MODULE_REGISTER(main);

static const struct device *i2c_dev;
static const struct device *bmp180_dev;


int main(void)
{
    int ret;
    struct sensor_value temp, press;
	struct sensor_value altitude = {0, 0};

    printk("BMP180 sensor example started.\n");
    // Initialize I2C device
    i2c_dev = DEVICE_DT_GET(DT_NODELABEL(i2c1));
    if (!i2c_dev) {
        printk("I2C: Device driver not found.\n");
        return 0;
    }
    
    // Initialize BMP180 sensor device
    bmp180_dev = DEVICE_DT_GET_ANY(bosch_bmp180);
    if (!bmp180_dev) {
        printk("BMP180: Device driver not found.\n");
        return 0;
    }

    // Set measurement accuracy mode if needed
    // (Assuming the driver supports a set_mode function, otherwise skip this part)
    // struct sensor_value mode;
    // mode.val1 = BMP180_MODE_HIGH_RESOLUTION;
    // ret = sensor_attr_set(bmp180_dev, SENSOR_CHAN_ALL, SENSOR_ATTR_CONFIGURATION, &mode);
    // if (ret) {
    //     printk("Failed to set BMP180 mode: %s\n", strerror(-ret));
    //     return 0;
    // }

    // Fetch a sample from the BMP180 sensor
	while (true) {
		// do {
			ret = bmp180_sample_fetch(bmp180_dev, SENSOR_CHAN_ALL);
			if (ret) {
				printk("Failed to fetch sample from BMP180: %s\n", strerror(-ret));
				return 0;
			}

			// Get pressure [Pa] and temperature [C] measurement
			ret = bmp180_channel_get(bmp180_dev, SENSOR_CHAN_AMBIENT_TEMP, &temp);
			if (ret) {
				printk("Failed to get temperature: %d\n", ret);
				return 0;
			}

			ret = bmp180_channel_get(bmp180_dev, SENSOR_CHAN_PRESS, &press);
			if (ret) {
				printk("Failed to get pressure: %d\n", ret);
				return 0;
			}

			// Calculate estimated altitude [m]
			altitude.val1 = 44330 * (1.0 - pow((press.val1 / 101325.0), 0.1903));
    		
			double temperature = temp.val1 + (temp.val2 / 1000000.0);
			double pressure = press.val1 + (press.val2 / 1000000.0);

			printk("Temperature: %.2f C, Pressure: %.2f Pa, Altitude: %d.%02d m\n",
				temperature,
				pressure,
				altitude.val1, altitude.val2);
		// } while (false);

		k_sleep(K_MSEC(1000));
	}
}