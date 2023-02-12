/****************************************************************************************************************************************** 
* �ļ�����:	IOSPI_MST.c
* ����˵��:	GPIO����ģ�ⵥ��SPI����
* ��������:	
* ע������:	
*******************************************************************************************************************************************/ 
#include "SWM181.h"

#include "IOSPI_MST.h"

volatile uint16_t IOSPI_Done = 0;
volatile uint16_t IOSPI_Data = 0;

static uint16_t IOSPI_Cmd_Read = 0;
static uint32_t IOSPI_Buf_Write = 0x00;

/****************************************************************************************************************************************** 
* ��������:	IOSPI_MST_Init()
* ����˵��: IOSPI������ʼ��
* ��    ��: uint32_t baud		IOSPI���������ʣ���λ�������ʣ���λbps
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void IOSPI_MST_Init(uint32_t freq)
{
	GPIO_Init(GPIOB, PIN2, 1, 0, 0, 0);					//GPIOB.2 -> IOSPI.SSEL
#define SPI_SSEL_LOW()		GPIO_ClrBit(GPIOB, PIN2)
#define SPI_SSEL_HIGH()		GPIO_SetBit(GPIOB, PIN2)	
	SPI_SSEL_HIGH();
	
	GPIO_Init(GPIOB, PIN1, 1, 0, 0, 0);					//GPIOB.1 -> IOSPI.SCLK
#define SPI_SCLK_LOW()		GPIO_ClrBit(GPIOB, PIN1)	
#define SPI_SCLK_HIGH()		GPIO_SetBit(GPIOB, PIN1)
	SPI_SCLK_LOW();
	
	GPIO_Init(GPIOB, PIN0, 1, 1, 0, 1);					//GPIOB.0 -> IOSPI.MOSI, ��©�����������0xFFFFʱ�ɶ�ȡRF IC�����
#define SPI_MOSI_LOW()		GPIO_ClrBit(GPIOB, PIN0)	
#define SPI_MOSI_HIGH()		GPIO_SetBit(GPIOB, PIN0)
#define SPI_MOSI_Value()	GPIO_GetBit(GPIOB, PIN0)
	
	TIMR_Init(TIMR0, TIMR_MODE_TIMER, SystemCoreClock/(2 * freq), 1);
	
	IRQ_Connect(IRQ0_15_TIMR0, IRQ5_IRQ, 1);		//��ʱ���ж����ӵ�IRQ5�ж��ߣ������ȼ�
}

void IOSPI_MST_Write(uint8_t addr, uint16_t data)
{
	IOSPI_Cmd_Read = 0;
	
	IOSPI_Buf_Write = ((addr << 16) | data) << 8;
	
	if(IOSPI_Buf_Write & 0x80000000) SPI_MOSI_HIGH();
	else                             SPI_MOSI_LOW();
	IOSPI_Buf_Write <<= 1;
	
	SPI_SSEL_LOW();
	TIMR_Start(TIMR0);
}

void IOSPI_MST_Read(uint8_t addr)
{
	IOSPI_Cmd_Read = 1;
	
	IOSPI_Data = 0;
	
	IOSPI_Buf_Write = ((addr << 16) | 0xFFFF) << 8;
	
	if(IOSPI_Buf_Write & 0x80000000) SPI_MOSI_HIGH();
	else                             SPI_MOSI_LOW();
	IOSPI_Buf_Write <<= 1;
	
	SPI_SSEL_LOW();
	TIMR_Start(TIMR0);
}


void IRQ5_Handler(void)
{
	static uint32_t bit = 24;
	static uint32_t edge_fall = 0;	// ʱ���½���
	
	if(TIMR_INTStat(TIMR0))
	{
		TIMR_INTClr(TIMR0);
		
		if(edge_fall)
		{
			edge_fall = 0;
			
			if(--bit == 0)
			{
				TIMR_Stop(TIMR0);
				
				IOSPI_Done = 1;
				
				bit = 24;
			}
			
			if(IOSPI_Buf_Write & 0x80000000) SPI_MOSI_HIGH();
			else                             SPI_MOSI_LOW();
			IOSPI_Buf_Write <<= 1;
			
			SPI_SCLK_LOW();
			
			if(IOSPI_Done) SPI_SSEL_HIGH();
		}
		else
		{
			edge_fall = 1;
			
			SPI_SCLK_HIGH();
			
			if(IOSPI_Cmd_Read && (bit < 17))
			{
				IOSPI_Data <<= 1;
				IOSPI_Data |= SPI_MOSI_Value();
			}
		}
	}
}
