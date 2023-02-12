/****************************************************************************************************************************************** 
* �ļ�����: IOUART_TX.c
* ����˵��:	GPIO����ģ��UART���͹���
* ע������: ������9600
* �汾����: 
* ������¼: 
*******************************************************************************************************************************************/
#include "SWM181.h"

#include "IOUART_TX.h"

#define IOUART_STOP_1		0
#define IOUART_STOP_2		1
#define IOUART_STOP			IOUART_STOP_2

#define IOUART_PARITY_NONE	0
#define IOUART_PARITY_EVEN	1
#define IOUART_PARITY_ODD	2
#define IOUART_PARITY		IOUART_PARITY_EVEN


static char *TxBuff = 0;
static uint32_t ChrCnt = 0;
static volatile uint32_t ChrIdx = 0;
static volatile uint32_t BitIdx = 0;

static volatile uint32_t TxBusy = 0;

/****************************************************************************************************************************************** 
* ��������:	IOUART_TX_Init()
* ����˵��: IOUART_TX��ʼ��
* ��    ��: ��
* ��    ��: ��
* ע������: ������9600
******************************************************************************************************************************************/
void IOUART_TX_Init(void)
{	
	GPIO_Init(GPIOA, PIN4, 1, 0, 0, 0);			//GPIOA.4��ʼ��Ϊ���
#define UART_TX_LOW()	GPIO_ClrBit(GPIOA, PIN4)
#define UART_TX_HIGH()	GPIO_SetBit(GPIOA, PIN4)
	UART_TX_HIGH();								//UART����ʱ����ߵ�ƽ
	
	TIMR_Init(TIMR2, TIMR_MODE_TIMER, SystemCoreClock/9600, 1);
	
	IRQ_Connect(IRQ0_15_TIMR2, IRQ4_IRQ, 1);	//��ʱ��2�ж����ӵ�IRQ4�ж��ߣ������ȼ�
}

/****************************************************************************************************************************************** 
* ��������:	IOUART_TX_SendChars()
* ����˵��: IOUART����ָ�������ַ�
* ��    ��: char buff[]		Ҫ���͵��ַ�
*			uint32_t size	Ҫ���͵��ַ��ĸ���
* ��    ��: ��
* ע������: ֻ���� IOUART_TX_IsBusy() == 0 ʱ���ͣ�����֮ǰ���͵����ݿ��ܷ��Ͳ���ȫ
******************************************************************************************************************************************/
void IOUART_TX_SendChars(char buff[], uint32_t size)
{
	TxBuff = buff;
	ChrCnt = size;
	
	ChrIdx = 0;
	BitIdx = 0;
	TxBusy = 1;
	TIMR_Start(TIMR2);
}

/****************************************************************************************************************************************** 
* ��������:	IOUART_TX_IsBusy()
* ����˵��: IOUART�����ڷ���
* ��    ��: ��					
* ��    ��: uint32_t		1 IOUART��æ�ڷ���    0 IOUART��ǰ���У����Է����µķ���
* ע������: ��
******************************************************************************************************************************************/
uint32_t IOUART_TX_IsBusy(void)
{
	return TxBusy;
}

/****************************************************************************************************************************************** 
* ��������:	IRQ4_Handler()
* ����˵��: TIMR2�жϷ�����
* ��    ��: ��
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void IRQ4_Handler(void)
{
	const uint8_t evenOf1[16] = {1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1};	//����0--15��Ӧ�Ƿ���ż����'1'
	
	if(TIMR_INTStat(TIMR2))
	{
		TIMR_INTClr(TIMR2);
		
		if(TxBusy == 1)
		{
			if(BitIdx == 0)
			{
				UART_TX_LOW();				//��ʼλ
			}
			else if(BitIdx < 9)
			{
				if(TxBuff[ChrIdx] & (0x01 << (BitIdx-1)))
					UART_TX_HIGH();
				else
					UART_TX_LOW();
			}
#if(IOUART_PARITY == IOUART_PARITY_NONE)
	#if(IOUART_STOP == IOUART_STOP_1)
			else if(BitIdx == 9)
	#else //IOUART_STOP == IOUART_STOP_2
			else if(BitIdx == 9)
			{
				UART_TX_HIGH();				//ֹͣλ
			}
			else if(BitIdx == 10)
	#endif
#else
			else if(BitIdx == 9)
			{
	#if(IOUART_PARITY == IOUART_PARITY_EVEN)
				if(((evenOf1[TxBuff[ChrIdx] & 0xF] == 1) && (evenOf1[(TxBuff[ChrIdx] >> 4) & 0xF] == 1)) || 
				   ((evenOf1[TxBuff[ChrIdx] & 0xF] == 0) && (evenOf1[(TxBuff[ChrIdx] >> 4) & 0xF] == 0)))	// ż����1
	#else //IOUART_PARITY == IOUART_PARITY_ODD
				if(((evenOf1[TxBuff[ChrIdx] & 0xF] == 0) && (evenOf1[(TxBuff[ChrIdx] >> 4) & 0xF] == 1)) || 
				   ((evenOf1[TxBuff[ChrIdx] & 0xF] == 1) && (evenOf1[(TxBuff[ChrIdx] >> 4) & 0xF] == 0)))	// ������1
	#endif
					UART_TX_LOW();
				else
					UART_TX_HIGH();
			}
	#if(IOUART_STOP == IOUART_STOP_1)
			else if(BitIdx == 10)
	#else //IOUART_STOP == IOUART_STOP_2
			else if(BitIdx == 10)
			{
				UART_TX_HIGH();				//ֹͣλ
			}
			else if(BitIdx == 11)
	#endif
#endif
			{
				UART_TX_HIGH();				//ֹͣλ
				
				ChrIdx++;
				if(ChrIdx < ChrCnt)
				{
					BitIdx = 0;
					
					goto T3Svr_End;		//������BitIdx++�������򷢲�����ʼλ
				}
				else
				{
					TIMR_Stop(TIMR2);
					
					TxBusy = 0;
				}
			}
			
			BitIdx++;
		}
		
T3Svr_End:
		;
	}
}
