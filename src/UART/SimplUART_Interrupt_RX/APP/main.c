#include "SWM181.h"
#include "CircleBuffer.h"

CircleBuffer_t CirBuf;


void SerialInit(void);

int main(void)
{
	SystemInit();
	
	SerialInit();
   	
	while(1==1)
	{
		uint8_t chr;
		if(CirBuf_Read(&CirBuf, &chr, 1))
			printf("%c", chr);
	}
}


void IRQ0_Handler(void)
{
	uint32_t chr;
	
	if(UART_INTStat(UART0, UART_IT_RX_THR | UART_IT_RX_TOUT))
	{
		while(UART_IsRXFIFOEmpty(UART0) == 0)
		{
			if(UART_ReadByte(UART0, &chr) == 0)
			{
				CirBuf_Write(&CirBuf, (uint8_t *)&chr, 1);
			}
		}
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
	UART_initStruct.RXThreshold = 3;
	UART_initStruct.RXThresholdIEn = 1;
	UART_initStruct.TXThreshold = 3;
	UART_initStruct.TXThresholdIEn = 0;
	UART_initStruct.TimeoutTime = 10;		//10���ַ�ʱ����δ���յ��µ������򴥷���ʱ�ж�
	UART_initStruct.TimeoutIEn = 1;
 	UART_Init(UART0, &UART_initStruct);
	
	IRQ_Connect(IRQ0_15_UART0, IRQ0_IRQ, 1);
	
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
