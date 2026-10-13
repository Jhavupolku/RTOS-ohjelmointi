#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>

/****************************
 * Remember to add line:
 * CONFIG_HEAP_MEM_POOL_SIZE=1024
 * to prj.conf
 ****************************/

// Thread initializations
void receiver_task(void *, void *, void *);
void dispatcher_task(void *, void *, void *);
void red_task(void *, void *, void *);
void yellow_task(void *, void *, void *);
void green_task(void *, void *, void *);

#define STACKSIZE 500
#define PRIORITY 5

K_THREAD_DEFINE(receiver_thread,
		STACKSIZE,
		receiver_task,
		NULL, NULL, NULL,
		PRIORITY, 0, 0);

K_THREAD_DEFINE(dispatcher_thread,
		STACKSIZE,
		dispatcher_task,
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

// FIFO buffer
K_FIFO_DEFINE(data_fifo);

// FIFO data
struct data_t {
	void *fifo_reserved;
	char color;
};


// Condition variables
K_MUTEX_DEFINE(light_mutex);

K_CONDVAR_DEFINE(red_cond);
K_CONDVAR_DEFINE(yellow_cond);
K_CONDVAR_DEFINE(green_cond);


bool red_signal = false;
bool yellow_signal = false;
bool green_signal = false;


int main(void)
{
	printk("Main thread runs once\n");

	return 0;
}

// Receiver task
void receiver_task(void *, void *, void *)
{
	char sequence[] = "RYGYRYG";
	int index = 0;

	while (true) {

		char c = sequence[index];

		struct data_t *buf = k_malloc(sizeof(struct data_t));

		if (buf == NULL) {
			printk("Memory allocation failed\n");
			return;
		}

		buf->color = c;

		k_fifo_put(&data_fifo, buf);

		printk("Receiver added to FIFO: %c\n", c);

		index++;

		if (sequence[index] == '\0') {
			index = 0;
		}

		k_msleep(1000);
	}
}


// Dispatcher
void dispatcher_task(void *, void *, void *)
{
	struct data_t *received;

	while (true) {

		received = k_fifo_get(&data_fifo, K_FOREVER);

		printk("Dispatcher received: %c\n", received->color);

		k_mutex_lock(&light_mutex, K_FOREVER);

		switch (received->color) {

		case 'R':
			red_signal = true;
			k_condvar_signal(&red_cond);
			break;

		case 'Y':
			yellow_signal = true;
			k_condvar_signal(&yellow_cond);
			break;

		case 'G':
			green_signal = true;
			k_condvar_signal(&green_cond);
			break;

		default:
			printk("Unknown color: %c\n", received->color);
			break;
		}

		k_mutex_unlock(&light_mutex);

		k_free(received);
	}
}


// Red task
void red_task(void *, void *, void *)
{
	while (true) {

		k_mutex_lock(&light_mutex, K_FOREVER);

		while (!red_signal) {
			k_condvar_wait(&red_cond,
				       &light_mutex,
				       K_FOREVER);
		}

		red_signal = false;

		k_mutex_unlock(&light_mutex);

		printk("RED LIGHT ON\n");

		k_msleep(1000);

		printk("RED LIGHT OFF\n");
	}
}

// Yellow task
void yellow_task(void *, void *, void *)
{
	while (true) {

		k_mutex_lock(&light_mutex, K_FOREVER);

		while (!yellow_signal) {
			k_condvar_wait(&yellow_cond,
				       &light_mutex,
				       K_FOREVER);
		}

		yellow_signal = false;

		k_mutex_unlock(&light_mutex);

		printk("YELLOW LIGHT ON\n");

		k_msleep(1000);

		printk("YELLOW LIGHT OFF\n");
	}
}

// Green task
void green_task(void *, void *, void *)
{
	while (true) {

		k_mutex_lock(&light_mutex, K_FOREVER);

		while (!green_signal) {
			k_condvar_wait(&green_cond,
				       &light_mutex,
				       K_FOREVER);
		}

		green_signal = false;

		k_mutex_unlock(&light_mutex);

		printk("GREEN LIGHT ON\n");

		k_msleep(1000);

		printk("GREEN LIGHT OFF\n");
	}
}
