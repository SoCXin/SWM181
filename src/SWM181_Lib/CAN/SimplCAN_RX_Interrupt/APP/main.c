#include "SWM181.h"

CAN_RXMessage CAN_RXMsg;

void SerialInit(void);

uint32_t sameBits(uint32_t std_id[], uint32_t std_n, uint32_t ext_id[], uint32_t ext_n);

int main(void)
{
	uint32_t i;
	CAN_InitStructure CAN_initStruct;
	
	SystemInit();
	
	SerialInit();
	
   	PORT_Init(PORTA, PIN4, FUNMUX_CAN_RX, 1);	//GPIOA.4����ΪCAN��������
	PORT_Init(PORTA, PIN5, FUNMUX_CAN_TX, 0);	//GPIOA.5����ΪCAN�������
	
	CAN_initStruct.Mode = CAN_MODE_NORMAL;
	CAN_initStruct.CAN_BS1 = CAN_BS1_4tq;
	CAN_initStruct.CAN_BS2 = CAN_BS2_3tq;
	CAN_initStruct.CAN_SJW = CAN_SJW_3tq;
	CAN_initStruct.Baudrate = 100000;
	CAN_initStruct.FilterMode = CAN_FILTER_32b;
	switch(0)
	{
	case 0:		// ����IDΪ����ֵ��֡
		CAN_initStruct.FilterMask32b  = 0xFFFFFFFF;				// 0 must match    1 don't care
		CAN_initStruct.FilterCheck32b = 0xFFFFFFFF;
		break;
	
	case 1:		// ����IDΪ0x122�ı�׼֡
		CAN_initStruct.FilterMask32b = ~(0x7FFu << 21);
		CAN_initStruct.FilterCheck32b = (0x122  << 21);			// ��׼֡ID�ڵ�21λ��31λ������ֲ�
		break;
	
	case 2:		// ����IDΪ0x122����չ֡
		CAN_initStruct.FilterMask32b = ~(0x1FFFFFFFu << 3);
		CAN_initStruct.FilterCheck32b = (0x00000122  << 3);		// ��չ֡ID�ڵ� 3λ��31λ������ֲ�
		break;
	
	case 3:		// ����IDΪ0x12X�ı�׼֡��X��ʾID�ĵ�4λ��ʲôֵ����ν
		CAN_initStruct.FilterMask32b = ~(0x7F0u << 21);
		CAN_initStruct.FilterCheck32b = (0x122  << 21);
		break;
	
	case 4:		// ����IDΪ0x122��0x101�ı�׼֡
		CAN_initStruct.FilterMode = CAN_FILTER_16b;
		CAN_initStruct.FilterMask16b1 = (uint16_t)~(0x7FFu << 5);
		CAN_initStruct.FilterCheck16b1 = (0x122  << 5);
		CAN_initStruct.FilterMask16b2 = (uint16_t)~(0x7FFu << 5);
		CAN_initStruct.FilterCheck16b2 = (0x101  << 5);
		break;
	
	case 5:		// ������IDΪ0x122��0x235��0x450�ı�׼֡��Ҳ����IDΪ0x101, 0x235, 0x1780����չ֡
		{		// ������ֻ����IDΪ��6��ֵ��֡�����ǽ��վ������ٵ�֡�����ܱ�֤IDΪҪ��ֵ��֡���ܽ��յ�
		uint32_t stdID[] = {0x122, 0x235, 0x450};
		uint32_t extID[] = {0x101, 0x235, 0x1780};
		
		CAN_initStruct.FilterMask32b = ~(sameBits(stdID, 3, extID, 3) << 3);
		CAN_initStruct.FilterCheck32b = (extID[0]  << 3);
		// ���ߣ�
// 		CAN_initStruct.FilterCheck32b = (stdID[0]  << 21);
		}
		break;
	}
	CAN_initStruct.RXNotEmptyIEn = 1;
	CAN_initStruct.RXOverflowIEn = 0;
	CAN_initStruct.ArbitrLostIEn = 0;
	CAN_initStruct.ErrPassiveIEn = 0;
	CAN_Init(CAN, &CAN_initStruct);
	
	IRQ_Connect(IRQ0_15_CAN, IRQ5_IRQ, 1);
	
	CAN_Open(CAN);
	
	printf("Hi from Synwit\r\n");
	
	while(1==1)
	{
		if(CAN_RXMsg.size > 0)
		{
			printf("\r\nReceive %s: %08X, ", CAN_RXMsg.format == CAN_FRAME_STD ? "STD" : "EXT", CAN_RXMsg.id);
			for(i = 0; i < CAN_RXMsg.size; i++) printf("%02X, ", CAN_RXMsg.data[i]);
			
			CAN_RXMsg.size = 0;
		}
		else if(CAN_RXMsg.remote == 1)	//Զ��֡
		{
			printf("\r\nReceive %s Remote Request", CAN_RXMsg.format == CAN_FRAME_STD ? "STD" : "EXT");
			
			CAN_RXMsg.remote = 0;
		}
	}
}

void IRQ5_Handler(void)
{
	uint32_t int_sr = CAN_INTStat(CAN);
	
	if(int_sr & CAN_IF_RXDA_Msk)
	{
		CAN_Receive(CAN, &CAN_RXMsg);
	}
}

/****************************************************************************************************************************************** 
* ��������: sameBits()
* ����˵��: �ҵ�����ID����ͬλ��ֵ��ͬ��λ����������ID�ĵ�10λ����0����1���򷵻�ֵ�ĵ�10λΪ1
* ��    ��: 
* ��    ��: 
* ע������: ��
******************************************************************************************************************************************/
uint32_t sameBits(uint32_t std_id[], uint32_t std_n, uint32_t ext_id[], uint32_t ext_n)
{
	uint32_t i, j;

	uint32_t mask = 0;
	
	for(i = 0; i < std_n; i++)
		std_id[i] = std_id[i] << 18;
	
	for(i = 0; i < std_n-1; i++)
	{
		for(j = i+1; j < std_n; j++)
		{
			mask |= std_id[i] ^ std_id[j];
		}
	}
	
	for(i = 0; i < ext_n-1; i++)
	{
		for(j = i+1; j < ext_n; j++)
		{
			mask |= ext_id[i] ^ ext_id[j];
		}
	}
	
	return ~mask;
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
