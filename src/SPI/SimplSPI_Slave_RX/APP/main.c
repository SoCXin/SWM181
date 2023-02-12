#include "SWM181.h"

char SPI1RXBuffer[128] = {0};
volatile uint32_t SPI1RXIndex = 0;
volatile uint32_t SPI1RXFinish = 0;


void SerialInit(void);
void SPIMstInit(void);
void SPISlvInit(void);

int main(void)
{	
	SystemInit();
	
	SerialInit();
	
	SPIMstInit();
	
	SPISlvInit();
	
	TIMR_Init(TIMR0, TIMR_MODE_TIMER, SystemCoreClock/1000, 1);
	IRQ_Connect(IRQ0_15_TIMR0, IRQ3_IRQ, 2);
	TIMR_Start(TIMR0);
	
	while(1==1)
	{
		while(SPI1RXFinish == 0) __NOP();
		SPI1RXFinish = 0;
		SPI1RXIndex = 0;
		
		printf("%s\n\n", SPI1RXBuffer);
	}
}


void SPIMstInit(void)
{
	SPI_InitStructure SPI_initStruct;
	
// 	PORT_Init(PORTA, PIN12, PORTA_PIN12_SPI0_SSEL, 0);
	GPIO_Init(GPIOA, PIN12, 1, 0, 0, 0);				//�������Ƭѡ
#define SPI0_CS_Low()	GPIO_ClrBit(GPIOA, PIN12)
#define SPI0_CS_High()	GPIO_SetBit(GPIOA, PIN12)
	SPI0_CS_High();
	
	PORT_Init(PORTA, PIN13, PORTA_PIN13_SPI0_MISO, 1);
	PORT_Init(PORTA, PIN14, PORTA_PIN14_SPI0_MOSI, 0);
	PORT_Init(PORTA, PIN15, PORTA_PIN15_SPI0_SCLK, 0);
	
	SPI_initStruct.clkDiv = SPI_CLKDIV_32;
	SPI_initStruct.FrameFormat = SPI_FORMAT_SPI;
	SPI_initStruct.SampleEdge = SPI_SECOND_EDGE;
	SPI_initStruct.IdleLevel = SPI_HIGH_LEVEL;
	SPI_initStruct.WordSize = 8;
	SPI_initStruct.Master = 1;
	SPI_initStruct.RXHFullIEn = 0;
	SPI_initStruct.TXEmptyIEn = 0;
	SPI_initStruct.TXCompleteIEn = 0;
	SPI_Init(SPI0, &SPI_initStruct);
	
	SPI_Open(SPI0);
}


void IRQ3_Handler(void)
{
	static int counter = 0;
	static char txbuffer[64] = "Hello from Synwit!\r\n";
	
	TIMR_INTClr(TIMR0);
	
	counter += 1;
	
	if(counter == 1)
	{
		SPI0_CS_Low();
	}
	else if(counter < 50)
	{
		SPI0->DATA = txbuffer[counter - 2];
	}
	else if(counter == 50)
	{
		SPI0_CS_High();
	}
	else if(counter == 1000)
	{
		counter = 0;
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
