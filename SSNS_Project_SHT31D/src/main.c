#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <stdio.h>






int main(void)
{
	const struct device *const dev = DEVICE_DT_GET_ANY(sensirion_sht3xd);
	int rs;

	if (!device_is_ready(dev)) {
		printk("sensor: device not ready.\n");
		return 0;
	}
	while (1) {
		struct sensor_value temp, humidity;
		rs = sensor_sample_fetch(dev);
		if (rs < 0) {
			printk("sensor: failed to fetch sample (error: %d)\n", rs);
			return 0;
		}
		rs = sensor_channel_get(dev, SENSOR_CHAN_AMBIENT_TEMP, &temp);
		if (rs < 0) {
			printk("sensor: failed to get temperature (error: %d)\n", rs);
			return 0;
		}
		rs = sensor_channel_get(dev, SENSOR_CHAN_HUMIDITY, &humidity);
		if (rs < 0) {
			printk("sensor: failed to get humidity (error: %d)\n", rs);
			return 0;
		}
		printk("Temperature: %d.%06d °C, Humidity: %d.%06d %%RH\n",
			   temp.val1, temp.val2, humidity.val1, humidity.val2);
		k_sleep(K_SECONDS(1));
	}

}