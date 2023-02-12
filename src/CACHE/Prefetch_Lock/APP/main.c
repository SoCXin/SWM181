#include "SWM181.h"


void SerialInit(void);

int main(void)
{		
	uint32_t addr, temp;
	
	CACHE->CR = (1 << CACHE_CR_RST_Pos) | (CACHE_ALG_LRU << CACHE_CR_ALG_Pos);
	for(addr = 0; addr < 0x800; addr += 128)	// ������ǰ2K������Cache�У��ڷ�ɢ�����ļ��н���������жϷ������λ�������ǰ2K
	{
		temp = CACHE_Prefetch(addr);
		temp = temp;
	}
	
	SystemInit();
	
	SerialInit();
   	
	SysTick_Config(SystemCoreClock / 10);
	
	while(1==1)
	{
	}
}


__attribute__((section("..LockInCache")))
void SysTick_Handler(void)
{
	
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
