#include "SWM181.h"

void IRQ4_Handler(void);

void SerialInit(void);

int main(void)
{
	uint32_t res;
	
	CACHE_InitStructure CACHE_initStruct;
	
	SystemInit();
	
	SerialInit();
   	
	CACHE_initStruct.Alg = CACHE_ALG_LRU;
	CACHE_initStruct.Threshold = 96;		//ע�⣺SRAM_SWITCH = 0ʱ��128��Slot��Code RAM��= 1ʱ��64��Slot��Code RAM
	CACHE_initStruct.ResetIEn = 1;
	CACHE_Init(&CACHE_initStruct);
	
	IRQ_Connect(IRQ0_15_CACHE, IRQ4_IRQ, 3);
	
	res = CACHE_Prefetch((uint32_t)IRQ4_Handler);
	if(res == 0)
	{
		printf("Prefetch Success\r\n");
		printf("Prefetch Slot = %d\r\n", CACHE_PrefetchSlotNumber());
	}
	else
	{
		printf("Prefetch Failure\r\n");
	}
	
	res = CACHE_Invalid((uint32_t)IRQ4_Handler);
	if(res == 0)
	{
		printf("Invalid Success\r\n");
		printf("Invalid Slot = %d\r\n", CACHE_InvalidSlotNumber());
	}
	else
	{
		printf("Invalid Failure\r\n");
	}
	
	while(1==1)
	{
	}
}

void IRQ4_Handler(void)
{
	if(CACHE->IF & CACHE_IF_RESET_Msk)
	{
		CACHE->IF = CACHE_IF_RESET_Msk;
		
		CACHE_Reset();
	}
	else if(CACHE->IF & CACHE_IF_INVALID_Msk)
	{
		CACHE->IF = CACHE_IF_INVALID_Msk;
	}
	else if(CACHE->IF & CACHE_IF_PREFETCH_Msk)
	{
		CACHE->IF = CACHE_IF_PREFETCH_Msk;
	}
}

void SerialInit(void)
{
	UART_InitStructure UART_initStruct;
	
	PORT_Init(PORTA, PIN0, FUNMUX_UART0_RXD, 1);	//GPIOA.0����ΪUART0��������
	PORT_Init(PORTA, PIN1, FUNMUX_UART0_TXD, 0);	//GPIOA.1����ΪUART0�������
 	
 	UART_initStruct.Baudrate = 57600;
	UART_initStruct.DataBits = UART_DATA_8BIT;
	UART_initStruct.Parity = UART_PARITY_NONE;
	UART_initStruct.StopBits = UART_STOP_1BIT;
	UART_initStruct.RXThresholdIEn = 0;
	UART_initStruct.TXThresholdIEn = 0;
	UART_initStruct.TimeoutIEn = 0;
 	UART_Init(UART0, &UART_initStruct);
	UART_Open(UART0);
}

/****************************************************************************************************************************************** 
* ��������: fputc()
* ����˵��: printf()ʹ�ô˺������ʵ�ʵĴ��ڴ�ӡ����
* ��    ��: int ch		Ҫ��ӡ���ַ�
*			FILE *f		�ļ����
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
int fputc(int ch, FILE *f)
{
	UART_WriteByte(UART0, ch);
	
	while(UART_IsTXBusy(UART0));
 	
	return ch;
}
