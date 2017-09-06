#include <stdint.h>

#include "stm32f0xx_hal.h"
#include "main.h"
#include "yxt.h"

uint8_t YXT_Buf[12];
uint8_t YXT_Status[9];
uint8_t YXT_RxBit,YXT_RxData;
uint8_t YXT_Update=0;

void YXT_Init(void)
{
	#if 0
	CLK_PeripheralClockConfig(CLK_PERIPHERAL_TIMER1, ENABLE);

	//TIM1_DeInit();
	TIM1_TimeBaseInit(31,TIM1_COUNTERMODE_UP,0xFFFF,0);
	TIM1_PWMIConfig(TIM1_CHANNEL_1,TIM1_ICPOLARITY_FALLING,TIM1_ICSELECTION_DIRECTTI,TIM1_ICPSC_DIV1,0x0F);
	TIM1_SelectSlaveMode(TIM1_SLAVEMODE_RESET);
	TIM1_SelectInputTrigger(TIM1_TS_TI1FP1);
	TIM1_ClearFlag(TIM1_FLAG_UPDATE | TIM1_FLAG_CC2);
	TIM1_ITConfig(TIM1_IT_CC2,ENABLE);
	TIM1_Cmd(ENABLE);
	
	#endif
	YXT_Update = 0;
}

void YXT_Tim_Receive(uint16_t duty)
{
  int8_t PlusCode;
  uint8_t checksum;
  uint8_t i;
  	
	YXT_RxData <<= 1;
	if ( duty > 25 && duty < 42 ) {
		YXT_RxData |= 0x01;
	}
  
	if ( (++YXT_RxBit % 8) == 0 ){
		YXT_Buf[YXT_RxBit/8-1] = YXT_RxData;
		YXT_RxData = 0;
	}
	
	if ( YXT_RxBit >= 96 ){
		YXT_RxBit = 0;
		
		checksum = YXT_Buf[0];
		for(i=1;i<12;i++)
			checksum ^= YXT_Buf[i];
	
		if ( checksum )	return ;
		if ( YXT_Update ) return ;
		
		if ( YXT_Buf[0] == 0x1F ){	//YXT_WXJX
			PlusCode = (char)(YXT_Buf[1]+0x9E);
			PlusCode = (char)(PlusCode ^ 0x47);
			PlusCode = (char)(PlusCode + 0x8D);
			PlusCode = (char)(PlusCode ^ 0x3A);
			PlusCode = (char)(PlusCode + (YXT_Buf[2] >> 4));
			PlusCode = (char)(PlusCode ^ 0xCB);
			PlusCode = (char)(PlusCode + 0x03);
			PlusCode = (char)(PlusCode ^ 0xAC);
			PlusCode = PlusCode & 0x7F;
		} else if ( YXT_Buf[0] == 0x25 ){	//??-????
			PlusCode = (char)(YXT_Buf[1]+0xEA);
			PlusCode = (char)(PlusCode ^ 0xDB);
			PlusCode = (char)(PlusCode + 0xBF);
			PlusCode = (char)(PlusCode ^ 0x5A);
			PlusCode = (char)(PlusCode + (YXT_Buf[2] >> 4));
			PlusCode = (char)(PlusCode ^ 0x4B);
			PlusCode = (char)(PlusCode + 0xDB);
			PlusCode = (char)(PlusCode ^ 0xBD);
			PlusCode = PlusCode & 0x7F;
		} else if ( YXT_Buf[0] == 0x2B ){	//YXT_LQGS
			PlusCode = (char)(YXT_Buf[1]+0x2C);
			PlusCode = (char)(PlusCode ^ 0x6F);
			PlusCode = (char)(PlusCode + 0xDF);
			PlusCode = (char)(PlusCode ^ 0x7A);
			PlusCode = (char)(PlusCode + (YXT_Buf[2] >> 4));
			PlusCode = (char)(PlusCode ^ 0x2B);
			PlusCode = (char)(PlusCode + 0x0D);
			PlusCode = (char)(PlusCode ^ 0xC2);
			PlusCode = PlusCode & 0x7F;
		} else if ( YXT_Buf[0] == 0x08 ){	//????????
			PlusCode = 0;
		}	else {
			return ;
		}
		
		YXT_Status[0] = YXT_Buf[2] - (((YXT_Buf[2] >> 4))*0x10);
		YXT_Status[1] = YXT_Buf[3] - PlusCode;
		YXT_Status[2] = YXT_Buf[4] - PlusCode;
		YXT_Status[3] = YXT_Buf[5] - PlusCode;
		YXT_Status[4] = YXT_Buf[6];
		YXT_Status[5] = YXT_Buf[7] - PlusCode;
		YXT_Status[6] = YXT_Buf[8] - PlusCode;
		YXT_Status[7] = YXT_Buf[9] - PlusCode;
		YXT_Status[8] = YXT_Buf[10]- PlusCode;
		
		YXT_Update = 1;
	}
  if ( duty > 2560 ) {
		YXT_RxBit  = 0;
		YXT_RxData = 0;
	}	
}

void YXT_Task(BIKE_STATUS *bike)
{
	static uint32_t pre_tick=0;
	uint32_t speed;
	
	if ( YXT_Update ){
		pre_tick = HAL_GetTick();
		bike->YXTERR = 0;

		if ( (YXT_Status[1] & (1<<0)) )	bike->PhaseERR= 1; else bike->PhaseERR= 0;
		if ( (YXT_Status[1] & (1<<6)) )	bike->HallERR = 1; else bike->HallERR = 0;
		if ( (YXT_Status[1] & (1<<5)) ) bike->WheelERR= 1; else bike->WheelERR= 0; 
		if ( (YXT_Status[1] & (1<<4)) )	bike->ECUERR 	= 1; else bike->ECUERR 	= 0;
	//if ( (YXT_Status[1] & (1<<2)) )	bike->Cruise 	= 1; else bike->Cruise 	= 0;
		if ( (YXT_Status[2] & (1<<5)) )	bike->Braked 	= 1; else bike->Braked 	= 0;
	
		bike->SpeedMode = ((YXT_Status[2]>>5)&0x04)|(YXT_Status[2]&0x03);
		speed = ((uint32_t)YXT_Status[5]<<8) | YXT_Status[6];
		bike->Speed = speed*5/60;	//600->50Km/h

		YXT_Update = 0;  
	} else if ( Get_ElapseTick(pre_tick) > 3000 ){
		bike->YXTERR 		= 1;
		bike->HallERR 	= 0;
		bike->PhaseERR 	= 0;
		bike->WheelERR 	= 0;
		bike->ECUERR 		= 0;
	}	
}

