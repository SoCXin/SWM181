#include "SWM181.h"

#define CACHE_BYPASS   (*((volatile uint32_t *)0x40000400))
#define DRAM_SIZE      (*((volatile uint32_t *)0x40000408))


__attribute__((section("PlaceInRAM")))
void jumpToApp(void)
{
	uint32_t i = 0;
	uint32_t FlagWord[3] = {0};

	__disable_irq();

	while(FLASH->STAT&FLASH_STAT_BUSY_Msk);	
	FLASH->CR = (1<<FLASH_CR_FFCLR_Pos);
	FLASH->CR = (FLASH_CMD_READ_DATA<<FLASH_CR_CMD_Pos)|(0<<FLASH_CR_LEN_Pos);
	FLASH->ADDR = 0x20;
	FLASH->START = 1;	
	for(i=0; i<3; i++)
	{
		while(FLASH->STAT&FLASH_STAT_FE_Msk);
		FlagWord[i] = FLASH->DATA;
	}	
	FLASH->START = 0;	
	FLASH->CR = (1<<FLASH_CR_FFCLR_Pos);
	FLASH->CR = 0;
	
	FlagWord[2] >>= 16;
	if(FlagWord[2]==3)  //基于sram  24K(16K + Low_8K) + 8K(High_8K)
	{
		DRAM_SIZE = 0x02;  //切换RAM为24K+8K模式 
		FlagWord[1] >>= 12;
		DMA->EN = 0x01;
		for(i=0; i<FlagWord[1]; i++)
		{
			DMA->CH[DMA_CHR_FLASH].CR &= ~(0x01<<DMA_CR_REN_Pos);			
			DMA->CH[DMA_CHR_FLASH].SRC = 0x00+i*0x1000;  //FlashAddr = 0x00
			DMA->CH[DMA_CHR_FLASH].DST = 0x00+i*0x1000;  //SramAddr = 0x00				
			DMA->CH[DMA_CHR_FLASH].CR &= ~DMA_CR_LEN_Msk;				
			DMA->CH[DMA_CHR_FLASH].CR |= ((0x1000-1)<<DMA_CR_LEN_Pos);			
			DMA->CH[DMA_CHR_FLASH].CR |= (0x01<<DMA_CR_REN_Pos);				
			while(DMA->CH[DMA_CHR_FLASH].CR & DMA_CR_REN_Msk);
			while(FLASH->STAT&(0x01<<16));
		}		
	}
	else
	{
		for(i=0; i<65; i++)  //循环次数根据IspToApp()函数大小确定
		{
			*((volatile uint32_t *)(0x00+i*4)) = *((volatile uint32_t *)(0x20000000+i*4));
		}				
		if(FlagWord[2]==0)  //基于cache  16K(Low_8K + High_8K) + 16K 
		{
			CACHE_BYPASS = 0x02;
			CACHE->CR = 0x01; 
			CACHE->PREFCR = 0x01;
			while(!(CACHE->PREFSR&0x01));
			CACHE->PREFSR = (0x07<<1);		
		}
		else  //基于cache  8K(Low_8K) + 24K(16K + High_8K)
		{
			CACHE_BYPASS = 0x00;
			CACHE->CR = 0x01;
			CACHE->PREFCR = 0x01;
			while(!(CACHE->PREFSR&0x01));
			CACHE->PREFSR = (0x07<<1);	
		}	
	}
	SCB->AIRCR = ((0x5FA<<SCB_AIRCR_VECTKEY_Pos)|SCB_AIRCR_SYSRESETREQ_Msk);
	__DSB();        
	while(1);	
}


__attribute__((section("PlaceAtFront")))
void load8KCode(void)
{
	uint32_t i = 0;
	uint32_t FlagWord = 0;

	while(FLASH->STAT&FLASH_STAT_BUSY_Msk);	
	FLASH->CR = (1<<FLASH_CR_FFCLR_Pos);
	FLASH->CR = (FLASH_CMD_READ_DATA<<FLASH_CR_CMD_Pos)|(0<<FLASH_CR_LEN_Pos);
	FLASH->ADDR = 0x28;
	FLASH->START = 1;	
	while(FLASH->STAT&FLASH_STAT_FE_Msk);
	FlagWord = FLASH->DATA;
	FLASH->START = 0;	
	FLASH->CR = (1<<FLASH_CR_FFCLR_Pos);
	FLASH->CR = 0;
	
	FlagWord >>= 8;
	FlagWord &= 0xFF;
	
	DRAM_SIZE = 0x02;  //切换RAM为24K+8K模式
	
	
	while(FLASH->STAT & FLASH_STAT_BUSY_Msk);
	
	FLASH->CR = (1 << FLASH_CR_FFCLR_Pos);			//Clear FIFO
	
	FLASH->CR = (FLASH_CMD_READ_DATA << FLASH_CR_CMD_Pos) |
				(0 << FLASH_CR_LEN_Pos);			//连续读
	FLASH->ADDR = (FlagWord + 4) * 0x1000;
	FLASH->START = 1;
	
	for(i = 0; i < 0x1000 * 2; i += 4)
	{
		while(FLASH->STAT & FLASH_STAT_FE_Msk);		//FIFO Empty
		
		*((uint32_t *)(4 * 0x1000 + i)) = FLASH->DATA;
	}
	
	FLASH->START = 0;
	
	FLASH->CR = (1 << FLASH_CR_FFCLR_Pos);			//Clear FIFO
	FLASH->CR = 0;
}
