
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/atomic.h>

#define LED_NODE DT_ALIAS(led0)

#if !DT_NODE_EXISTS(LED_NODE)
#error "No led0 alias defined in devicetree"
#endif

static const struct gpio_dt_spec led3 =
    GPIO_DT_SPEC_GET(LED_NODE, gpios);

static const struct gpio_dt_spec y0 =
    GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);

static atomic_t led_state = ATOMIC_INIT(0);

void thread_led_blinky(void *p1, void *p2, void *p3)
{
    int ret;

    if (!gpio_is_ready_dt(&led3)) {
        printk("LED3 GPIO controller is not ready\n");
        return;
    }

    ret = gpio_pin_configure_dt(&led3, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) {
        printk("Failed to configure LED3: %d\n", ret);
        return;
    }

    while (1) {
        gpio_pin_set_dt(&led3, 1);
        atomic_set(&led_state, 1);
        k_msleep(2000);

        gpio_pin_set_dt(&led3, 0);
        atomic_set(&led_state, 0);
        k_msleep(2000);
    }
}

void thread_y0_follow(void *p1, void *p2, void *p3)
{
    int ret;
    int last_state = -1;
    int current_state;

    if (!gpio_is_ready_dt(&y0)) {
        printk("Y0 GPIO controller is not ready\n");
        return;
    }

    ret = gpio_pin_configure_dt(&y0, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) {
        printk("Failed to configure Y0: %d\n", ret);
        return;
    }

    while (1) {
        current_state = atomic_get(&led_state);

        if (current_state != last_state) {
            gpio_pin_set_dt(&y0, current_state);
            last_state = current_state;
        }

        k_msleep(10);
    }
}

K_THREAD_DEFINE(thread_led_blinky_id, 1024,
                thread_led_blinky,
                NULL, NULL, NULL,
                7, 0, 0);

K_THREAD_DEFINE(thread_y0_follow_id, 1024,
                thread_y0_follow,
                NULL, NULL, NULL,
                7, 0, 0);

int main(void)
{
    printk("Exercise 2: LED3 blinks, Y0 follows LED3\n");

    return 0;
}
