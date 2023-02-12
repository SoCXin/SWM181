#include "SWM181.h"

void SerialInit(void);

int main(void)
{
	SystemInit();
	
	SerialInit();
		
	GPIO_Init(GPIOA, PIN5, 1, 0, 0, 0);			//�������LED
	
	WDT_Init(WDT, SystemCoreClock/2, WDT_MODE_INTERRUPT);
	
	IRQ_Connect(IRQ0_15_WDT, IRQ4_IRQ, 2);
	
	WDT_Start(WDT);								//����WDT
	
	while(1==1)
	{
		
	}
}

void IRQ4_Handler(void)
{
	WDT_INTClr(WDT);
	
	GPIO_InvBit(GPIOA, PIN5);
	
	printf("WDT Handler\r\n");
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
