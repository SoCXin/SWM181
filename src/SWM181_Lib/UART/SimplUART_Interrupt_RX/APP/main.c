#include "SWM181.h"

#include <string.h>


void SerialInit(void);

#define UART_RX_LEN	 128

uint32_t UART_GetChars(char *data);

int main(void)
{
	uint32_t len, i;
	char buffer[UART_RX_LEN] = {0};
	
	SystemInit();
	
	SerialInit();
   	
	while(1==1)
	{
		if((len = UART_GetChars(buffer)) != 0)
		{
			for(i = 0; i < len; i++)
				printf("%c", buffer[i]);
		}
	}
}

char UART_RXBuffer[UART_RX_LEN] = {0};
uint32_t UART_RXIndex = 0;

uint32_t UART_GetChars(char *data)
{
	uint32_t len = 0;
	
	if(UART_RXIndex != 0)
	{
		NVIC_DisableIRQ(IRQ0_IRQ);		//��UART_RXBuffer��ȡ���ݹ�����Ҫ�ر��жϣ���ֹ��д����
		memcpy(data, UART_RXBuffer, UART_RX_LEN);
		len = UART_RXIndex;
		UART_RXIndex = 0;
		NVIC_EnableIRQ(IRQ0_IRQ);
	}
	
	return len;
}

void IRQ0_Handler(void)
{
	uint32_t chr;
	
	if(UART_INTRXThresholdStat(UART0) || UART_INTTimeoutStat(UART0))
	{
		while(UART_IsRXFIFOEmpty(UART0) == 0)
		{
			if(UART_ReadByte(UART0, &chr) == 0)
			{
				if(UART_RXIndex < UART_RX_LEN)
				{
					UART_RXBuffer[UART_RXIndex] = chr;
					
					UART_RXIndex++;
				}
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
