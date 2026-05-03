#include "Function.h"
#include "game.h"
#include "usart.h"
#include "OLED.h"

static void cache_enable(void);

void System_Init(void)
{

	cache_enable();

	systick_config();

	OLED_Init();

	my_usart_init();

}

void UsrFunction(void)
{
	Game_Start();
}

static void cache_enable(void)
{
	/* Enable I-Cache */
	SCB_EnableICache();

	/* Enable D-Cache */
	SCB_EnableDCache();
}
