#include "SWM181.h"

#include <string.h>

uint16_t SPI1RXBuffer[32] = {0};
uint32_t SPI1RXIndex = 0;
volatile uint32_t SPI1RXFinish = 0;

void SerialInit(void);
void SPISlvInit(void);

int main(void)
{	
	uint32_t i;
	
	SystemInit();
	
	SerialInit();	
	
	SPISlvInit();
	
	while(1==1)
	{
		while(SPI1RXFinish == 0) __NOP();
		SPI1RXFinish = 0;
		
		for(i = 0; i < 16; i++) printf("%d, ", SPI1RXBuffer[i]);
		printf("\r\n\r\n");
		
		SPI1RXIndex = 0;
		memset(SPI1RXBuffer, 0, 64);
	}
}


void SPISlvInit(void)
{
	SPI_InitStructure SPI_initStruct;
	
	PORT_Init(PORTC, PIN4, PORTC_PIN4_SPI1_SSEL, 1);
	PORT_Init(PORTC, PIN5, PORTC_PIN5_SPI1_MISO, 0);
	PORT_Init(PORTC, PIN6, PORTC_PIN6_SPI1_MOSI, 1);
	PORT_Init(PORTC, PIN7, PORTC_PIN7_SPI1_SCLK, 1);
	
	SPI_initStruct.FrameFormat = SPI_FORMAT_SPI;
	SPI_initStruct.SampleEdge = SPI_SECOND_EDGE;
	SPI_initStruct.IdleLevel = SPI_HIGH_LEVEL;
	SPI_initStruct.WordSize = 8;
	SPI_initStruct.Master = 0;
	SPI_initStruct.RXHFullIEn = 1;
	SPI_initStruct.TXEmptyIEn = 0;
	SPI_initStruct.TXCompleteIEn = 0;
	SPI_Init(SPI1, &SPI_initStruct);
	
	IRQ_Connect(IRQ0_15_SPI1, IRQ4_IRQ, 2);
	
	SPI_Open(SPI1);
	
	/* ��PC4�������������ڼ��SPI�ӻ���CS Deassert�¼� */
	GPIO_Init(GPIOA, PIN4, 0, 1, 0, 0);			//���룬����ʹ��
	
	EXTI_Init(GPIOA, PIN4, EXTI_RISE_EDGE);		//�����ش����ж�
	
	IRQ_Connect(IRQ0_15_GPIOA4, IRQ5_IRQ, 2);
	
	EXTI_Open(GPIOA, PIN4);
}

void IRQ4_Handler(void)
{
	if(SPI1->IF & SPI_IF_RFHF_Msk)
	{
		while(SPI1->STAT & SPI_STAT_RFNE_Msk)
		{
			SPI1RXBuffer[SPI1RXIndex++] = SPI1->DATA;
		}
		
		SPI1->IF = (1 << SPI_IF_RFHF_Pos);	// �ȶ������ݣ��������־
	}
}

void IRQ5_Handler(void)
{	
	EXTI_Clear(GPIOA, PIN4);
	
	while(SPI1->STAT & SPI_STAT_RFNE_Msk)
	{
		SPI1RXBuffer[SPI1RXIndex++] = SPI1->DATA;
	}
	
	SPI1RXFinish = 1;
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
