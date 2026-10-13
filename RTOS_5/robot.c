#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <string.h>
#include "TimeParser.h"

#define UART_DEVICE_NODE DT_CHOSEN(zephyr_shell_uart)
#define UART_MSG_SIZE 20

static const struct device *const uart_dev =
    DEVICE_DT_GET(UART_DEVICE_NODE);

int main(void)
{
    if (!device_is_ready(uart_dev)) {
        printk("UART initialization failed!\r\n");
        return 0;
    }

    k_msleep(100);
    printk("Started time parser test\r\n");

    char rc;
    char uart_msg[UART_MSG_SIZE];
    int uart_msg_cnt = 0;

    memset(uart_msg, 0, sizeof(uart_msg));

    while (true) {
        while (uart_poll_in(uart_dev, &rc) == 0) {
            if (rc == 'X') {
                uart_msg[uart_msg_cnt] = '\0';

                int result = time_parse(uart_msg);
                printk("%dX", result);

                uart_msg_cnt = 0;
                memset(uart_msg, 0, sizeof(uart_msg));
            } else if (uart_msg_cnt < UART_MSG_SIZE - 1) {
                uart_msg[uart_msg_cnt++] = rc;
            } else {
                // Message character limit
                uart_msg_cnt = 0;
                memset(uart_msg, 0, sizeof(uart_msg));
                printk("-1X");
            }
        }

        k_msleep(10);
    }

    return 0;
}