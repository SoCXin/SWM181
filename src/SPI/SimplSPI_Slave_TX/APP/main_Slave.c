#include "SWM181.h"

uint16_t *SPI1TXBuff = 0;
uint32_t SPI1TXCount = 0;
uint32_t SPI1TXIndex = 0;

void SerialInit(void);
void SPISlvInit(void);
void SPISlvSend(uint16_t buff[], uint32_t cnt);

int main(void)
{	
	uint16_t txbuff[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
	
	SystemInit();
	
	SerialInit();	
	
	SPISlvInit();
	
	SPISlvSend(txbuff, 16);

	GPIO_Init(GPIOA, PIN5, 1, 0, 0, 0);	// �жϺ�������ָʾ

	while(1==1)
	{
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
	SPI_initStruct.RXHFullIEn = 0;
	SPI_initStruct.TXEmptyIEn = 0;
	SPI_initStruct.TXCompleteIEn = 0;
	SPI_Init(SPI1, &SPI_initStruct);
	
	SPI_Open(SPI1);
	
	SPI_INTTXEmptyEn(SPI1);
	SPI_INTTXWordCompleteEn(SPI1);
	IRQ_Connect(IRQ0_15_SPI1, IRQ4_IRQ, 2);
	
	
	/* ��PC4�������������ڼ��SPI�ӻ���CS Assert�¼� */
	GPIO_Init(GPIOA, PIN4, 0, 1, 0, 0);			//���룬����ʹ��
	
	EXTI_Init(GPIOA, PIN4, EXTI_FALL_EDGE);		//�½��ش����ж�
	
	IRQ_Connect(IRQ0_15_GPIOA4, IRQ5_IRQ, 2);
	
	EXTI_Open(GPIOA, PIN4);
}

void SPISlvSend(uint16_t buff[], uint32_t cnt)
{
	SPI1TXBuff = buff;
	SPI1TXCount = cnt;
}

void IRQ4_Handler(void)
{
	if((SPI1->IF & SPI_IF_TFE_Msk) || (SPI1->IF & SPI_IF_WTC_Msk))
	{
		while(SPI1->STAT & SPI_STAT_TFNF_Msk)
		{
			if(SPI1TXIndex < SPI1TXCount)
			{
				SPI1->DATA = SPI1TXBuff[SPI1TXIndex++];
			}
			else
			{
				SPI1->DATA = 0xFF;	// û�����ˣ�����0xFF
			}
		}
		
		SPI1->IF = (1 << SPI_IF_TFE_Pos) | (1 << SPI_IF_WTC_Pos);
	}
}

void IRQ5_Handler(void)
{	
	EXTI_Clear(GPIOA, PIN4);
	
	SPI1TXIndex = 0;
	
	SPI1->CTRL |=  (1 << SPI_CTRL_TFCLR_Pos);	// ����FIFO���
	SPI1->CTRL &= ~(1 << SPI_CTRL_TFCLR_Pos);
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
