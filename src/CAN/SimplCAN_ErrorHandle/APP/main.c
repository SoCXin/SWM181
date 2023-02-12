#include "SWM181.h"


/* ��ʾ����˵����
	һ���� CAN �� TX �� RX ���������߽� CAN �����շ������շ������������� CAN �ڵ㣬���߽� CAN �����շ������շ����������� CAN �ڵ㵫���� CAN �ڵ㲻��Ӧ 0x7FFFFFF ID��
	ִ�г���ͨ���������ֻᷢ�� CAN->TXERR ����𽥵����� 128�����˺��ٵ���������ԭ��ο� CAN spec ������˵����
	
		3. When a TRANSMITTER sends an ERROR FLAG the TRANSMIT ERROR COUNT is increased by 8.

		Exception 1:
		If the TRANSMITTER is 'error passive' and detects an ACKNOWLEDGMENT ERROR because of not detecting
		a 'dominant' ACK and does not detect a 'dominant' bit while sending its PASSIVE ERROR FLAG.
	
	������ʱ ACK ERROR ���ٵ��� CAN->TXERR���� CAN �� TX �� RX ���ӶϿ������߽� CAN ���շ����Ͽ��������� BIT ERROR��ͨ���������ֿɷ��� CAN->TXERR ���������� 256��
	���� CAN->SR.BUSOFF �� CAN->CR.RST ��λ��Ȼ�� CAN->TXERR ֵ��� 127����ʱ CAN �ڵ㴦�� Bus Off ״̬��CAN ���������ڸ�λ״̬��
	
	����CAN->CR.RST ���������󣬽� CAN �� TX �� RX ���������߽� CAN ���շ�����������CAN �ڵ��˳� Bus Off ״̬
*/


void SerialInit(void);

int main(void)
{
	uint32_t i;
	uint8_t tx_data[8] = {1, 2, 3, 4, 5, 6, 7, 8};
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
   	CAN_initStruct.FilterMask32b = 0x00000000;
	CAN_initStruct.FilterCheck32b = 0xFFFFFFFF;
	CAN_initStruct.RXNotEmptyIEn = 0;
	CAN_initStruct.RXOverflowIEn = 0;
	CAN_initStruct.ArbitrLostIEn = 0;
	CAN_initStruct.ErrPassiveIEn = 0;
	CAN_Init(CAN, &CAN_initStruct);
	
	CAN_Open(CAN);
	
	while(1==1)
	{
		while((CAN->SR & CAN_SR_BUSOFF_Msk) == 0)
		{
			CAN_Transmit(CAN, CAN_FRAME_STD, 0x60D, tx_data, 8, 1);
			
			while(CAN_TXComplete(CAN) == 0) __NOP();
			
			printf("CAN->TXERR: %d\r\n", CAN->TXERR);
			
			for(i = 0; i < SystemCoreClock/32; i++) __NOP();
		}
		
		printf("CAN->CR.RST: %d\r\n", CAN->CR & CAN_CR_RST_Msk);	// ���� BusOff ʱ CR.RST �Զ���λ�����븴λģʽ
		
		CAN->CR &= ~CAN_CR_RST_Msk;		// ��� CR.RST λ���˳���λģʽ��������������ģʽ
		
		for(i = 0; i < SystemCoreClock/2; i++) __NOP();
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
