#include "SWM181.h"


void SerialInit(void);

int main(void)
{
	uint32_t input;
	
	SystemInit();
	
	SerialInit();
   	
	CORDIC_Init(CORDIC);
	
	input = CORDIC_PI/6*16384;
	if((input > 163) && (input < 25559))		//ֻ�ܼ���0.01~1.56֮��Ļ���
	{
		CORDIC_Sin(input);
		while(CORDIC_Sin_IsDone() == 0);
		printf("sin(30) = %f\r\n", CORDIC_Sin_Result()/16384.0);
	}
	
	input = CORDIC_PI/3*16384;
	if((input > 163) && (input < 25559))		//ֻ�ܼ���0.01~1.56֮��Ļ���
	{
		CORDIC_Cos(input);
		while(CORDIC_Cos_IsDone() == 0);
		printf("cos(60) = %f\r\n", CORDIC_Cos_Result()/16384.0);
	}
	
	input = 1*16384;
	if((input > 819) && (input < 10000*16384))	//ֻ�ܼ���0.05~10000֮���ֵ
	{
		CORDIC_Arctan(input);
		while(CORDIC_Arctan_IsDone() == 0);
		printf("arctan(1) = pi/%d\r\n", (int)(CORDIC_PI/(CORDIC_Arctan_Result()/16384.0)));
	}
	
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
