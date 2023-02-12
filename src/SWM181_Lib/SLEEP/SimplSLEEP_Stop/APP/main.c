#include "SWM181.h"

void SerialInit(void);

/* ������͹�������ģʽ��Stopģʽ����ֻ��ͨ��A0�����½��ػ��ѣ����Ѻ�����ͷ��ʼ����ִ�� */

int main(void)
{	
	uint32_t i, j;
	
	SystemInit();
	
	SerialInit();
// 	printf("Hi from Synwit\r\n");
	
	GPIO_Init(GPIOA, PIN4, 1, 0, 0, 0);			//�� LED��ָʾ����ִ��״̬
	GPIO_ClrBit(GPIOA, PIN4);
	for(i = 0; i < 4; i++)
	{
		GPIO_InvBit(GPIOA, PIN4);
		for(j = 0; j < 5000000; j++) __NOP();	//��Ҫע�͵�����ֹ�ϵ����������Stopģʽû��JLink���³���
	}
	
	EnterStopMode();
	
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
