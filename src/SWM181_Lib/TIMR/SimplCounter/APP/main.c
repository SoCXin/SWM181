#include "SWM181.h"


int main(void)
{	
	SystemInit();
	
	GPIO_Init(GPIOA, PIN5, 1, 0, 0, 0);
	
	PORT_Init(PORTA, PIN7, FUNMUX_TIMR2_IN, 1);
	
	TIMR_Init(TIMR2, TIMR_MODE_COUNTER, 3, 1);	//ÿ��3�����ؽ����ж�
	
	IRQ_Connect(IRQ0_15_TIMR2, IRQ5_IRQ, 2);	//��ʱ��2�ж����ӵ�IRQ5�ж��ߣ������ȼ�
	
	TIMR_Start(TIMR2);
	
	while(1==1)
	{
	}
}

void IRQ5_Handler(void)
{
	TIMR_Stop(TIMR2);			//�������������������жϣ�������������˳���ܵ�����������ν����жϴ�����
	TIMR_INTClr(TIMR2);
	TIMR_Start(TIMR2);
	
	GPIO_InvBit(GPIOA, PIN5);
}
