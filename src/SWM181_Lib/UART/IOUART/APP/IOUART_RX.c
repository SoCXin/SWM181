/****************************************************************************************************************************************** 
* �ļ�����:	IOUART_RX.c
* ����˵��:	GPIO����ģ��UART���չ���
* ע������: ������9600
* �汾����: 
* ������¼: 
*******************************************************************************************************************************************/ 
#include "SWM181.h"

#include "IOUART_RX.h"


#define IOUART_STOP_1		0
#define IOUART_STOP_2		1
#define IOUART_STOP			IOUART_STOP_2

#define IOUART_PARITY_NONE	0
#define IOUART_PARITY_EVEN	1
#define IOUART_PARITY_ODD	2
#define IOUART_PARITY		IOUART_PARITY_EVEN


static char *RxBuff = 0;
static uint32_t ChrCnt = 0;
static volatile uint32_t ChrIdx = 0;
static volatile uint32_t BitIdx = 0;

static volatile uint32_t RxStat = IOUART_RX_IDLE;

static uint32_t TIM_BAUD_9600 = 0;			//������9600��Ӧ��ʱʱ��
static uint32_t TIM_BAUD_9600_X_1_5 = 0;	//������9600��Ӧ��ʱʱ����1.5��
											//����ʼλ�½���֮���1.5���ַ����ں�������������Բɵ�����λ���м�λ��

static volatile uint32_t Timeout_Cnt = 0;	//��ʱ������

/****************************************************************************************************************************************** 
* ��������:	IOUART_RX_Init()
* ����˵��: IOUART_RX��ʼ��
* ��    ��: ��
* ��    ��: ��
* ע������: ������9600
******************************************************************************************************************************************/
void IOUART_RX_Init(void)
{
	GPIO_Init(GPIOA, PIN5, 0, 1, 0, 0);			//GPIOA.5��ʼ��Ϊ���룬����
#define UART_RX_Value()		GPIO_GetBit(GPIOA, PIN5)
	
	GPIO_Init(GPIOA, PIN6, 1, 0, 0, 0);			//Sampling Point Indicate��������
#define UART_RXI_INV()	GPIO_InvBit(GPIOA, PIN6)
	
	TIM_BAUD_9600       = SystemCoreClock/9600;
	TIM_BAUD_9600_X_1_5 = SystemCoreClock/9600*1.5 * 0.85;
	
	TIMR_Init(TIMR3, TIMR_MODE_TIMER, SystemCoreClock/9600, 1);
	
	IRQ_Connect(IRQ0_15_TIMR3, IRQ9_IRQ, 1);		//��ʱ��3�ж����ӵ�IRQ9�ж��ߣ������ȼ�
	
	EXTI_Init(GPIOA, PIN5, EXTI_FALL_EDGE);		//�½��ش����ж�
	
	IRQ_Connect(IRQ0_15_GPIOA5, IRQ5_IRQ, 1);	//A5�����ⲿ�ж����ӵ�IRQ5�ж��ߣ������ȼ�
	
	EXTI_Open(GPIOA, PIN5);
}

/****************************************************************************************************************************************** 
* ��������:	IOUART_RX_RecvChars()
* ����˵��: IOUART����ָ�������ַ�
* ��    ��: char buff[]		���յ����ַ���˴�
*			uint32_t size	Ҫ���յ��ַ��ĸ���
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void IOUART_RX_RecvChars(char buff[], uint32_t size)
{
	RxBuff = buff;
	ChrCnt = size;
	
	ChrIdx = 0;
	RxStat = IOUART_RX_IDLE;
	
	EXTI_Clear(GPIOA, PIN5);
	EXTI_Open(GPIOA, PIN5);
}


/****************************************************************************************************************************************** 
* ��������:	IOUART_RX_State()
* ����˵��: IOUART����ģ�鵱ǰ״̬
* ��    ��: ��
* ��    ��: uint32_t		IOUART_RX_IDLE��IOUART_RX_BUSY��IOUART_RX_SUCCESS��UOUART_RX_TIMEOUT��IOUART_RX_FRAMERR
* ע������: ��
******************************************************************************************************************************************/
uint32_t IOUART_RX_State(void)
{
	return RxStat;
}

/****************************************************************************************************************************************** 
* ��������:	IOUART_RX_Count()
* ����˵��: IOUART����ģ����յ����ַ���
* ��    ��: ��
* ��    ��: uint32_t		���յ����ַ�����
* ע������: ��
******************************************************************************************************************************************/
uint32_t IOUART_RX_Count(void)
{
	return ChrIdx;
}

/****************************************************************************************************************************************** 
* ��������:	IOUART_RX_Clear()
* ����˵��: IOUART����ģ��״̬���
* ��    ��: ��
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void IOUART_RX_Clear(void)
{
	ChrIdx = 0;
	
	RxStat = IOUART_RX_IDLE;
}

/****************************************************************************************************************************************** 
* ��������:	IRQ5_Handler()
* ����˵��: IOUART����ģ�������½����жϷ���������⵽�½��غ�ر��½����жϡ�������ʱ��
* ��    ��: ��
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void IRQ5_Handler(void)
{
	EXTI_Clear(GPIOA, PIN5);
	
	if(RxStat == IOUART_RX_IDLE)
	{
		BitIdx = 0;
		RxStat = IOUART_RX_BUSY;
		
		TIMR_Stop(TIMR3);
		TIMR_SetPeriod(TIMR3, TIM_BAUD_9600_X_1_5);
		TIMR_Start(TIMR3);
	}
}

/****************************************************************************************************************************************** 
* ��������:	IRQ9_Handler()
* ����˵��: TIMR3�жϷ���������IOUART����ģ���λ����
* ��    ��: ��
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void IRQ9_Handler(void)
{
	static char ChrShf;		//������λ�Ĵ���
	const uint8_t evenOf1[16] = {1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1};	//����0--15��Ӧ�Ƿ���ż����'1'
	
	if(TIMR_INTStat(TIMR3))
	{
		TIMR_INTClr(TIMR3);
		
		if(RxStat == IOUART_RX_BUSY)
		{	
			UART_RXI_INV();
			
			if(BitIdx == 0)
			{
				TIMR_Stop(TIMR3);
				TIMR_SetPeriod(TIMR3, TIM_BAUD_9600);
				TIMR_Start(TIMR3);
			}
			
			if(BitIdx < 8)
			{
				if(UART_RX_Value()) ChrShf |=  (1 << BitIdx);
				else				ChrShf &= ~(1 << BitIdx);
			}
#if(IOUART_PARITY == IOUART_PARITY_NONE)
	#if(IOUART_STOP == IOUART_STOP_1)
			else if(BitIdx == 8)
	#else //IOUART_STOP == IOUART_STOP_2
			else if(BitIdx == 8)
			{
				if(UART_RX_Value() == 0)
				{
					RxStat = IOUART_RX_FRAMERR;
					
					EXTI_Close(GPIOA, PIN5);
				}
			}
			else if(BitIdx == 9)
	#endif
#else
			else if(BitIdx == 8)
			{
	#if(IOUART_PARITY == IOUART_PARITY_EVEN)
				if(((evenOf1[ChrShf & 0xF] == 1) && (evenOf1[(ChrShf >> 4) & 0xF] == 1)) || 
				   ((evenOf1[ChrShf & 0xF] == 0) && (evenOf1[(ChrShf >> 4) & 0xF] == 0)))	// ż����1
	#else //IOUART_PARITY == IOUART_PARITY_ODD
				if(((evenOf1[ChrShf & 0xF] == 0) && (evenOf1[(ChrShf >> 4) & 0xF] == 1)) || 
				   ((evenOf1[ChrShf & 0xF] == 1) && (evenOf1[(ChrShf >> 4) & 0xF] == 0)))	// ������1
	#endif
				{
					if(UART_RX_Value() == 1)
					{
						RxStat = IOUART_RX_PARITYERR;
						
						EXTI_Close(GPIOA, PIN5);
					}
				}
				else
				{
					if(UART_RX_Value() == 0)
					{
						RxStat = IOUART_RX_PARITYERR;
						
						EXTI_Close(GPIOA, PIN5);
					}
				}
			}
	#if(IOUART_STOP == IOUART_STOP_1)
			else if(BitIdx == 9)
	#else //IOUART_STOP == IOUART_STOP_2
			else if(BitIdx == 9)
			{
				if(UART_RX_Value() == 0)
				{
					RxStat = IOUART_RX_FRAMERR;
					
					EXTI_Close(GPIOA, PIN5);
				}
			}
			else if(BitIdx == 10)
	#endif
#endif
			{
				RxStat = IOUART_RX_IDLE;
				
				RxBuff[ChrIdx] = ChrShf;
				
				if(UART_RX_Value() == 1)
				{
					ChrIdx++;
					
					if(ChrIdx == ChrCnt)
					{
						RxStat = IOUART_RX_BUFFULL;
						
						EXTI_Close(GPIOA, PIN5);
					}
				}
				else
				{
					RxStat = IOUART_RX_FRAMERR;
					
					EXTI_Close(GPIOA, PIN5);
				}
			}
			
			BitIdx++;
			
			UART_RXI_INV();
			
			Timeout_Cnt = 0;
		}
		else if((RxStat == IOUART_RX_IDLE) && (ChrIdx != 0))
		{
			Timeout_Cnt++;
			if(Timeout_Cnt == 20)	//2���ַ�ʱ��
			{
				RxStat = IOUART_RX_TIMEOUT;
				
				EXTI_Close(GPIOA, PIN5);
				
				TIMR_Stop(TIMR3);
				
				Timeout_Cnt = 0;
			}
		}
	}
}
