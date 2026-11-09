#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/atomic.h>

#define LED_NODE DT_ALIAS(led0)

#if !DT_NODE_EXISTS(LED_NODE)
#error "No led0 alias defined in devicetree"
#endif

static const struct gpio_dt_spec led3 =
    GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

static const struct gpio_dt_spec y0 =
    GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);

static atomic_t led_state = ATOMIC_INIT(0);

void thread_led_blinky(void *p1, void *p2, void *p3)
{
    gpio_pin_configure_dt(&led3, GPIO_OUTPUT_INACTIVE);

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
    int last_state = -1;
    int current_state;

    gpio_pin_configure_dt(&y0, GPIO_OUTPUT_INACTIVE);

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
    int ret;

    if (!gpio_is_ready_dt(&led)) {
        printk("LED GPIO controller is not ready\n");
        return 0;
    }

    ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) {
        printk("Failed to configure LED: %d\n", ret);
        return 0;
    }

    printk("Zephyr STM32F4 Discovery: toggling LED on PE4\n");

    while (1) {
        ret = gpio_pin_toggle_dt(&led);
        if (ret < 0) {
            printk("Failed to toggle LED: %d\n", ret);
        }
        k_msleep(500);
    }

    return 0;
}
