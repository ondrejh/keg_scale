#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "board.h"
#include "tasks.h"

static void board_power_hold(void)
{
    gpio_config_t io = {
        .pin_bit_mask = 1ULL << BOARD_SYS_POWER_HOLD,
        .mode = GPIO_MODE_OUTPUT,
    };
    gpio_config(&io);
    gpio_set_level(BOARD_SYS_POWER_HOLD, 1);
}

void app_main(void)
{
    board_power_hold();

    BaseType_t ok;
    ok = xTaskCreate(task_disp,  "task_disp",  TASK_DISP_STACK,  NULL, TASK_DISP_PRIO,  NULL);
    configASSERT(ok == pdPASS);
    ok = xTaskCreate(task_scale, "task_scale", TASK_SCALE_STACK, NULL, TASK_SCALE_PRIO, NULL);
    configASSERT(ok == pdPASS);
    ok = xTaskCreate(task_net,   "task_net",   TASK_NET_STACK,   NULL, TASK_NET_PRIO,   NULL);
    configASSERT(ok == pdPASS);
}
