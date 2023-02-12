#include "SWM181.h"

void SerialInit(void);

int main(void)
{
	SystemInit();
	
	SerialInit();
   	
	PORT_Init(PORTB, PIN2, PORTB_PIN2_CMP1N, 0);
	PORT_Init(PORTB, PIN3, PORTB_PIN3_CMP1P, 0);
	
	CMP_Init(CMP1, 0, 1, 1);
	CMP_SetVRef(CMP_VREF_1V50);
	
	IRQ_Connect(IRQ0_15_CMP, IRQ7_IRQ, 2);
	
	CMP_Open(CMP1);
	
	while(1==1)
	{
		
	}
}

void IRQ7_Handler(void)
{
	printf("IRQ7 happened!\r\n");
 	
	CMP_INTClr(CMP1);
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
