#include "SWM181.h"

#include <string.h>

#define SLV_ADDR  0x6C

char mst_txbuff[4] = {0x37, 0x55, 0xAA, 0x78};
char mst_rxbuff[4] = {0};
char slv_txbuff[4] = {0};
char slv_rxbuff[4] = {0};

volatile uint32_t slv_rxindx = 0;
volatile uint32_t slv_txindx = 0;

void SerialInit(void);
void I2CMstInit(void);
void I2CSlvInit(void);

int main(void)
{
	uint32_t i;
	uint8_t ack;
	
	SystemInit();
	
	SerialInit();
	
	I2CMstInit();
	I2CSlvInit();
	
	while(1==1)
	{
		/*************************** Master Write ************************************/
		slv_rxindx = 0;
		
		ack = I2C_Start(I2C0, (SLV_ADDR << 1) | 0);
		if(ack == 0)
		{
			printf("Slave send NACK for address\r\n");
			goto nextloop;
		}
		
		for(i = 0; i < 4; i++)
		{
			ack = I2C_Write(I2C0, mst_txbuff[i]);
			if(ack == 0)
			{
				printf("Slave send NACK for data\r\n");
				goto nextloop;
			}
		}
		
		I2C_Stop(I2C0);
		
		printf("Master Send %X %X %X %X\r\n", mst_txbuff[0], mst_txbuff[1], mst_txbuff[2], mst_txbuff[3]);
		
		
		/********************************** Master Read *******************************/
		slv_txindx = 0;
		memcpy(slv_txbuff, slv_rxbuff, 4);
		
		ack = I2C_Start(I2C0, (SLV_ADDR << 1) | 1);
		if(ack == 0)
		{
			printf("Slave send NACK for address\r\n");
			goto nextloop;
		}
		
		for(i = 0; i < 3; i++)
		{
			mst_rxbuff[i] = I2C_Read(I2C0, 1);
		}
		mst_rxbuff[i] = I2C_Read(I2C0, 0);
		
		printf("Master Read %X %X %X %X\r\n", mst_rxbuff[0], mst_rxbuff[1], mst_rxbuff[2], mst_rxbuff[3]);
		
		if((mst_txbuff[0] == mst_rxbuff[0]) && (mst_txbuff[1] == mst_rxbuff[1]) && (mst_txbuff[2] == mst_rxbuff[2]) && (mst_txbuff[3] == mst_rxbuff[3]))
			printf("Success\r\n");
		else
			printf("Fail\r\n");

nextloop:
		I2C_Stop(I2C0);
		for(i = 0; i < 4000000; i++) ;
	}
}


void I2CMstInit(void)
{
	I2C_InitStructure I2C_initStruct;
	
	PORT_Init(PORTA, PIN4, FUNMUX_I2C0_SCL, 1);		//GPIOA.4����ΪI2C0 SCL����
	PORTA->OPEND |= (1 << PIN4);					//��©
	PORTA->PULLU |= (1 << PIN4);					//����
	PORT_Init(PORTA, PIN5, FUNMUX_I2C0_SDA, 1);		//GPIOA.5����ΪI2C0 SDA����
	PORTA->OPEND |= (1 << PIN5);					//��©
	PORTA->PULLU |= (1 << PIN5);					//����
	
	I2C_initStruct.Master = 1;
	I2C_initStruct.Addr7b = 1;
	I2C_initStruct.MstClk = 10000;
	I2C_initStruct.MstIEn = 0;
	I2C_Init(I2C0, &I2C_initStruct);
	
	I2C_Open(I2C0);
}

void I2CSlvInit(void)
{
	I2C_InitStructure I2C_initStruct;
	
	PORT_Init(PORTA, PIN6, FUNMUX_I2C1_SCL, 1);		//GPIOA.6����ΪI2C1 SCL����
	PORTA->OPEND |= (1 << PIN6);					//��©
	PORTA->PULLU |= (1 << PIN6);					//����
	PORT_Init(PORTA, PIN7, FUNMUX_I2C1_SDA, 1);		//GPIOA.7����ΪI2C1 SDA����
	PORTA->OPEND |= (1 << PIN7);					//��©
	PORTA->PULLU |= (1 << PIN7);					//����
	
	I2C_initStruct.Master = 0;
	I2C_initStruct.Addr7b = 1;
	I2C_initStruct.SlvAddr = SLV_ADDR;
	I2C_initStruct.SlvAddrMask = 0xFF;
	I2C_initStruct.SlvSTADetIEn = 1;
	I2C_initStruct.SlvRdReqIEn = 0;
	I2C_initStruct.SlvWrReqIEn = 1;
	I2C_initStruct.SlvTxEndIEn = 1;
	I2C_initStruct.SlvRxEndIEn = 1;
	I2C_initStruct.SlvSTODetIEn = 1;
	I2C_Init(I2C1, &I2C_initStruct);
	
	IRQ_Connect(IRQ0_15_I2C1, IRQ4_IRQ, 2);
	
	I2C_Open(I2C1);
}


void IRQ4_Handler(void)
{
	if(I2C1->SLVIF & I2C_SLVIF_STADET_Msk)				//�յ���ʼλ
	{
		I2C1->SLVIF = (1 << I2C_SLVIF_STADET_Pos);
		
		I2C1->SLVTX = slv_txbuff[0];
		slv_txindx = 1;
	}
	else if(I2C1->SLVIF & I2C_SLVIF_STODET_Msk)			//�յ�ֹͣλ
	{
		I2C1->SLVIF = (1 << I2C_SLVIF_STODET_Pos);
	}
	else if(I2C1->SLVIF & I2C_SLVIF_WRREQ_Msk)			//�յ�д����
	{
		I2C1->SLVIF = (1 << I2C_SLVIF_WRREQ_Pos);
		
		slv_rxindx = 0;
		I2C1->SLVCR |= (1 << I2C_SLVCR_ACK_Pos);
	}
	else if(I2C1->SLVIF & I2C_SLVIF_RXEND_Msk)			//�������
	{
		I2C1->SLVIF = (1 << I2C_SLVIF_RXEND_Pos);
		
		slv_rxbuff[slv_rxindx] = I2C1->SLVRX;
		if(slv_rxindx < 3) slv_rxindx++;
	}
	else if(I2C1->SLVIF & I2C_SLVIF_RDREQ_Msk)			//�յ�������
	{		
		I2C1->SLVIF = (1 << I2C_SLVIF_RDREQ_Pos);
		
		//�յ���������������SLVTX�е�ֵ���ͳ�ȥ�������ǵȴ����д����ٷ��ͣ����Խ���һ������д��SLVTXֻ����ǰ�����յ���ʼλ��
	}
	else if(I2C1->SLVIF & I2C_SLVIF_TXEND_Msk)			//�������
	{
		I2C1->SLVIF = (1 << I2C_SLVIF_TXEND_Pos);
		
		I2C1->SLVTX = slv_txbuff[slv_txindx];
		if(slv_txindx < 3) slv_txindx++;
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
