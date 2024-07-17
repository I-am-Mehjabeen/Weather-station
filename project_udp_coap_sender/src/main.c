#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/sys/printk.h>
#include <zephyr/logging/log.h>
#include <sensor/bmp180/bmp180.h>
#include <math.h>
#include <stdio.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/net/openthread.h>
#include <openthread/thread.h>
#include <openthread/udp.h>
#include <openthread/coap.h>
#include <sps30/sps30.h>>

#define SLEEP_TIME_MS 1000
#define DATA_SENDING_INTERVAL 30000
#define TEXTBUFFER_SIZE 30
#define BUTTON0_NODE DT_NODELABEL(button0) // DT_N_S_buttons_S_button_0
#define BUTTON1_NODE DT_NODELABEL(button1) // DT_N_S_buttons_S_button_0


LOG_MODULE_REGISTER(main);

static const struct device *i2c_dev;
static const struct device *bmp180_dev;
static const struct device *sht3xd_dev;
struct sensor_value temp_sht3xd, humidity, temp_bmp180, press;
struct sensor_value altitude = {0, 0};
struct sps30_measurement m;
typedef struct {
    float C_L;  // Lower concentration breakpoint
    float C_H;  // Upper concentration breakpoint
    int I_L;    // Lower AQI breakpoint
    int I_H;    // Upper AQI breakpoint
} AQIBreakpoint;

static const struct gpio_dt_spec button0_spec = GPIO_DT_SPEC_GET(BUTTON0_NODE, gpios);
static const struct gpio_dt_spec button1_spec = GPIO_DT_SPEC_GET(BUTTON1_NODE, gpios);
static struct gpio_callback button_cb;
static struct k_timer send_timer; // Kernel timer
static volatile bool function_running = false;
int err;

void button_pressed_cb(const struct device *gpiob, struct gpio_callback *cb,
                       gpio_port_pins_t pins);
                       
static void coap_send_data_request(struct k_work *work);
static void send_timer_callback(struct k_timer *timer_id);
static void coap_send_data_response_cb(void *p_context, otMessage *p_message,
                                       const otMessageInfo *p_message_info, otError result);
int calculate_aqi(float concentration, AQIBreakpoint breakpoints[], int num_breakpoints);


char myText[TEXTBUFFER_SIZE];
uint16_t myText_length = 0;
char sensors_data[150];

// US EPA breakpoints for PM2.5
AQIBreakpoint pm25_breakpoints[] = {
    {0.0, 12.0, 0, 50},
    {12.1, 35.4, 51, 100},
    {35.5, 55.4, 101, 150},
    {55.5, 150.4, 151, 200},
    {150.5, 250.4, 201, 300},
    {250.5, 350.4, 301, 400},
    {350.5, 500.4, 401, 500}
};

// US EPA breakpoints for PM10
AQIBreakpoint pm10_breakpoints[] = {
    {0.0, 54.0, 0, 50},
    {55.0, 154.0, 51, 100},
    {155.0, 254.0, 101, 150},
    {255.0, 354.0, 151, 200},
    {355.0, 424.0, 201, 300},
    {425.0, 504.0, 301, 400},
    {505.0, 604.0, 401, 500}
};


static void storedata_response_send(otMessage *p_request_message,
                                    const otMessageInfo *p_message_info);


void udpReceiveCb(void *aContext, otMessage *aMessage, const otMessageInfo *aMessageInfo)
{
    uint16_t payloadLength = otMessageGetLength(aMessage) - otMessageGetOffset(aMessage);
    char buf[payloadLength + 1];
    otMessageRead(aMessage, otMessageGetOffset(aMessage), buf, payloadLength);
    buf[payloadLength] = '\0';
    printk("Received: %s\n", buf);
}

static void storedata_response_send(otMessage *p_request_message,
                                    const otMessageInfo *p_message_info)
{
    otError error = OT_ERROR_NO_BUFS;
    otMessage *p_response;
    otInstance *p_instance = openthread_get_default_instance();
    p_response = otCoapNewMessage(p_instance, NULL);
    if (p_response == NULL)
    {
        printk("Failed to allocate message for CoAP Request\n");
        return;
    }
    do
    {
        error = otCoapMessageInitResponse(p_response, p_request_message,
                                          OT_COAP_TYPE_ACKNOWLEDGMENT,
                                          OT_COAP_CODE_CHANGED);
        if (error != OT_ERROR_NONE)
        {
            break;
        }
        error = otCoapSendResponse(p_instance, p_response, p_message_info);
    } while (false);
    if (error != OT_ERROR_NONE)
    {
        printk("Failed to send store data response: %d\n", error);
        otMessageFree(p_response);
    }
}
static void coap_send_data_request(struct k_work *work)
{
    otError error = OT_ERROR_NONE;
    otMessage *myMessage;
    otMessageInfo myMessageInfo;
    otInstance *myInstance = openthread_get_default_instance();
    const otMeshLocalPrefix *ml_prefix = otThreadGetMeshLocalPrefix(myInstance);
    uint8_t serverInterfaceID[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
    const char *serverIpAddr =  "fd5f:7a91:b399:1:daa0:38be:eb1f:f9a7";
    const char *myTemperatureJson = "{\"temperature\": 23.32}";

    do
    {
        int ret;

        // Fetch sample from BMP180 sensor
        ret = bmp180_sample_fetch(bmp180_dev, SENSOR_CHAN_ALL);
        if (ret) {
            printk("Failed to fetch sample from BMP180: %s\n", strerror(-ret));
            return 0;
        }

        // Get pressure [Pa] and temperature [C] measurement from BMP180
        ret = bmp180_channel_get(bmp180_dev, SENSOR_CHAN_AMBIENT_TEMP, &temp_bmp180);
        if (ret) {
            printk("Failed to get temperature from BMP180: %d\n", ret);
            return 0;
        }

        ret = bmp180_channel_get(bmp180_dev, SENSOR_CHAN_PRESS, &press);
        if (ret) {
            printk("Failed to get pressure from BMP180: %d\n", ret);
            return 0;
        }

        // Fetch sample from SHT3XD sensor
        ret = sensor_sample_fetch(sht3xd_dev);
        if (ret) {
            printk("Failed to fetch sample from SHT3XD: %d\n", ret);
            return 0;
        }

        // Get temperature [C] and humidity [%RH] measurement from SHT3XD
        ret = sensor_channel_get(sht3xd_dev, SENSOR_CHAN_AMBIENT_TEMP, &temp_sht3xd);
        if (ret) {
            printk("Failed to get temperature from SHT3XD: %d\n", ret);
            return 0;
        }

        ret = sensor_channel_get(sht3xd_dev, SENSOR_CHAN_HUMIDITY, &humidity);
        if (ret) {
            printk("Failed to get humidity from SHT3XD: %d\n", ret);
            return 0;
        }

        char serial_number[SPS30_MAX_SERIAL_LEN];
        ret = sps30_get_serial(serial_number);
        if (ret) {
            printf("error reading serial number\n");
        } 

        ret = sps30_start_measurement();
        if (ret < 0)
            printf("error starting measurement\n");
        
        sensirion_sleep_usec(SPS30_MEASUREMENT_DURATION_USEC); /* wait 1s */
        ret = sps30_read_measurement(&m);
        if (ret < 0) {
            printf("error reading measurement\n");
            return 0;
        }

           
        // Calculate estimated altitude [m] from BMP180
        altitude.val1 = 243.00 * (1.0 - pow((press.val1 / 101325.0), 0.1903));
        
        double pressure = (press.val1 + 98800) + (press.val2 / 1000000.0);
        double pressureInKiloPascals = pressure / 1000;
        double finalTemperature = (sensor_value_to_double(&temp_sht3xd) + sensor_value_to_double(&temp_bmp180)) / 2;

        int pm25Aqi = calculate_aqi(m.mc_2p5, pm25_breakpoints, sizeof(pm25_breakpoints) / sizeof(pm25_breakpoints[0]));
        int pm10Aqi = calculate_aqi(m.mc_10p0, pm10_breakpoints, sizeof(pm10_breakpoints) / sizeof(pm10_breakpoints[0]));
   
        // Determine overall AQI
        int overallAqi = (pm25Aqi > pm10Aqi) ? pm25Aqi : pm10Aqi;
        
        // printf("Overall AQI: %d\n", overall_aqi);
        myMessage = otCoapNewMessage(myInstance, NULL);
        if (myMessage == NULL)
        {
            printk("Failed to allocate message for CoAP Request\n");
            return;
        }
        otCoapMessageInit(myMessage, OT_COAP_TYPE_CONFIRMABLE, OT_COAP_CODE_PUT);
        error = otCoapMessageAppendUriPathOptions(myMessage, "storedata");
        if (error != OT_ERROR_NONE)
        {
            break;
        }
        error = otCoapMessageAppendContentFormatOption(myMessage,
                                                    OT_COAP_OPTION_CONTENT_FORMAT_JSON);
        if (error != OT_ERROR_NONE)
        {
            break;
        }
        error = otCoapMessageSetPayloadMarker(myMessage);
        if (error != OT_ERROR_NONE)
        {
            break;
        }
        

        snprintf(sensors_data, sizeof(sensors_data), 
            "{\"Pressure\": %.2f ,\"Humidity\": %.2f, \"Temperature\": %.2f, \"PM1_0\": %.2f, \"PM2_5\": %.2f, \"PM4\": %.2f, \"PM10\": %.2f, \"AQI\": %d}", 
            pressureInKiloPascals, 
            sensor_value_to_double(&humidity),
            finalTemperature,
            m.mc_1p0,
            m.mc_2p5, 
            m.mc_4p0, 
            m.mc_10p0,
            overallAqi
        );
        sensors_data[149] = '\0';
        printk("%s\n", sensors_data);

        error = otMessageAppend(myMessage, sensors_data,
                                strlen(sensors_data));
        if (error != OT_ERROR_NONE)
        {
            break;
        }
        memset(&myMessageInfo, 0, sizeof(myMessageInfo));
        memset(sensors_data, 0, sizeof(sensors_data));
        myMessageInfo.mPeerPort = OT_DEFAULT_COAP_PORT;

        error = otIp6AddressFromString(serverIpAddr, &myMessageInfo.mPeerAddr);
        if (error != OT_ERROR_NONE) { break; }

        error = otCoapSendRequest(myInstance, myMessage, &myMessageInfo,
                                coap_send_data_response_cb, NULL);


    } while (false);
    if (error != OT_ERROR_NONE)
    {
        printk("Failed to send CoAP Request: %d\n", error);
        otMessageFree(myMessage);
    }
    else
    {
        printk("CoAP data send.\n");
    }

}

K_WORK_DEFINE(sensor_work, coap_send_data_request);

static void coap_send_data_response_cb(void *p_context, otMessage *p_message,
                                       const otMessageInfo *p_message_info, otError result)
{
    if (result == OT_ERROR_NONE)
    {
        printk("Delivery confirmed.\n");
    }
    else
    {
        printk("Delivery not confirmed: %d\n", result);
    }
}

void coap_init(void)
{
    otInstance *p_instance = openthread_get_default_instance();
    otError error = otCoapStart(p_instance, OT_DEFAULT_COAP_PORT);
    if (error != OT_ERROR_NONE)
        printk("Failed to start Coap: %d\n", error);
}

void button_pressed_cb(const struct device *dev, struct gpio_callback *cb, uint32_t pins) {
    if (pins & BIT(button0_spec.pin)) {
        function_running = true;
        printk("Start sending data....\n");
        k_timer_start(&send_timer, K_NO_WAIT, K_MSEC(DATA_SENDING_INTERVAL));

    } else if (pins & BIT(button1_spec.pin)) {
        function_running = false;
        printk("Stop sending data....\n");
        k_timer_stop(&send_timer);
    }
}

void send_timer_callback(struct k_timer *timer_id)
{
    if (function_running) {
        k_work_submit(&sensor_work);
    }
}

//Function to calculate air quality index
int calculate_aqi(float concentration, AQIBreakpoint breakpoints[], int num_breakpoints) {
    for (int i = 0; i < num_breakpoints; i++) {
        if (concentration >= breakpoints[i].C_L && concentration <= breakpoints[i].C_H) {
            float aqi = ((breakpoints[i].I_H - breakpoints[i].I_L) / 
                         (breakpoints[i].C_H - breakpoints[i].C_L)) * 
                         (concentration - breakpoints[i].C_L) + 
                         breakpoints[i].I_L;
            return (int)(aqi + 0.5);  // Round to nearest integer
        }
    }
    return -1;
}

int main(void)
{
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

    // Initialize SHT3XD sensor device
    sht3xd_dev = DEVICE_DT_GET_ANY(sensirion_sht3xd);
    if (!sht3xd_dev) {
        printk("SHT3XD: Device driver not found.\n");
        return 0;
    }
    if (!device_is_ready(sht3xd_dev)) {
        printk("SHT3XD: Device not ready.\n");
        return 0;
    }

    /* Initialize I2C bus */
    sensirion_i2c_init();

    /* Busy loop for initialization, because the main loop does not work without
     * a sensor.
     */
    while (sps30_probe() != 0) {
        printf("SPS sensor probing failed\n");
        sensirion_sleep_usec(1000000);
    }

    coap_init();
    gpio_pin_configure_dt(&button0_spec, GPIO_INPUT);
    gpio_pin_interrupt_configure_dt(&button0_spec, GPIO_INT_EDGE_TO_ACTIVE);
    gpio_pin_configure_dt(&button1_spec, GPIO_INPUT);
    gpio_pin_interrupt_configure_dt(&button1_spec, GPIO_INT_EDGE_TO_ACTIVE);
    gpio_init_callback(&button_cb, button_pressed_cb, BIT(button0_spec.pin) | BIT(button1_spec.pin));
    gpio_add_callback(button0_spec.port, &button_cb);
    gpio_add_callback(button1_spec.port, &button_cb);

    k_timer_init(&send_timer, send_timer_callback, NULL);

    while (1)
    {
        k_msleep(SLEEP_TIME_MS);
    }
}
