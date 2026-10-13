#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/timing/timing.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>


// Debug FIFO
#define DEBUG_STACKSIZE       500
#define DEBUG_PRIORITY        7
#define DEBUG_MSG_SIZE        128
#define DEBUG_MSG_COUNT       16


struct debug_msg {
	void *fifo_reserved;
	char text[DEBUG_MSG_SIZE];
};


K_FIFO_DEFINE(debug_fifo);

K_MEM_SLAB_DEFINE(debug_slab,
		  sizeof(struct debug_msg),
		  DEBUG_MSG_COUNT,
		  4);


// Debugs go through this
static void debug_print(const char *fmt, ...)
{
	struct debug_msg *msg;
	va_list args;

	int ret = k_mem_slab_alloc(&debug_slab,
				   (void **)&msg,
				   K_FOREVER);

	if (ret != 0) {
		return;
	}

	va_start(args, fmt);

	vsnprintf(msg->text,
		  sizeof(msg->text),
		  fmt,
		  args);

	va_end(args);

	k_fifo_put(&debug_fifo, msg);
}

// Debug task
void debug_task(void *, void *, void *);

K_THREAD_DEFINE(debug_thread,
		DEBUG_STACKSIZE,
		debug_task,
		NULL, NULL, NULL,
		DEBUG_PRIORITY, 0, 0);


void debug_task(void *, void *, void *)
{
	struct debug_msg *msg;

	while (true) {

		msg = k_fifo_get(&debug_fifo, K_FOREVER);

		if (msg != NULL) {

			printk("%s", msg->text);

			k_mem_slab_free(&debug_slab, (void *)msg);
		}
	}
}

// Led pins
static const struct gpio_dt_spec red =
	GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

static const struct gpio_dt_spec yellow =
	GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);

static const struct gpio_dt_spec green =
	GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios);

// Thread initialization
#define STACKSIZE 500
#define PRIORITY 5

void red_led_task(void *, void *, void *);
void yellow_led_task(void *, void *, void *);
void green_led_task(void *, void *, void *);

K_THREAD_DEFINE(red_thread,
		STACKSIZE,
		red_led_task,
		NULL, NULL, NULL,
		PRIORITY, 0, 0);

K_THREAD_DEFINE(yellow_thread,
		STACKSIZE,
		yellow_led_task,
		NULL, NULL, NULL,
		PRIORITY, 0, 0);

K_THREAD_DEFINE(green_thread,
		STACKSIZE,
		green_led_task,
		NULL, NULL, NULL,
		PRIORITY, 0, 0);

// Task measurement timer
volatile uint64_t red_time_us = 0;
volatile uint64_t yellow_time_us = 0;
volatile uint64_t green_time_us = 0;

// Led initialization
void init_leds(void)
{
	int ret;

	ret = gpio_pin_configure_dt(&red, GPIO_OUTPUT_INACTIVE);

	if (ret < 0) {
		debug_print("Red LED init failed\n");
	}

	ret = gpio_pin_configure_dt(&yellow, GPIO_OUTPUT_INACTIVE);

	if (ret < 0) {
		debug_print("Yellow LED init failed\n");
	}

	ret = gpio_pin_configure_dt(&green, GPIO_OUTPUT_INACTIVE);

	if (ret < 0) {
		debug_print("Green LED init failed\n");
	}

	debug_print("LEDs initialized ok\n");
}

int main(void)
{
	timing_init();
	timing_start();

	init_leds();

	k_msleep(100);

	debug_print("Program started..\n");

	timing_stop();

	while (true) {

		k_sleep(K_SECONDS(3));

		uint64_t total_time_us =
			red_time_us +
			yellow_time_us +
			green_time_us;

		debug_print("\n============================\n");
		debug_print("SEQUENCE TIMES\n");
		debug_print("============================\n");

		debug_print("Red task:    %llu us\n",
			    red_time_us);

		debug_print("Yellow task: %llu us\n",
			    yellow_time_us);

		debug_print("Green task:  %llu us\n",
			    green_time_us);

		debug_print("----------------------------\n");

		debug_print("Total:       %llu us\n",
			    total_time_us);

		debug_print("============================\n\n");
	}

	return 0;
}

// Red task
void red_led_task(void *, void *, void *)
{
	debug_print("Red led thread started\n");

	while (true) {

		timing_start();

		timing_t start_time = timing_counter_get();

		gpio_pin_set_dt(&red, 1);

		debug_print("Red ON\n");

		k_sleep(K_SECONDS(1));

		gpio_pin_set_dt(&red, 0);

		debug_print("Red OFF\n");

		k_sleep(K_SECONDS(1));

		timing_t end_time = timing_counter_get();

		timing_stop();

		uint64_t timing_ns =
			timing_cycles_to_ns(
				timing_cycles_get(
					&start_time,
					&end_time));

		red_time_us = timing_ns / 1000;

		debug_print("Red task: %llu us\n",
			    red_time_us);
	}
}

// Yellow task
void yellow_led_task(void *, void *, void *)
{
	debug_print("Yellow led thread started\n");

	while (true) {

		timing_start();

		timing_t start_time = timing_counter_get();

		gpio_pin_set_dt(&yellow, 1);

		debug_print("Yellow ON\n");

		k_sleep(K_SECONDS(1));

		gpio_pin_set_dt(&yellow, 0);

		debug_print("Yellow OFF\n");

		k_sleep(K_SECONDS(1));

		timing_t end_time = timing_counter_get();

		timing_stop();

		uint64_t timing_ns =
			timing_cycles_to_ns(
				timing_cycles_get(
					&start_time,
					&end_time));

		yellow_time_us = timing_ns / 1000;

		debug_print("Yellow task: %llu us\n",
			    yellow_time_us);
	}
}

// Green task
void green_led_task(void *, void *, void *)
{
	debug_print("Green led thread started\n");

	while (true) {

		timing_start();

		timing_t start_time = timing_counter_get();

		gpio_pin_set_dt(&green, 1);

		debug_print("Green ON\n");

		k_sleep(K_SECONDS(1));

		gpio_pin_set_dt(&green, 0);

		debug_print("Green OFF\n");

		k_sleep(K_SECONDS(1));

		timing_t end_time = timing_counter_get();

		timing_stop();

		uint64_t timing_ns =
			timing_cycles_to_ns(
				timing_cycles_get(
					&start_time,
					&end_time));

		green_time_us = timing_ns / 1000;

		debug_print("Green task: %llu us\n",
			    green_time_us);
	}
}