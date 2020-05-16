#if __has_include("build/config/sdkconfig.h")
#include "build/config/sdkconfig.h"
#endif
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

#define GPIO_OUT_00     GPIO_NUM_16
#define GPIO_IN_00      GPIO_NUM_4

static xQueueHandle my_queue = NULL;
const TaskHandle_t my_task1_handle;

static void gpio_isr(void* arg)
{
    uint32_t gpio_num = (uint32_t)arg;
    xQueueSendFromISR(my_queue, &gpio_num, NULL);
}

static void my_task1(void* arg)
{
    int data;
    while(true)
    {
        if(xQueueReceive(my_queue, &data, portMAX_DELAY))
        {
            printf("called from ISR, GPIO_IN_00(GPIO%d) is %s\n", data, ((gpio_get_level(data)==1)?"HIGH":"LOW"));
            gpio_set_level(GPIO_OUT_00, gpio_get_level(GPIO_IN_00));
        }
    }
}

void app_main(void)
{
    gpio_config_t io_conf;
    io_conf.pin_bit_mask = 1ULL << GPIO_OUT_00;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_DEF_OUTPUT;
    io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);
    io_conf.pin_bit_mask = 1ULL << GPIO_IN_00;
    io_conf.mode = GPIO_MODE_DEF_INPUT;
    io_conf.intr_type = GPIO_PIN_INTR_ANYEDGE;
    gpio_config(&io_conf);
    
    my_queue = xQueueCreate(10, sizeof(int));
    xTaskCreate(my_task1, "my_task1", 2048, NULL, 0, &my_task1_handle);

    printf("GPIO_IN_00 degeri:0x%x\n", GPIO_IN_00);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(GPIO_IN_00, gpio_isr, (void *)GPIO_IN_00);
    vTaskDelete(NULL);
}
