#include <stdint.h>

#ifdef IPL_01G

#define syscon_tx_dword(x, y, z) ((int (*) (uint32_t, uint32_t, uint32_t)) 0x4005BC4)(x, y, z)
#define syscon_rx_dword(x, y) ((int (*) (uint32_t, uint32_t)) 0x4005AC0)(x, y)
#define return_to_payload() ((void (*)()) 0x4000364)()

#elif IPL_02G

#define syscon_tx_dword(x, y, z) ((int (*) (uint32_t, uint32_t, uint32_t)) 0x4006B98)(x, y, z)
#define syscon_rx_dword(x, y) ((int (*) (uint32_t, uint32_t)) 0x4006A54)(x, y)
#define return_to_payload() ((void (*)()) 0x4000364)()

#elif IPL_03G

#define syscon_tx_dword(x, y, z) ((int (*) (uint32_t, uint32_t, uint32_t)) 0x4006BA8)(x, y, z)
#define syscon_rx_dword(x, y) ((int (*) (uint32_t, uint32_t)) 0x4006A64)(x, y)
#define return_to_payload() ((void (*)()) 0x4000364)()

#else

#error Define IPL

#endif

extern uint32_t size_payloadex;
extern uint8_t payloadex;
void Dcache();
void Icache();

void *memcpy(void *dest, void *src, uint32_t size)
{
	uint32_t *d = (uint32_t *) dest;
	uint32_t *s = (uint32_t *) src;
	
	size /= 4;
	
	while (size--)
		*(d++) = *(s++);

	return dest;
}

int main()
{
	memcpy((uint8_t *) 0x8FC0000, &payloadex, size_payloadex);
	
#ifdef MSIPL
	// power on ms 1,0x4c,3
	syscon_tx_dword(1, 0x4C, 3);
#endif
	
	// Get ctrl
	*(uint32_t *) 0x8FB0000 = -1;
	syscon_rx_dword(0x8FB0000, 0x07);

	Dcache();

	return_to_payload();
}
