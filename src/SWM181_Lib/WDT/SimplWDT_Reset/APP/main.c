#include "SWM181.h"


int main(void)
{
	uint32_t i;

	SystemInit();
	
	for(i=0;i<1000000;i++);						//��λ���һ����ٵ���LED
	
	GPIO_Init(GPIOA, PIN5, 1, 0, 0, 0);			//�������LED
	GPIO_SetBit(GPIOA, PIN5);					//����LED
	
	WDT_Init(WDT, SystemCoreClock/2, WDT_MODE_RESET);	//ÿ0.5����Ҫι��һ��
	WDT_Start(WDT);								//����WDT
	
	while(1==1)
	{
		
	}
}
