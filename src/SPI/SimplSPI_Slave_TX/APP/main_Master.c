#include "SWM181.h"

void SerialInit(void);
void SPIMstInit(void);

int main(void)
{	
	uint32_t i, j;
	uint16_t rxbuff[16] = {0};
	
	SystemInit();
	
	SerialInit();	
	
	SPIMstInit();			// SPI0与SPI1对应引脚相连
	
	while(1==1)
	{
		GPIO_ClrBit(GPIOA, PIN12);	// SPI_CS_Low()
		for(i = 0; i < 240; i++);	// CS拉低后需要延时一下再发送
		for(i = 0, j = 0; i < 16; i++)
		{
			SPI0->DATA = 0xFF;
			while((SPI0->STAT & SPI_STAT_TFNF_Msk) == 0);
			
			if(SPI0->STAT & SPI_STAT_RFNE_Msk) rxbuff[j++] = SPI0->DATA;
		}
		while(j < 16)
		{
			if(SPI0->STAT & SPI_STAT_RFNE_Msk) rxbuff[j++] = SPI0->DATA;
		}
		GPIO_SetBit(GPIOA, PIN12);	// SPI_CS_High()
		
		for(i = 0; i < 16; i++) printf("%d, ", rxbuff[i]);
		printf("\r\n\r\n");
		
		for(i = 0; i < SystemCoreClock/10; i++);
	}
}


void SPIMstInit(void)
{
	SPI_InitStructure SPI_initStruct;
	
// 	PORT_Init(PORTA, PIN12, PORTA_PIN12_SPI0_SSEL, 0);
	GPIO_Init(GPIOA, PIN12, 1, 0, 0, 0);				//软件控制片选
#define SPI_CS_Low()	GPIO_ClrBit(GPIOA, PIN12)
#define SPI_CS_High()	GPIO_SetBit(GPIOA, PIN12)
	SPI_CS_High();
	
	PORT_Init(PORTA, PIN13, PORTA_PIN13_SPI0_MISO, 1);
	PORT_Init(PORTA, PIN14, PORTA_PIN14_SPI0_MOSI, 0);
	PORT_Init(PORTA, PIN15, PORTA_PIN15_SPI0_SCLK, 0);
	
	SPI_initStruct.clkDiv = SPI_CLKDIV_8;
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


void SerialInit(void)
{
	UART_InitStructure UART_initStruct;
	
	PORT_Init(PORTA, PIN0, FUNMUX_UART0_RXD, 1);	//GPIOA.0配置为UART0输入引脚
	PORT_Init(PORTA, PIN1, FUNMUX_UART0_TXD, 0);	//GPIOA.1配置为UART0输出引脚
 	
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
* 函数名称: fputc()
* 功能说明: printf()使用此函数完成实际的串口打印动作
* 输    入: int ch		要打印的字符
*			FILE *f		文件句柄
* 输    出: 无
* 注意事项: 无
******************************************************************************************************************************************/
int fputc(int ch, FILE *f)
{
	UART_WriteByte(UART0, ch);
	
	while(UART_IsTXBusy(UART0));
 	
	return ch;
}
