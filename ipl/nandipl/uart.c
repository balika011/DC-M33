#include <stdint.h>

#define REG_BUS_CLK_ENABLE 0xBC100050
#define REG_CLK2_ENABLE 0xBC100058
#define REG_IO_ENABLE 0xBC100078

#define REG_GPIO_OUTPUT 0xBE240000

#define UART_IO_BASE	0xBE400000

#define DBG_UART4_BASE	(UART_IO_BASE + 3 * 0x40000)
#define HP_REMOTE_BASE	(UART_IO_BASE + 4 * 0x40000)
#define IRDA_BASE 	(UART_IO_BASE + 5 * 0x40000)

#define RX_TX_OFFSET		0x00
#define STATUS_OFFSET		0x20
#define BPS_HIGH_OFFSET	0x24
#define BPS_LOW_OFFSET		0x28
#define UNK_2C_OFFSET		0x2C
#define UNK_30_OFFSET		0x30
#define FIFO_OFFSET		0x34


void uart_init()
{
	*(uint32_t *) REG_BUS_CLK_ENABLE |= (1 << 14);
	*(uint32_t *) REG_CLK2_ENABLE |= (1 << 10);
	*(uint32_t *) REG_IO_ENABLE |= (1 << 20);

	*(uint32_t *)(HP_REMOTE_BASE + UNK_30_OFFSET) = 0x300;
	
	uint32_t val = 96000000 / 115200;

	*(uint32_t *)(HP_REMOTE_BASE + BPS_HIGH_OFFSET) = val >> 6;
	*(uint32_t *)(HP_REMOTE_BASE + BPS_LOW_OFFSET) = val & 0x3F;
	
	*(uint32_t *)(HP_REMOTE_BASE + UNK_2C_OFFSET) = 0x60;
	
	*(uint32_t *)(HP_REMOTE_BASE + UNK_2C_OFFSET) |= 0x10;
	*(uint32_t *)(HP_REMOTE_BASE + FIFO_OFFSET) = 0;
	
	*(uint32_t *)(HP_REMOTE_BASE + UNK_30_OFFSET) = 0x301;
}

void sleep(uint32_t usec)
{
	volatile uint32_t dummy = 0;

	while (usec-- > 0) {
		for (uint32_t i = 0; i < 10; i++) {
			dummy ^= *(volatile uint32_t *) REG_GPIO_OUTPUT;
		}
	}
}

void _putchar(char c)
{
	//if (c == '\n')
	//	_putchar('\r');

	while ((*(uint32_t *)(HP_REMOTE_BASE + STATUS_OFFSET) & 0x20) != 0);

	*(uint32_t *)(HP_REMOTE_BASE + RX_TX_OFFSET) = c;
	
	sleep(100);
}
