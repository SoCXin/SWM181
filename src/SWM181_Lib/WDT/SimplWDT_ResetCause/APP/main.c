#include "SWM181.h"

void SerialInit(void);

int main(void)
{
	uint32_t i;

	SystemInit();
	
	SerialInit();
	
	for(i=0;i<2400000;i++);							//����SWD�޷�����
	
	if(SYS->RSTSR & SYS_RSTSR_POR_Msk)		//ע�⣺��������߻���ʱ����λҲ������
	{
		SYS->RSTSR = SYS_RSTSR_POR_Msk;
		printf("POR Reset\r\n");
	}
	if(SYS->RSTSR & SYS_RSTSR_WDT_Msk)
	{
		SYS->RSTSR = SYS_RSTSR_WDT_Msk;
		printf("WDT Reset\r\n");
	}
	
	WDT_Init(WDT, SystemCoreClock, WDT_MODE_RESET);	//ÿ1���Ӹ�λ1��
	WDT_Start(WDT);
	
	while(1==1)
	{
		
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
