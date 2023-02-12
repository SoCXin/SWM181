#include "SWM181.h"

#include "ucos_ii.h"

/*  ˵����Ϊ�˱����޸� startup_SWM181.s �е�����������
	os_cpu_a.asm��os_cpu.h �е� OS_CPU_PendSVHandler �޸�Ϊ PendSV_Handler
	os_cpu_c.c��os_cpu.h �е� OS_CPU_SysTickHandler �޸�Ϊ SysTick_Handler
*/

OS_EVENT *queueADC;
void *queueADCTbl[16];

static  OS_STK TaskADCStk[TASK_ADC_STK_SIZE];
static  OS_STK TaskPWMStk[TASK_PWM_STK_SIZE];

void TaskADC(void *arg);
void TaskPWM(void *arg);
void SerialInit(void);

int main(void)
{ 	
 	SystemInit();
	
	SerialInit();
		
	OSInit();
	
	queueADC = OSQCreate(&queueADCTbl[0], 16);
	
	OSTaskCreate((void (*)(void *))TaskADC, (void *)0, &TaskADCStk[TASK_ADC_STK_SIZE - 1], TASK_ADC_PRIO);
	OSTaskCreate((void (*)(void *))TaskPWM, (void *)0, &TaskPWMStk[TASK_PWM_STK_SIZE - 1], TASK_PWM_PRIO);
	
	OS_CPU_SysTickInit(SystemCoreClock / OS_TICKS_PER_SEC);
	OSStart();
}


/****************************************************************************************************************************************** 
* ��������:	TaskADC()
* ����˵��: ����ADC�ɼ�����
* ��    ��: void *arg
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void TaskADC(void *arg)
{
	ADC_InitStructure ADC_initStruct;
	
	PORT_Init(PORTE, PIN4,  PORTE_PIN4_ADC_CH0, 0);	//PE.4  => ADC.CH0
	PORT_Init(PORTA, PIN15, PORTA_PIN15_ADC_CH1, 0);//PA.15 => ADC.CH1
	PORT_Init(PORTA, PIN14, PORTA_PIN14_ADC_CH2, 0);//PA.14 => ADC.CH2
	PORT_Init(PORTA, PIN13, PORTA_PIN13_ADC_CH3, 0);//PA.13 => ADC.CH3
	PORT_Init(PORTA, PIN12, PORTA_PIN12_ADC_CH4, 0);//PA.12 => ADC.CH4
	PORT_Init(PORTC, PIN7,  PORTC_PIN7_ADC_CH5, 0);	//PC.7  => ADC.CH5
	PORT_Init(PORTC, PIN6,  PORTC_PIN6_ADC_CH6, 0);	//PC.6  => ADC.CH6
													//ADC_CH7 => ADC.CH7
	
	ADC_initStruct.clk_src = ADC_CLKSRC_HRC_DIV4;
	ADC_initStruct.channels = ADC_CH6;
	ADC_initStruct.samplAvg = ADC_AVG_SAMPLE1;
	ADC_initStruct.trig_src = ADC_TRIGSRC_SW;
	ADC_initStruct.Continue = 0;					//������ģʽ��������ģʽ
	ADC_initStruct.EOC_IEn = ADC_CH6;	
	ADC_initStruct.OVF_IEn = 0;
	ADC_Init(ADC, &ADC_initStruct);					//����ADC
	
	IRQ_Connect(IRQ0_15_ADC, IRQ5_IRQ, 1);
	NVIC_SetPriority(IRQ5_IRQ, 2);
	
	ADC_Open(ADC);									//ʹ��ADC
	
	GPIO_Init(GPIOA, PIN5, 1, 0, 0, 0);				//����ָʾ�ź�
	
	while(1)
	{
		ADC_Start(ADC);
		
		OSTimeDly(OS_TICKS_PER_SEC/5);
	}
}

void IRQ5_Handler(void)
{		
	uint32_t val;
	
	ADC_IntEOCClr(ADC, ADC_CH6);	//����жϱ�־
	
	val = ADC_Read(ADC, ADC_CH6);
	
	OSQPost(queueADC, (void *)val);
	
	GPIO_InvBit(GPIOA, PIN5);
}


/****************************************************************************************************************************************** 
* ��������:	TaskPWM()
* ����˵��: �ȴ�ADCת�����������ADCת���������PWMռ�ձ�
* ��    ��: void *arg
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
void TaskPWM(void *arg)
{
	PWM_InitStructure PWM_initStruct;
	
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
	
	PORT_Init(PORTA, PIN6, FUNMUX_PWM1A_OUT, 0);
	PORT_Init(PORTA, PIN7, FUNMUX_PWM1B_OUT, 0);
	
	PWM_Start(PWM1, 1, 1);
	
	while(1)
	{
		uint8_t err;
		uint32_t duty;
		
		duty = (uint32_t)OSQPend(queueADC, 10, &err);
		if(err == OS_ERR_NONE)
		{
			printf("%d,", duty);
			if(duty <  100) duty =  100;
			if(duty > 4000) duty = 4000;
			
			PWM_SetHDuty(PWM1, PWM_CH_A, 10000 * duty / 4095);
			PWM_SetHDuty(PWM1, PWM_CH_B, 10000 - PWM_GetHDuty(PWM1, PWM_CH_A));
		}
	}
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
