#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <zephyr/net/openthread.h>
#include <openthread/thread.h>
#include <openthread/udp.h>
#include <openthread/coap.h>
#include <zephyr/drivers/sensor.h>
#include <stdio.h>

#define SLEEP_TIME_MS 1000
#define DATA_SENDING_INTERVAL 30000
#define TEXTBUFFER_SIZE 30
#define BUTTON0_NODE DT_NODELABEL(button0) // DT_N_S_buttons_S_button_0
#define BUTTON1_NODE DT_NODELABEL(button1) // DT_N_S_buttons_S_button_0

//FOR temperature testing
const struct device *mcp9808_dev = DEVICE_DT_GET_ANY(microchip_mcp9808);
static struct sensor_value temp;
//

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


char myText[TEXTBUFFER_SIZE];
uint16_t myText_length = 0;

static void storedata_response_send(otMessage *p_request_message,
                                    const otMessageInfo *p_message_info);


//For Temperature testing
float mcp9808_calculateTemperature(uint8_t upperByte, uint8_t lowerByte){
    float temperature = (upperByte&0x0F)*16+(float)(lowerByte)/16;
    if ((upperByte & 0x10) == 0x10){ //Temperatur < 0°C
    temperature=256-temperature; //von 2^8 subtrahieren, da Zweierkompliment
    }
    return temperature;
}
//

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
        err = sensor_sample_fetch(mcp9808_dev);
		if (err < 0){ printk("MCP9808 write failed: %d\n", err); break; }
		err = sensor_channel_get(mcp9808_dev, SENSOR_CHAN_AMBIENT_TEMP, &temp);
		if (err < 0){ printk("MCP9808 read failed: %d\n", err); break; }
		// printk("MCP9808: %.2f Cel\n", sensor_value_to_double(&temp));

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
        
        char temperature_string[100];
        snprintf(temperature_string, sizeof(temperature_string), "{\"temperature\": %.2f}", sensor_value_to_double(&temp));
        
        error = otMessageAppend(myMessage, temperature_string,
                                strlen(temperature_string));
        if (error != OT_ERROR_NONE)
        {
            break;
        }
        memset(&myMessageInfo, 0, sizeof(myMessageInfo));
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

K_WORK_DEFINE(mcp9808_work, coap_send_data_request);

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
        k_work_submit(&mcp9808_work);
    }
}


int main(void)
{
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