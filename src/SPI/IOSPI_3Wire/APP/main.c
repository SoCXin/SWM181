#include "SWM181.h"

#include "IOSPI_MST.h"

void SerialInit(void);

int main(void)
{	
	uint32_t i, j;
	
	SystemInit();
	
	SerialInit();
	
	IOSPI_MST_Init(20000);
	
	while(1==1)
	{
		for(i = 0; i < 8; i++)
		{
			for(j = 0; j < 16; j++)
			{
				IOSPI_MST_Write(1 << i, 1 << j);
				
				while(IOSPI_Done == 0) __NOP();
				IOSPI_Done = 0;
			}
		}
		
		IOSPI_MST_Read(0x33);
		
		while(IOSPI_Done == 0) __NOP();
		IOSPI_Done = 0;
		printf("Received: 0x%04X\n", IOSPI_Data);
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
