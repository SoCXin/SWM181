#include "SWM181.h"

#include <string.h>

#include "IOUART_RX.h"
#include "IOUART_TX.h"

char RxBuff[64] = {0};


int main(void)
{
	char buff[65];
	uint32_t stat;
	
 	SystemInit();
		
	IOUART_RX_Init();
	IOUART_TX_Init();
	
	printf("Hi from Synwit!\r\n");
	
 	while(1==1)
 	{
		IOUART_RX_RecvChars(RxBuff, 64);	//֪ͨUARTģ��Ҫ����64���ַ������յ����ַ�����RxBuff��
		while((stat = IOUART_RX_State()) < IOUART_RX_BUFFULL);
		switch(stat)
		{
		case IOUART_RX_BUFFULL:
		case IOUART_RX_TIMEOUT:
			memset(buff, 0, 65);
			memcpy(buff, RxBuff, IOUART_RX_Count());	//�����յ������ݿ�����������
			IOUART_RX_Clear();							//������������ݱ�������״̬
			printf("IOUART Received: %s\r\n", buff);			
			break;
		
		case IOUART_RX_FRAMERR:
		case IOUART_RX_PARITYERR:
			IOUART_RX_Clear();							//������������ݱ�������״̬
			printf("IOUART Receive Error!\r\n");
			break;
		}
 	}
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
	IOUART_TX_SendChars((char *)&ch, 1);
	
	while(IOUART_TX_IsBusy());
	
	return ch;
}
