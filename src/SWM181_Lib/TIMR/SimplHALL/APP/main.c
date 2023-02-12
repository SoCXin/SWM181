#include "SWM181.h"


volatile uint32_t TStart[3] = {0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF};	//Timer��ʼֵ
volatile uint32_t Period[3] = {0, 0, 0},			//PWM���ڳ���
	              LWidth[3] = {0, 0, 0};			//PWM�͵�ƽ���

void SerialInit(void);
void TestSignal(void);

int main(void)
{	
	uint32_t i;
	
	SystemInit();
	
	SerialInit();
	
	TestSignal();	//���������źŹ�HALL���ܲ���
	
	PORT_Init(PORTA, PIN4, FUNMUX_HALL_A, 1);		//PA4 -> HALL_A
	PORT_Init(PORTA, PIN5, FUNMUX_HALL_B, 1);		//PA5 -> HALL_B
	PORT_Init(PORTA, PIN6, FUNMUX_HALL_C, 1);		//PA6 -> HALL_C
	
	TIMR_Init(TIMR0, TIMR_MODE_TIMER, TStart[0], 0);
	
	TIMRG->HALLCR &= ~(TIMRG_HALLCR_IEA_Msk | TIMRG_HALLCR_IEB_Msk | TIMRG_HALLCR_IEC_Msk);
	TIMRG->HALLCR |= (3 << TIMRG_HALLCR_IEA_Pos);	//HALL_A˫���ز����ж�
	TIMRG->HALLCR |= (3 << TIMRG_HALLCR_IEB_Pos);	//HALL_B˫���ز����ж�
// 	TIMRG->HALLCR |= (3 << TIMRG_HALLCR_IEC_Pos);	//HALL_C˫���ز����ж�
	
	IRQ_Connect(IRQ0_15_HALL, IRQ4_IRQ, 1);
	
	TIMR_Start(TIMR0);
	
	while(1==1)
	{
		for(i = 0; i < 3; i++)
			printf("HALL_%c: Period = %d, HWidth = %d, Duty = %%%d\r\n", i==0 ? 'A' : (i==1 ? 'B' : 'C'), Period[i], Period[i]-LWidth[i], (Period[i]-LWidth[i])*100/Period[i]);
		printf("\r\n");
		
		for(i = 0; i < SystemCoreClock/8; i++) __NOP();
	}
}

void IRQ4_Handler(void)
{	
	if(TIMRG->HALLSR & TIMRG_HALLSR_IFA_Msk)
	{
		TIMRG->HALLSR = TIMRG_HALLSR_IFA_Msk;		//����жϱ�־
		
		if(TIMRG->HALLSR & TIMRG_HALLSR_STA_Msk)	//������
		{
			LWidth[0] = TStart[0] > TIMRG->HALL_A ? TStart[0] - TIMRG->HALL_A : (0x7FFFFFFF + TStart[0]) - TIMRG->HALL_A;
		}
		else										//�½���
		{
			if(LWidth[0] != 0)						//�Ѳ⵽������
				Period[0] = TStart[0] > TIMRG->HALL_A ? TStart[0] - TIMRG->HALL_A : (0x7FFFFFFF + TStart[0]) - TIMRG->HALL_A;
			
			TStart[0] = TIMRG->HALL_A;
		}
	}
	
	if(TIMRG->HALLSR & TIMRG_HALLSR_IFB_Msk)
	{
		TIMRG->HALLSR = TIMRG_HALLSR_IFB_Msk;		//����жϱ�־
		
		if(TIMRG->HALLSR & TIMRG_HALLSR_STB_Msk)	//������
		{
			LWidth[1] = TStart[1] > TIMRG->HALL_B ? TStart[1] - TIMRG->HALL_B : (0x7FFFFFFF + TStart[1]) - TIMRG->HALL_B;
		}
		else										//�½���
		{
			if(LWidth[1] != 0)						//�Ѳ⵽������
				Period[1] = TStart[1] > TIMRG->HALL_B ? TStart[1] - TIMRG->HALL_B : (0x7FFFFFFF + TStart[1]) - TIMRG->HALL_B;
			
			TStart[1] = TIMRG->HALL_B;
		}
	}
	
	if(TIMRG->HALLSR & TIMRG_HALLSR_IFC_Msk)
	{
		TIMRG->HALLSR = TIMRG_HALLSR_IFC_Msk;		//����жϱ�־
		
		if(TIMRG->HALLSR & TIMRG_HALLSR_STC_Msk)	//������
		{
			LWidth[2] = TStart[2] > TIMRG->HALL_C ? TStart[2] - TIMRG->HALL_C : (0x7FFFFFFF + TStart[2]) - TIMRG->HALL_C;
		}
		else										//�½���
		{
			if(LWidth[2] != 0)						//�Ѳ⵽������
				Period[2] = TStart[2] > TIMRG->HALL_C ? TStart[2] - TIMRG->HALL_C : (0x7FFFFFFF + TStart[2]) - TIMRG->HALL_C;
			
			TStart[2] = TIMRG->HALL_C;
		}
	}
}

void TestSignal(void)
{
	PWM_InitStructure  PWM_initStruct;
	
	PORT_Init(PORTA, PIN7, FUNMUX_PWM1A_OUT, 0);
	PORT_Init(PORTA, PIN8, FUNMUX_PWM1B_OUT, 0);
	
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
