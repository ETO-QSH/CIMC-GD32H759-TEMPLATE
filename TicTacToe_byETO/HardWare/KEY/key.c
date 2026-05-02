#include "key.h"

static uint8_t key1_last = 0;
static uint8_t key2_last = 0;
static uint8_t key3_last = 0;

static uint8_t key_read_level(uint32_t gpio_port, uint32_t gpio_pin)
{
	return (gpio_input_bit_get(gpio_port, gpio_pin) == SET) ? 1U : 0U;
}

static uint8_t key_read_pressed(uint32_t gpio_port, uint32_t gpio_pin)
{
	if (key_read_level(gpio_port, gpio_pin) == 0U)
	{
		return 0U;
	}

	delay_1ms(20);
	return key_read_level(gpio_port, gpio_pin);
}

void my_key_init(void)
{
	/* enable the key GPIO clock */
	rcu_periph_clock_enable(RCU_GPIOC);
	rcu_periph_clock_enable(RCU_GPIOG);
	rcu_periph_clock_enable(RCU_GPIOB); // enable GPIOB for KEY3
	/* configure key pin as input */
	gpio_mode_set(GPIOC , GPIO_MODE_INPUT , GPIO_PUPD_PULLDOWN, GPIO_PIN_13);
	gpio_mode_set(GPIOG , GPIO_MODE_INPUT , GPIO_PUPD_PULLDOWN, GPIO_PIN_3);
	gpio_mode_set(GPIOB , GPIO_MODE_INPUT , GPIO_PUPD_PULLDOWN, GPIO_PIN_12);

}

uint8_t my_key_scan(void)
{
	uint8_t key1 = key_read_pressed(GPIOC, GPIO_PIN_13);
	uint8_t key2 = key_read_pressed(GPIOG, GPIO_PIN_3);
	uint8_t key3 = key_read_pressed(GPIOB, GPIO_PIN_12);

	if (key1 == 0U)
	{
		key1_last = 0U;
	}
	else if (key1_last == 0U)
	{
		key1_last = 1U;
		return 1;
	}

	if (key2 == 0U)
	{
		key2_last = 0U;
	}
	else if (key2_last == 0U)
	{
		key2_last = 1U;
		return 2;
	}

	if (key3 == 0U)
	{
		key3_last = 0U;
	}
	else if (key3_last == 0U)
	{
		key3_last = 1U;
		return 3;
	}

	return 0;
}

uint8_t my_key_state(void)
{
	uint8_t state = 0U;

	if (key_read_level(GPIOC, GPIO_PIN_13) != 0U)
	{
		state |= 0x01U;
	}
	if (key_read_level(GPIOG, GPIO_PIN_3) != 0U)
	{
		state |= 0x02U;
	}
	if (key_read_level(GPIOB, GPIO_PIN_12) != 0U)
	{
		state |= 0x04U;
	}

	return state;
}
