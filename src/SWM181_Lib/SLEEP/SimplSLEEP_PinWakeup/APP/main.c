#include "SWM181.h"


int main(void)
{	
	uint32_t i;
	
	SystemInit();
	
	GPIO_Init(GPIOA, PIN4, 1, 0, 0, 0);				//�� LED��ָʾ����ִ��״̬
	GPIO_SetBit(GPIOA, PIN4);
	
	for(i = 0; i < SystemCoreClock; i++);			//��ʱ����ֹ�ϵ��SWD�����е��޷����س���
	
	GPIO_Init(GPIOA, PIN1, 0, 1, 0, 0);				//�Ӱ���������ʹ��
	SYS->PAWKEN |= (1 << PIN1);						//����PA1���ŵ͵�ƽ����
	
													//�ر����ѣ�ִ��������һ���SWD�������ŵ���������͹ر��ˣ�������������������
	PORTA->INEN = (1 << PIN1);						//����˯��ǰ��ر��������ŵ��������빦�ܣ��Խ���˯�߹���
	PORTB->INEN = 0;								//ע�ⲻҪ�ر�PA1���������빦�ܣ���Ϊ�����ŵ�ƽ�仯��������
	PORTC->INEN = 0;
	PORTD->INEN = 0;
	
	while(1==1)
	{
		GPIO_SetBit(GPIOA, PIN4);					//����LED
		for(i = 0; i < SystemCoreClock/5; i++);
		GPIO_ClrBit(GPIOA, PIN4);					//Ϩ��LED
		
		EnterSleepMode();							//����˯��ģʽ
		
		while((SYS->PAWKSR & (1 << PIN1)) == 0);	//�ȴ���������
		SYS->PAWKSR |= (1 << PIN1);					//�������״̬
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
