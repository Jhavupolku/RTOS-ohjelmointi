#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>
#include <string.h>
#include <stdlib.h>

/****************************
 * Remember to add line:
 * CONFIG_HEAP_MEM_POOL_SIZE=1024
 * to prj.conf
 ****************************/

// Thread initialization
#define STACKSIZE 500
#define PRIORITY 5

void uart_task(void *, void *, void *);
void dispatcher_task(void *, void *, void *);
void red_task(void *, void *, void *);
void yellow_task(void *, void *, void *);
void green_task(void *, void *, void *);


// UART initialization
#define UART_DEVICE_NODE DT_CHOSEN(zephyr_shell_uart)

static const struct device *const uart_dev =
	DEVICE_DT_GET(UART_DEVICE_NODE);


// Dispatcher FIFO
K_FIFO_DEFINE(dispatcher_fifo);

struct dispatcher_data_t {
	void *fifo_reserved;
	char msg[20];
};


// Color FIFO
K_FIFO_DEFINE(red_fifo);
K_FIFO_DEFINE(yellow_fifo);
K_FIFO_DEFINE(green_fifo);


// Dispatcher to task data
struct light_data_t {
	void *fifo_reserved;
	char color;
	int time;
};


// Condition variables
K_MUTEX_DEFINE(light_mutex);

K_CONDVAR_DEFINE(red_cond);
K_CONDVAR_DEFINE(yellow_cond);
K_CONDVAR_DEFINE(green_cond);


bool red_ready = false;
bool yellow_ready = false;
bool green_ready = false;


// UART initialization
int init_uart(void)
{
	if (!device_is_ready(uart_dev)) {
		return 1;
	}

	return 0;
}


int main(void)
{
	int ret = init_uart();

	if (ret != 0) {
		printk("UART initialization failed!\n");
		return ret;
	}

	printk("Main thread runs once\n");

	return 0;
}


// UART task
void uart_task(void *unused1, void *unused2, void *unused3)
{
	char rc = 0;

	char uart_msg[20];
	memset(uart_msg, 0, sizeof(uart_msg));

	int uart_msg_cnt = 0;

	while (true) {

		if (uart_poll_in(uart_dev, &rc) == 0) {

			if (rc == '\r') {

				if (uart_msg_cnt > 0) {

					printk("UART msg: %s\n", uart_msg);

					struct dispatcher_data_t *buf =
						k_malloc(sizeof(struct dispatcher_data_t));

					if (buf == NULL) {
						printk("malloc failed\n");
						return;
					}

					strncpy(buf->msg,
						uart_msg,
						sizeof(buf->msg) - 1);

					buf->msg[sizeof(buf->msg) - 1] = '\0';

					k_fifo_put(&dispatcher_fifo, buf);
				}

				uart_msg_cnt = 0;
				memset(uart_msg, 0, sizeof(uart_msg));
			}
			else {

				if (uart_msg_cnt < sizeof(uart_msg) - 1) {

					uart_msg[uart_msg_cnt] = rc;
					uart_msg_cnt++;
					uart_msg[uart_msg_cnt] = '\0';
				}
				else {
					printk("UART message too long\n");

					uart_msg_cnt = 0;
					memset(uart_msg, 0,
					       sizeof(uart_msg));
				}
			}
		}

		k_msleep(10);
	}
}

// Dispatcher task
void dispatcher_task(void *unused1, void *unused2, void *unused3)
{
	while (true) {

		struct dispatcher_data_t *rec_item =
			k_fifo_get(&dispatcher_fifo, K_FOREVER);

		char sequence[20];

		memcpy(sequence, rec_item->msg, sizeof(sequence));

		k_free(rec_item);

		printk("Dispatcher: %s\n", sequence);

		char color = sequence[0];

		int time = atoi(sequence + 2);

		printk("Data: %c %d\n", color, time);

		struct light_data_t *buf =
			k_malloc(sizeof(struct light_data_t));

		if (buf == NULL) {
			printk("malloc failed\n");
			return;
		}

		buf->color = color;
		buf->time = time;

		switch (color) {

		case 'R':
			k_fifo_put(&red_fifo, buf);

			k_mutex_lock(&light_mutex, K_FOREVER);
			red_ready = true;
			k_condvar_signal(&red_cond);
			k_mutex_unlock(&light_mutex);

			break;


		case 'Y':
			k_fifo_put(&yellow_fifo, buf);

			k_mutex_lock(&light_mutex, K_FOREVER);
			yellow_ready = true;
			k_condvar_signal(&yellow_cond);
			k_mutex_unlock(&light_mutex);

			break;


		case 'G':
			k_fifo_put(&green_fifo, buf);

			k_mutex_lock(&light_mutex, K_FOREVER);
			green_ready = true;
			k_condvar_signal(&green_cond);
			k_mutex_unlock(&light_mutex);

			break;


		default:
			printk("Unknown color: %c\n", color);
			k_free(buf);
			break;
		}

	}
}

// Red task
void red_task(void *unused1, void *unused2, void *unused3)
{
	while (true) {

		k_mutex_lock(&light_mutex, K_FOREVER);

		while (!red_ready) {
			k_condvar_wait(&red_cond,
				       &light_mutex,
				       K_FOREVER);
		}

		red_ready = false;

		k_mutex_unlock(&light_mutex);

		struct light_data_t *data =
			k_fifo_get(&red_fifo, K_FOREVER);

		printk("RED: ON for %d ms\n", data->time);

		// gpio_pin_set_dt(&red_led, 1);

		k_msleep(data->time);

		// gpio_pin_set_dt(&red_led, 0);

		printk("RED: OFF\n");

		k_free(data);
	}
}

// Yellow task
void yellow_task(void *unused1, void *unused2, void *unused3)
{
	while (true) {

		k_mutex_lock(&light_mutex, K_FOREVER);

		while (!yellow_ready) {
			k_condvar_wait(&yellow_cond,
				       &light_mutex,
				       K_FOREVER);
		}

		yellow_ready = false;

		k_mutex_unlock(&light_mutex);


		struct light_data_t *data =
			k_fifo_get(&yellow_fifo, K_FOREVER);

		printk("YELLOW: ON for %d ms\n", data->time);

		// gpio_pin_set_dt(&yellow_led, 1);

		k_msleep(data->time);

		// gpio_pin_set_dt(&yellow_led, 0);

		printk("YELLOW: OFF\n");

		k_free(data);
	}
}

// Green task
void green_task(void *unused1, void *unused2, void *unused3)
{
	while (true) {

		k_mutex_lock(&light_mutex, K_FOREVER);

		while (!green_ready) {
			k_condvar_wait(&green_cond,
				       &light_mutex,
				       K_FOREVER);
		}

		green_ready = false;

		k_mutex_unlock(&light_mutex);


		struct light_data_t *data =
			k_fifo_get(&green_fifo, K_FOREVER);

		printk("GREEN: ON for %d ms\n", data->time);

		// gpio_pin_set_dt(&green_led, 1);

		k_msleep(data->time);

		// gpio_pin_set_dt(&green_led, 0);

		printk("GREEN: OFF\n");

		k_free(data);
	}
}

// Thread defition
K_THREAD_DEFINE(dis_thread,
		STACKSIZE,
		dispatcher_task,
		NULL, NULL, NULL,
		PRIORITY, 0, 0);

K_THREAD_DEFINE(uart_thread,
		STACKSIZE,
		uart_task,
		NULL, NULL, NULL,
		PRIORITY, 0, 0);

K_THREAD_DEFINE(red_thread,
		STACKSIZE,
		red_task,
		NULL, NULL, NULL,
		PRIORITY, 0, 0);

K_THREAD_DEFINE(yellow_thread,
		STACKSIZE,
		yellow_task,
		NULL, NULL, NULL,
		PRIORITY, 0, 0);

K_THREAD_DEFINE(green_thread,
		STACKSIZE,
		green_task,
		NULL, NULL, NULL,
		PRIORITY, 0, 0);
