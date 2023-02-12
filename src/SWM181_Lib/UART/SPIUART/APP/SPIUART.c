/****************************************************************************************************************************************** 
* �ļ�����:	SPIUART.c
* ����˵��:	SPIģ��UART����˫����֧����żУ��
* ����֧��:	
* ע������: 180��SPIʹ�û�����λ�Ĵ���ʵ�֣���SI��һλΪ0����SO�������һλʱ��ʱ�Ӳ�����֮��ܿ�SO��ƽ����0
*			�����������˫��ͨ�ţ�����ʱSI���ſ϶��Ǹߣ������⣻����ʱ��SO�����г�GPIO����ߵ�ƽ��SO�ϵĵͲ��������������
* �汾����:
* ������¼: 2017/11/21 �Է���������ʱ��ⲻ���ǵ�һ�ֽڵ���ʼλ�½���
*					   ���������SPI ֻ����10λ�����ٽ��պͼ���11λ��ֹͣλ��
*
******************************************************************************************************************************************/
#include "SWM181.h"

#include "SPIUART.h"


#define IOUART_PARITY_EVEN	1
#define IOUART_PARITY_ODD	2
#define IOUART_PARITY		IOUART_PARITY_EVEN


volatile uint16_t SPIUART_SendComplete = 0;


#define SPI_TX_LEN	64
static uint16_t SPI_TX_Buff[SPI_TX_LEN] = {0};
static uint16_t SPI_TX_Count;
static uint16_t SPI_TX_Index;

#define SPI_RX_LEN	64
static uint16_t SPI_RX_Buff[SPI_RX_LEN] = {0};
static uint16_t SPI_RX_Index;


/****************************************************************************************************************************************** 
* ��������:	SPIUART_Init()
* ����˵��:	SPIģ��UART��ʼ��
* ��    ��: uint16_t baudrate		���ڲ�����
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void SPIUART_Init(uint16_t baudrate)
{
	SPI_InitStructure SPI_initStruct;
	PWM_InitStructure PWM_initStruct;
	
	GPIO_Init(GPIOA, PIN7, 1, 0, 0, 0);				//ģ��SPI������nCS���ţ���Ҫ��SPI�ӻ���nCS����
	GPIO_SetBit(GPIOA, PIN7);
	
	PORT_Init(PORTA, PIN8,  PORTA_PIN8_SPI0_SSEL,  1);
	
	PORT_Init(PORTA, PIN9,  PORTA_PIN9_SPI0_MISO,  0);
	GPIO_Init(GPIOA, PIN9,  1, 0, 0, 0);			//�������г�GPIOʱ����Ϊ��������ֵΪ�ߵ�ƽ
	GPIO_SetBit(GPIOA, PIN9);
#define PA9_AsGPIO()  PORTG->PORTA_SEL2 &= ~(0x03 << ((PIN9-8)*2));
#define PA9_AsMISO()  PORTG->PORTA_SEL2 |= PORTA_PIN9_SPI0_MISO << ((PIN9-8)*2);
	
	PORT_Init(PORTA, PIN10, PORTA_PIN10_SPI0_MOSI, 1);
	GPIO_Init(GPIOA, PIN10, 0, 1, 0, 0);			//�������г�GPIOʱ����Ϊ���롢ʹ�������������жϼ����ʼλ���½���
#define PA10_AsGPIO() PORTG->PORTA_SEL2 &= ~(0x03 << ((PIN10-8)*2));
#define PA10_AsMOSI() PORTG->PORTA_SEL2 |= PORTA_PIN10_SPI0_MOSI << ((PIN10-8)*2);
	
	EXTI_Init(GPIOA, PIN10, EXTI_FALL_EDGE);			//�½��ش����ж�
	IRQ_Connect(IRQ0_15_GPIOA10, IRQ9_IRQ, 1);			//�����ȼ�
	
	PORT_Init(PORTA, PIN11, PORTA_PIN11_SPI0_SCLK, 1);
	
	
	SPI_initStruct.clkDiv = SPI_CLKDIV_512;
	SPI_initStruct.FrameFormat = SPI_FORMAT_SPI;
	SPI_initStruct.SampleEdge = SPI_SECOND_EDGE;
	SPI_initStruct.IdleLevel = SPI_HIGH_LEVEL;
	SPI_initStruct.WordSize = 11;
	SPI_initStruct.Master = 0;
	SPI_initStruct.RXHFullIEn = 0;
	SPI_initStruct.TXHFullIEn = 0;
	SPI_Init(SPI0, &SPI_initStruct);
	
	IRQ_Connect(IRQ0_15_SPI0, IRQ8_IRQ, 1);
	
	SPI_Open(SPI0);
	
	
	PWM_initStruct.mode = PWM_MODE_INDEP;			//A·��B·�������
	PWM_initStruct.clk_div = PWM_CLKDIV_1;
	PWM_initStruct.cycleA = SystemCoreClock/baudrate;		
	PWM_initStruct.hdutyA = SystemCoreClock/baudrate/2;
	PWM_initStruct.initLevelA = 0;
	PWM_initStruct.HEndAIEn = 0;
	PWM_initStruct.NCycleAIEn = 0;
	PWM_initStruct.HEndBIEn = 0;
	PWM_initStruct.NCycleBIEn = 0;
	
	PORT_Init(PORTA, PIN4, FUNMUX_PWM1A_OUT, 0);	//ģ��SPI������CLK���ţ�������SPI�ӻ���CLK����
	
	PWM_Init(PWM1, &PWM_initStruct);

	
#define SPI_MASTER_CLK_GEN()	__disable_irq(); SPI_Open(SPI0); GPIO_ClrBit(GPIOA, PIN7); PWM_Start(PWM1, 1, 0); __enable_irq()
#define SPI_MASTER_CLK_STOP()	__disable_irq(); PWM_Stop(PWM1, 1, 1); GPIO_SetBit(GPIOA, PIN7); SPI_Close(SPI0); __enable_irq()
}


static void _SPIUART_Send(uint16_t cnt);
/****************************************************************************************************************************************** 
* ��������:	SPIUART_Send()
* ����˵��:	SPIģ��UART���ݷ���
* ��    ��: uint8_t buff[]		Ҫ���͵����ݣ�8λ
*			uint16_t cnt		Ҫ�������ݵĸ���
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void SPIUART_Send(uint8_t buff[], uint16_t cnt)
{
	EXTI_Close(GPIOA, PIN10);
	
	PA10_AsMOSI();
	
	SPIUART_SendComplete = 0;
	
	UART2SPI_encode(buff, cnt);
	
	SPI0->CTRL &= ~SPI_CTRL_DSS_Msk;
	SPI0->CTRL |= (10 << SPI_CTRL_DSS_Pos);
	
	PA9_AsMISO();	//����ǰ�л�
	
	_SPIUART_Send(cnt);
}

static void _SPIUART_Send(uint16_t cnt)
{	
	SPI_TX_Count = cnt;
	SPI_TX_Index = 0;
	
	SPI_Write(SPI0, SPI_TX_Buff[SPI_TX_Index++]);
	
	SPI_MASTER_CLK_GEN();
	
	SPI_INTTXEmptyEn(SPI0);
}

/****************************************************************************************************************************************** 
* ��������:	SPIUART_Recv()
* ����˵��:	��ȡ���յ������ݣ��������ݸ��������ݵ�bit15��֡�����־��bit14��У������־
* ��    ��: uint16_t buff[]		��ȡ�������ݴ���buff
*			uint16_t min		�����ݸ������� min ���򲻶�ȡ���ݣ�ֱ�ӷ���0
* ��    ��: uint16_t 			���ݸ���
* ע������: buff����Ĵ�С���벻С��SPI_RX_LEN
******************************************************************************************************************************************/
uint16_t SPIUART_Recv(uint16_t buff[], uint16_t min)
{
	uint16_t len;
	
	if(SPI_RX_Index < min) return 0;
	
	SPI_INTTXEmptyDis(SPI0);
	memcpy(buff, SPI_RX_Buff, (len = SPI_RX_Index) * 2);
	SPI_RX_Index = 0;
	SPI_INTTXEmptyEn(SPI0);
	
	SPI2UART_decode(buff, len);
	
	return len;
}

void SPIUART_RecvPrepare(void)
{
	PA9_AsGPIO();	//����ʱSO��������ݲ����͵�������
	
	PA10_AsGPIO();
	
	SPI_RX_Index = 0;
	
	SPI0->CTRL &= ~SPI_CTRL_DSS_Msk;
	SPI0->CTRL |= (9 << SPI_CTRL_DSS_Pos);
	
	EXTI_Clear(GPIOA, PIN10);
	EXTI_Open(GPIOA, PIN10);
}

void IRQ8_Handler(void)
{		
	if(SPI_TX_Index == SPI_TX_Count + 1)
	{
		SPI_MASTER_CLK_STOP();
		
		PA9_AsGPIO();	//������ʱSO��������ǵͣ��г�GPIO�����
		
		PA10_AsGPIO();	//׼����Ӧ��һ��EXTI�ж�
		
		SPIUART_SendComplete = 1;
		
		SPI_INTTXEmptyDis(SPI0);
	}
	else if(SPI_TX_Index == SPI_TX_Count)
	{
		SPI_Write(SPI0, 0x7FF);
		
		SPI_TX_Index++;
	}
	else
	{
		while(SPI_IsTXFull(SPI0) == 0)
		{
			if(SPI_TX_Index < SPI_TX_Count)
			{
				SPI_Write(SPI0, SPI_TX_Buff[SPI_TX_Index++]);
			}
			else
			{
				break;
			}
		}
	}
	
	while(SPI_IsRXEmpty(SPI0) == 0)
	{
		volatile uint16_t tmp;
		
		if(SPI_RX_Index < SPI_RX_LEN)
			SPI_RX_Buff[SPI_RX_Index++] = SPI0->DATA;
		else
			tmp = SPI0->DATA;
	}
	
	SPI0->IF = SPI_IF_TFE_Msk;	//����жϱ�־�����������TX FIFO�����жϱ�־
}

void IRQ9_Handler(void)
{	
	EXTI_Clear(GPIOA, PIN10);
	
	PA10_AsMOSI();
	
	SPI_TX_Buff[0] = 0x7FF;
	_SPIUART_Send(1);
}


/********************************************************************************************************
                                            �ڲ�����
********************************************************************************************************/

const uint8_t InvCode[16] = {0x0, 0x8, 0x4, 0xC, 0x2, 0xA, 0x6, 				//UART�ȷ���LSB��SPI�ȷ���MSB����Ҫ��תλ��
							 0xE, 0x1, 0x9, 0x5, 0xD, 0x3, 0xB, 0x7, 0xF};		//����0--15��Ӧ��λ��ת��ֵ
const uint8_t EvenOf1[16] = {1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1};	//����0--15��Ӧ�Ƿ���ż����'1' 

void UART2SPI_encode(uint8_t buff[], uint16_t cnt)
{
	uint16_t i, code;
	
	for(i = 0; i < cnt; i++)
	{
		code = (0 << 10) | 
		       (((InvCode[buff[i] & 0xF] << 4) | InvCode[(buff[i] >> 4) & 0xF]) << 2) |
			   ((
	#if IOUART_PARITY == IOUART_PARITY_EVEN
				 (((EvenOf1[buff[i] & 0xF] == 1) && (EvenOf1[(buff[i] >> 4) & 0xF] == 1)) || 
				  ((EvenOf1[buff[i] & 0xF] == 0) && (EvenOf1[(buff[i] >> 4) & 0xF] == 0)))	/* 	ż����1 */
	#else //IOUART_PARITY == IOUART_PARITY_ODD
				 (((EvenOf1[buff[i] & 0xF] == 0) && (EvenOf1[(buff[i] >> 4) & 0xF] == 1)) || 
				  ((EvenOf1[buff[i] & 0xF] == 1) && (EvenOf1[(buff[i] >> 4) & 0xF] == 0)))	/* 	������1 */
	#endif
				 ? 0 : 1) << 1) |
		       (1 << 0);					//��ʼλ0������λ��У��λ��ֹͣλ1
		
		SPI_TX_Buff[i] = code;
	}
}

void SPI2UART_decode(uint16_t buff[], uint16_t cnt)
{
	uint16_t i, code;
		
	for(i = 0; i < cnt; i++)
	{
		code = (InvCode[(buff[i] >> 1) & 0xF] << 4) | InvCode[(buff[i] >> 5) & 0xF];
	
	#if IOUART_PARITY == IOUART_PARITY_EVEN
		if(((EvenOf1[(buff[i] >> 1) & 0xF] == 1) && (EvenOf1[(buff[i] >> 5) & 0xF] == 1)) || 
		   ((EvenOf1[(buff[i] >> 1) & 0xF] == 0) && (EvenOf1[(buff[i] >> 5) & 0xF] == 0)))	/* 	ż����1 */
	#else //IOUART_PARITY == IOUART_PARITY_ODD
		if(((EvenOf1[(buff[i] >> 1) & 0xF] == 0) && (EvenOf1[(buff[i] >> 5) & 0xF] == 1)) || 
		   ((EvenOf1[(buff[i] >> 1) & 0xF] == 1) && (EvenOf1[(buff[i] >> 5) & 0xF] == 0)))	/* 	������1 */
	#endif
		{
			if(((buff[i] >> 0) & 1) != 0) code |= SPIUART_PARITY_ERR_MASK;	//У�����
		}
		
//		if(((buff[i] >> 0) & 1) != 1) 	  code |= SPIUART_FRAME_ERR_MASK;	//֡���󣬼�ֹͣλ��Ϊ1
		
		buff[i] = code;
	}
}
