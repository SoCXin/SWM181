#include "SWM181.h"


int main(void)
{	
	SystemInit();
	
	GPIO_Init(GPIOA, PIN5, 1, 0, 0, 0);			//�������LED
	
	TIMR_Init(TIMR0, TIMR_MODE_TIMER, SystemCoreClock/2, 1);	//ÿ0.5���Ӵ���һ���ж�
	
	IRQ_Connect(IRQ0_15_TIMR0, IRQ5_IRQ, 2);		//��ʱ��0�ж����ӵ�IRQ5�ж��ߣ������ȼ�
	
	TIMR_Start(TIMR0);
	
	while(1==1)
	{
	}
}

void IRQ5_Handler(void)
{
	TIMR_INTClr(TIMR0);
	
	GPIO_InvBit(GPIOA, PIN5);	//��תLED����״̬
}

