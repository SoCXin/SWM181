#include "SWM181.h"


volatile uint32_t PulseWidth = 0;

void SerialInit(void);
void TestSignal(void);

int main(void)
{	
	uint32_t i;
	
	SystemInit();
	
	SerialInit();
	
	TestSignal();		// ���������źŹ�Pulse���ܲ���
	
	PORT_Init(PORTA, PIN4, FUNMUX_PULSE_IN, 1);		// PA4 -> Pulse_IN
	
	TIMR_Init(TIMR0, TIMR_MODE_TIMER, 0xFFFFFFFF, 0);
	
	TIMRG->PCTRL = (1 << TIMRG_PCTRL_EN_Pos) |
				   (1 << TIMRG_PCTRL_HIGH_Pos);		// �����ߵ�ƽ����
	TIMRG->IE |=   (1 << TIMRG_IE_PULSE_Pos);
	
	IRQ_Connect(IRQ0_15_PULSE, IRQ4_IRQ, 1);
		
	while(1==1)
	{		
		printf("Pulse Width = %d\r\n", PulseWidth);
		
		for(i = 0; i < 5000000; i++);
	}
}

void IRQ4_Handler(void)
{
	if(TIMRG->IF & TIMRG_IF_PULSE_Msk)
	{
		TIMRG->IF = (1 << TIMRG_IF_PULSE_Pos);
		
		PulseWidth = TIMRG->PCVAL;
		
 		TIMRG->PCTRL |= (1 << TIMRG_PCTRL_EN_Pos);		// �ٴβ���
	}
}

void TestSignal(void)
{
	PWM_InitStructure  PWM_initStruct;
	
	PWM_initStruct.clk_div = PWM_CLKDIV_4;		//F_PWM = 24M/4 = 6M
	
	PWM_initStruct.mode = PWM_MODE_INDEP;		//A·��B·�������					
	PWM_initStruct.cycleA = 10000;				//6M/10000 = 600Hz			
	PWM_initStruct.hdutyA =  2500;				//2500/10000 = 25%
	PWM_initStruct.initLevelA = 1;
	PWM_initStruct.cycleB = 10000;
	PWM_initStruct.hdutyB =  5000;				//5000/10000 = 50%
	PWM_initStruct.initLevelB = 1;
	PWM_initStruct.HEndAIEn = 0;
	PWM_initStruct.NCycleAIEn = 0;
	PWM_initStruct.HEndBIEn = 0;
	PWM_initStruct.NCycleBIEn = 0;
	
	PWM_Init(PWM1, &PWM_initStruct);
	
	PORT_Init(PORTA, PIN5, FUNMUX_PWM1B_OUT, 0);
	
	PWM_Start(PWM1, 1, 1);
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
