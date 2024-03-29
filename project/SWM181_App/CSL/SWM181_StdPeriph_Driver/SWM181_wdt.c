/****************************************************************************************************************************************** 
* 文件名称: SWM181_wdt.c
* 功能说明:	SWM181单片机的WDT看门狗功能驱动库
* 技术支持:	http://www.synwit.com.cn/e/tool/gbook/?bid=1
* 注意事项:
* 版本日期:	V1.0.0		2016年1月30日
* 升级记录:  
*
*
*******************************************************************************************************************************************
* @attention
*
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS WITH CODING INFORMATION 
* REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME. AS A RESULT, SYNWIT SHALL NOT BE HELD LIABLE 
* FOR ANY DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE CONTENT 
* OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING INFORMATION CONTAINED HEREIN IN CONN-
* -ECTION WITH THEIR PRODUCTS.
*
* COPYRIGHT 2012 Synwit Technology
*******************************************************************************************************************************************/
#include "SWM181.h"
#include "SWM181_wdt.h"


/****************************************************************************************************************************************** 
* 函数名称:	WDT_Init()
* 功能说明:	WDT看门狗初始化
* 输    入: WDT_TypeDef * WDTx		指定要被设置的看门狗，有效值包括WDT
*			uint32_t peroid			取值0--4294967295，单位为单片机系统时钟周期
*			uint32_t mode			WDT_MODE_RESET 超时产生复位    WDT_MODE_INTERRUPT 超时产生中断
* 输    出: 无
* 注意事项: 无
******************************************************************************************************************************************/
void WDT_Init(WDT_TypeDef * WDTx, uint32_t peroid, uint32_t mode)
{
	SYS->CLKEN |= (0x01 << SYS_CLKEN_WDT_Pos);
	
	WDT_Stop(WDTx);		//设置前先关闭
	
	WDT_INTClr(WDTx);
	
	WDTx->LOAD = peroid;
	
	if(mode == WDT_MODE_RESET)
	{
		WDTx->CR |= (1 << WDT_CR_RSTEN_Pos);
	}
	else // mode == WDT_MODE_INTERRUPT
	{
		WDTx->CR &= ~(1 << WDT_CR_RSTEN_Pos);
	}
}

/****************************************************************************************************************************************** 
* 函数名称:	WDT_Start()
* 功能说明:	启动指定WDT，开始倒计时
* 输    入: WDT_TypeDef * WDTx		指定要被设置的看门狗，有效值包括WDT
* 输    出: 无
* 注意事项: 无
******************************************************************************************************************************************/
void WDT_Start(WDT_TypeDef * WDTx)
{
	WDTx->CR |= (0x01 << WDT_CR_EN_Pos);
}

/****************************************************************************************************************************************** 
* 函数名称:	WDT_Stop()
* 功能说明:	关闭指定WDT，停止倒计时
* 输    入: WDT_TypeDef * WDTx		指定要被设置的看门狗，有效值包括WDT
* 输    出: 无
* 注意事项: 无
******************************************************************************************************************************************/
void WDT_Stop(WDT_TypeDef * WDTx)
{
	WDTx->CR &= ~(0x01 << WDT_CR_EN_Pos);
}

/****************************************************************************************************************************************** 
* 函数名称:	WDT_Feed()
* 功能说明:	喂狗，重新从装载值开始倒计时
* 输    入: WDT_TypeDef * WDTx		指定要被设置的看门狗，有效值包括WDT
* 输    出: 无
* 注意事项: 无
******************************************************************************************************************************************/
void WDT_Feed(WDT_TypeDef * WDTx)
{
	WDTx->FEED = 0x55;
}

/****************************************************************************************************************************************** 
* 函数名称:	WDT_GetValue()
* 功能说明:	获取指定看门狗定时器的当前倒计时值
* 输    入: WDT_TypeDef * WDTx		指定要被设置的看门狗，有效值包括WDT
* 输    出: int32_t					看门狗当前计数值
* 注意事项: 无
******************************************************************************************************************************************/
int32_t WDT_GetValue(WDT_TypeDef * WDTx)
{
	return WDTx->VALUE;
}

/****************************************************************************************************************************************** 
* 函数名称:	WDT_INTClr()
* 功能说明:	中断标志清除
* 输    入: WDT_TypeDef * WDTx		指定要被设置的看门狗，有效值包括WDT
* 输    出: 无
* 注意事项: 无
******************************************************************************************************************************************/
void WDT_INTClr(WDT_TypeDef * WDTx)
{
	WDTx->IF = 0;
}

/****************************************************************************************************************************************** 
* 函数名称:	WDT_INTStat()
* 功能说明:	中断状态查询
* 输    入: WDT_TypeDef * WDTx		指定要被设置的看门狗，有效值包括WDT
* 输    出: int32_t					1 发生中断溢出    0 未发生中断溢出
* 注意事项: 无
******************************************************************************************************************************************/
uint32_t WDT_INTStat(WDT_TypeDef * WDTx)
{
	return WDTx->IF;
}
