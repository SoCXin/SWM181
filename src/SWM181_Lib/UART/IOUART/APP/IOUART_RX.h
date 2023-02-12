#ifndef __IOUART_RX_H__
#define __IOUART_RX_H__

void IOUART_RX_Init(void);

void IOUART_RX_RecvChars(char buff[], uint32_t size);

uint32_t IOUART_RX_State(void);
uint32_t IOUART_RX_Count(void);
void IOUART_RX_Clear(void);

#define IOUART_RX_IDLE		0	//�����У����յ�ֹͣλ����
#define IOUART_RX_BUSY		1	//�����У����յ���ʼλ����
#define IOUART_RX_BUFFULL	2	//��������ֹͣ����
#define IOUART_RX_TIMEOUT	3	//�ճ�ʱ��ֹͣ����
#define IOUART_RX_FRAMERR	4	//֡����ֹͣ����
#define IOUART_RX_PARITYERR 5	//��żУ�����ֹͣ����

#endif //__IOUART_RX_H__
