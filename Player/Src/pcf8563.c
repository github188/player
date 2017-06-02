
#include <stdint.h>

#include "stm32f0xx_hal.h"

#include "pcf8563.h"

#define HOUR   16
#define MIN    24

extern I2C_HandleTypeDef hi2c1;

/******************************************************************************
                            ?����??��1?��?��?��?o����y
******************************************************************************/

unsigned char buffer[6];
_PCF8563_Time_Typedef 	RtcTime;
_PCF8563_Date_Typedef   RtcDate;


#define RTC_BinToBcd2(BINValue) ( ((BINValue/10) << 4) | (BINValue%10))

unsigned char RTC_Bcd2ToBin(unsigned char BCDValue)
{
	return ((BCDValue>>4)*10 + (BCDValue & 0x0F));
}

/**
  *****************************************************************************
  * @Name   : PCF8563?3??��??��D�䨨?��???��??����y?Y
  *
  * @Brief  : none
  *
  * @Input  : REG_ADD��o��a2������??��??�¦�??��
  *           dat��o    ��aD�䨨?��?��y?Y
  *
  * @Output : none
  *
  * @Return : none
  *****************************************************************************
**/
void PCF8563_Write_Byte(unsigned char REG_ADD, unsigned char dat)
{
	unsigned char buf[2];
	
	buf[0] = REG_ADD;
	buf[1] = dat;
	//I2C_WriteBuf(PCF8563_Write, buf, 2);
	HAL_I2C_Master_Transmit(&hi2c1, (uint16_t)PCF8563_Write, (uint8_t*)buf, 2, 1000);
} 

/**
  *****************************************************************************
  * @Name   : PCF8563?3??��??��?����?��???��??����y?Y
  *
  * @Brief  : none
  *
  * @Input  : REG_ADD��o��a2������??��??�¦�??��
  *
  * @Output : none
  *
  * @Return : ?����?��?��?��???��??�¦�??��
  *****************************************************************************
**/
unsigned char PCF8563_Read_Byte(unsigned char REG_ADD)
{
	unsigned char ReData;
	unsigned char buf[2];
	
	//I2C_ReadBuf(PCF8563_Write, REG_ADD, buf, 1);
	HAL_I2C_Mem_Read(&hi2c1, (uint16_t)PCF8563_Write, REG_ADD, 1 ,(uint8_t*)buf, 1, 1000);

	return buf[0];
}


/**
  *****************************************************************************
  * @Name   : PCF8563?��2a��?��?��??��
  *
  * @Brief  : ?��?������?�¦�1??����??��??��D�䨨?��???��y?��?��?����?3?�����???������??����??y������?2?��??���䨪?��
  *
  * @Input  : none
  *
  * @Output : none
  *
  * @Return : 0: ?y3��
  *           1: PCF8563�䨪?��?��???e?��
  *****************************************************************************
**/
unsigned char PCF8563_Check(void)
{
	unsigned char test_value = 0;
	
	// PCF8563_Write_Byte(PCF8563_Address_Timer, PCF_Timer_Close);  
	// PCF8563_Write_Byte(PCF8563_Address_Timer_VAL, PCF8563_Check_Data);  
	// test_value = PCF8563_Read_Byte(PCF8563_Address_Timer_VAL);  
	// if(test_value != PCF8563_Check_Data)  return 1;  
	// PCF8563_Write_Byte(PCF8563_Address_Control_Status_1, 0);  //run
	
	buffer[0] = PCF8563_Address_Seconds;
	buffer[1] = 0x55;
	buffer[2] = RtcTime.RTC_Minutes;
	buffer[3] = RtcTime.RTC_Hours;
	//I2C_WriteBuf(PCF8563_Write, buffer, 4);
	HAL_I2C_Master_Transmit(&hi2c1, (uint16_t)PCF8563_Write, (uint8_t*)buffer, 4, 1000);

	//I2C_ReadBuf(PCF8563_Write, PCF8563_Address_Seconds,buffer, 3);
	HAL_I2C_Mem_Read(&hi2c1, (uint16_t)PCF8563_Write, PCF8563_Address_Seconds, 1 ,(uint8_t*)buffer, 3, 1000);

	//PCF8563_Write_Byte(PCF8563_Address_Seconds, 0x55);  
	//I2C_ReadBuf(PCF8563_Write, PCF8563_Address_Seconds,buffer, 3);
	HAL_I2C_Mem_Read(&hi2c1, (uint16_t)PCF8563_Write, PCF8563_Address_Seconds, 1 ,(uint8_t*)buffer, 3, 1000);
	//I2C_ReadBuf(PCF8563_Write, 0,buffer, 0x0f);
	HAL_I2C_Mem_Read(&hi2c1, (uint16_t)PCF8563_Write, PCF8563_Address_Seconds, 1 ,(uint8_t*)buffer, 0x0F, 1000);
	//test_value = PCF8563_Read_Byte(PCF8563_Address_Seconds);  

	return 0;
}

/**
  *****************************************************************************
  * @Name   : PCF8563???��
  *
  * @Brief  : none
  *
  * @Input  : none
  *
  * @Output : none
  *
  * @Return : none
  *****************************************************************************
**/
void PCF8563_Start(void)
{
	unsigned char temp = 0;
	
	temp = PCF8563_Read_Byte(PCF8563_Address_Control_Status_1);  //?����?????/���䨬???��??��1
	if (temp & PCF_Control_ChipStop)
	{
		temp &= PCF_Control_ChipRuns;  //??DDD???
	}
	if ((temp & (1<<7)) == 0)  //??����?�꨺?
	{
		temp &= PCF_Control_TestcClose;  //��??��?��???�꨺?����D��
	}
	PCF8563_Write_Byte(PCF8563_Address_Control_Status_1, temp);  //?��D�䨨?��y?��
}

/**
  *****************************************************************************
  * @Name   : PCF8563����?1
  *
  * @Brief  : ����?��?��?����?3?CLKOUT ?�� 32.768kHz ����?����?
  *
  * @Input  : none
  *
  * @Output : none
  *
  * @Return : none
  *****************************************************************************
**/
void PCF8563_Stop(void)
{
	unsigned char temp = 0;
	
	temp = PCF8563_Read_Byte(PCF8563_Address_Control_Status_1);  //?����?????/���䨬???��??��1
	temp |= PCF_Control_ChipStop;  //����?1??DD
	PCF8563_Write_Byte(PCF8563_Address_Control_Status_1, temp);  //?��D�䨨?��y?��
}

/**
  *****************************************************************************
  * @Name   : PCF8563����????DD?�꨺?
  *
  * @Brief  : none
  *
  * @Input  : Mode: ??DD?�꨺?
  *                 = PCF_Mode_Normal
  *                 = PCF_Mode_EXT_CLK
  *
  * @Output : none
  *
  * @Return : none
  *****************************************************************************
**/
void PCF8563_SetMode(unsigned char Mode)
{
	unsigned char temp = 0;
	
	temp = PCF8563_Read_Byte(PCF8563_Address_Control_Status_1);  //?����???��??��?��
	if (Mode == PCF_Mode_EXT_CLK)  //EXT_CLK2a��??�꨺?
	{
		temp |= PCF_Control_Status_EXT_CLKMode;
	}
	else if (Mode == PCF_Mode_Normal)
	{
		temp &= PCF_Control_Status_NormalMode;
		temp &= ~(1<<3);  //��??��?��??1|?������D��
	}
	PCF8563_Write_Byte(PCF8563_Address_Control_Status_1, temp);
}

/**
  *****************************************************************************
  * @Name   : PCF8563����??��??��?��??1|?��?a??��?1?��?
  *
  * @Brief  : none
  *
  * @Input  : NewState: ���䨬?��?PCF8563_PowerResetEnablePCF8563_PowerResetDisable
  *
  * @Output : none
  *
  * @Return : none
  *****************************************************************************
**/
void PCF8563_SetPowerReset(unsigned char NewState)
{
	unsigned char TestC = 0;
	
	TestC = PCF8563_Read_Byte(PCF8563_Address_Control_Status_1);  //??��???��??��?��
	TestC &= ~(1<<3);  //??3y???�㨦��??
	if (NewState == PCF8563_PowerResetEnable)  //?��??1|?����DD��
	{
		TestC |= PCF8563_PowerResetEnable;
	}
	else if (NewState == PCF8563_PowerResetDisable)
	{
		TestC &= ~PCF8563_PowerResetEnable;  //����D���???����?�꨺?��??��???-0��??�䨺��D��
	}
	PCF8563_Write_Byte(PCF8563_Address_Control_Status_1, TestC);  //D�䨨?��y?��
}

/**
  *****************************************************************************
  * @Name   : PCF8563����??��?3??��?��
  *
  * @Brief  : none
  *
  * @Input  :*PCF_CLKOUTStruct: ?��?��?��11????
  *
  * @Output : none
  *
  * @Return : none
  *****************************************************************************
**/
void PCF8563_SetCLKOUT(_PCF8563_CLKOUT_Typedef* PCF_CLKOUTStruct)
{
	unsigned char tmp = 0;
	
	tmp = PCF8563_Read_Byte(PCF8563_Address_CLKOUT);  //?����???��??��?��
	tmp &= 0x7c;  //??3y???�㨦��??
	if (PCF_CLKOUTStruct->CLKOUT_NewState == PCF_CLKOUT_Open)
	{
		tmp |= PCF_CLKOUT_Open;
	}
	else
	{
		tmp &= PCF_CLKOUT_Close;
	}
	tmp |= PCF_CLKOUTStruct->CLKOUT_Frequency;
	
	PCF8563_Write_Byte(PCF8563_Address_CLKOUT, tmp);
}

/**
  *****************************************************************************
  * @Name   : PCF8563����???������?��
  *
  * @Brief  : none
  *
  * @Input  :*PCF_TimerStruct: ?������?��?��11????
  *
  * @Output : none
  *
  * @Return : none
  *****************************************************************************
**/
void PCF8563_SetTimer(_PCF8563_Timer_Typedef* PCF_TimerStruct)
{
	unsigned char Timer_Ctrl = 0;
	unsigned char Timer_Value = 0;
	
	Timer_Ctrl = PCF8563_Read_Byte(PCF8563_Address_Timer);  //??��???????��??��?��
	Timer_Value = PCF8563_Read_Byte(PCF8563_Address_Timer_VAL);  //??��?��1??������y?��
	//
	//?������?1?������?��
	//
	Timer_Ctrl &= PCF_Timer_Close;
	PCF8563_Write_Byte(PCF8563_Address_Timer, Timer_Ctrl);
	
	Timer_Ctrl &= 0x7c;  //??3y?������?��???�㨦��??
	
	if (PCF_TimerStruct->RTC_Timer_NewState == PCF_Timer_Open)  //?a??
	{
		Timer_Ctrl |= PCF_Timer_Open;
		Timer_Ctrl |= PCF_TimerStruct->RTC_Timer_Frequency;  //��?��?D?��?1�����?��?��
		if (PCF_TimerStruct->RTC_Timer_Value)  //D����a��?��?D?��???��y?��
		{
			Timer_Value = PCF_TimerStruct->RTC_Timer_Value;  //��?��?D?��???��y?��
		}
	}
	else
	{
		Timer_Ctrl &= PCF_Timer_Close;
	}
	PCF8563_Write_Byte(PCF8563_Address_Timer_VAL, Timer_Value);  //D�䨨?��1??������y?��
	
	if (PCF_TimerStruct->RTC_Timer_Interrupt == PCF_Time_INT_Open)  //?a??��??D??��?3?
	{
		Timer_Value = PCF8563_Read_Byte(PCF8563_Address_Control_Status_2);  //??��?????/���䨬???��??��2��y?��
		Timer_Value &= PCF_Time_INT_Close;  //??3y?������?��?D??��1?��
		Timer_Value &= ~(1<<2);  //??3y?������?��?D??����??
		Timer_Value &= ~(1<<4);  //�̡� TF ��DD�쨺�� INT ��DD�� (��???���� TIE ��?���䨬?) 
		Timer_Value |= PCF_Time_INT_Open;  //?a???������?��?D??��?3?
		PCF8563_Write_Byte(PCF8563_Address_Control_Status_2, Timer_Value);
	}
	else
	{
		Timer_Value = PCF8563_Read_Byte(PCF8563_Address_Control_Status_2);  //??��?????/���䨬???��??��2��y?��
		Timer_Value &= PCF_Time_INT_Close;  //??3y?������?��?D??��1?��
		Timer_Value |= PCF_Time_INT_Open;  //?a???������?��?D??��?3?
		PCF8563_Write_Byte(PCF8563_Address_Control_Status_2, Timer_Value);
	}
	
	PCF8563_Write_Byte(PCF8563_Address_Timer, Timer_Ctrl);  //����???������?��??????��??��
}

/**
  *****************************************************************************
  * @Name   : ����??����??��??�¨�a��?����o�������̡¨�?��??��??3?��??������??��?
  *
  * @Brief  : ????��??������??3��0x00��?��?D?2?��???2?��???��?D??��?�̡�??���o0 ~ 6
  *
  * @Input  : PCF_Format:  ��y?Y??��?
  *                        = PCF_Format_BIN
  *                        = PCF_Format_BCD
  *           PCF_Century: ����?��??����?��
  *                        = PCF_Century_19xx
  *                        = PCF_Century_20xx
  *           Year:        ?��
  *           Month:       ??
  *           Date:        ��?
  *           Week:        D??��
  *           Hour:        ����
  *           Minute:      ��?
  *
  * @Output : none
  *
  * @Return : none
  *****************************************************************************
**/
void PCF8563_Set_Times(unsigned char PCF_Format,\
                       unsigned char PCF_Century,\
	                   unsigned char Year, unsigned char Month, unsigned char Date, unsigned char Week,\
                       unsigned char Hour, unsigned char Minute)
{
	_PCF8563_Time_Typedef Time_InitStructure;
	_PCF8563_Date_Typedef Date_InitStructure;
	
	if (PCF_Format == PCF_Format_BIN)
	{
		//
		//?D??��y?Y��?��?��?o?��??��
		//
		if (Year > 99)   Year  = 0;  //???��00?��
		if (Month > 12)  Month = 1;  //???��1??
		if (Date > 31)   Date  = 1;  //???��1��?
		if (Week > 6)    Week  = 1;  //???��D??����?
		
		if (Hour > 23)    Hour   = 0;  //???��0D?����
		if (Minute > 59)  Minute = 0;  //???��0��??��
		
		//
		//��a??��???
		//
		Date_InitStructure.RTC_Years    = RTC_BinToBcd2(Year);
		Date_InitStructure.RTC_Months   = RTC_BinToBcd2(Month);
		Date_InitStructure.RTC_Days     = RTC_BinToBcd2(Date);
		Date_InitStructure.RTC_WeekDays = RTC_BinToBcd2(Week);
		
		Time_InitStructure.RTC_Hours    = RTC_BinToBcd2(Hour);
		Time_InitStructure.RTC_Minutes  = RTC_BinToBcd2(Minute);
	}
	Time_InitStructure.RTC_Seconds = 0x00;  //???��0??
	Time_InitStructure.RTC_Seconds &= PCF_Accuracy_ClockYes;  //����?���?������?����??
	//
	//?D??����?��??
	//
	if (PCF_Century == PCF_Century_19xx)
	{
		Date_InitStructure.RTC_Months |= PCF_Century_SetBitC;
	}
	else
	{
		Date_InitStructure.RTC_Months &= ~PCF_Century_SetBitC;
	}
	//
	//D�䨨?D??���???��??��
	//
		unsigned char buf[2];
	
	buffer[0] = PCF8563_Address_Seconds;
	buffer[1] = Time_InitStructure.RTC_Seconds;
	buffer[2] = Time_InitStructure.RTC_Minutes;
	buffer[3] = Time_InitStructure.RTC_Hours;
	//I2C_WriteBuf(PCF8563_Write, buffer, 4);
	HAL_I2C_Master_Transmit(&hi2c1, (uint16_t)PCF8563_Write, (uint8_t*)buffer, 4, 1000);

	
	buffer[0] = PCF8563_Address_Days;
	buffer[1] = Date_InitStructure.RTC_Days;
	buffer[2] = Date_InitStructure.RTC_WeekDays;
	buffer[3] = Date_InitStructure.RTC_Months;
	buffer[4] = Date_InitStructure.RTC_Years;
	//I2C_WriteBuf(PCF8563_Write, buffer, 5);
	HAL_I2C_Master_Transmit(&hi2c1, (uint16_t)PCF8563_Write, (uint8_t*)buffer, 5, 1000);
}

/**
   ============================================================================
                     #### ?����D??��??�¨�?2?2������1|?��o����y ####
   ============================================================================
**/

/**
  *****************************************************************************
  * @Name   : ?D??����??D??�騺?��?��?o?��??���?3?3??????��3??��
  *
  * @Brief  : D??��?�̡�??���o0 ~ 6
  *
  * @Input  : PCF_DataStruct: ??��??��?��11????
  *
  * @Output : none
  *
  * @Return : none
  *****************************************************************************
**/
static void IS_PCF8563_Data(_PCF8563_Register_Typedef* PCF_DataStruct)
{
	if (PCF_DataStruct->Years > 99)           PCF_DataStruct->Years          = 0;  //???��00?��
	if (PCF_DataStruct->Months_Century > 12)  PCF_DataStruct->Months_Century = 1;  //???��1??
	if (PCF_DataStruct->Days > 31)            PCF_DataStruct->Days           = 1;  //???��1��?
	if (PCF_DataStruct->WeekDays > 6)         PCF_DataStruct->WeekDays       = 1;  //???��D??����?
	
	if (PCF_DataStruct->Hours > 23)           PCF_DataStruct->Hours          = 0;  //???��0D?����
	if (PCF_DataStruct->Minutes > 59)         PCF_DataStruct->Minutes        = 0;  //???��0��??��
	if (PCF_DataStruct->Seconds > 59)         PCF_DataStruct->Seconds        = 0;  //???��0??
}

/**
  *****************************************************************************
  * @Name   : PCF8563D�䨨???��??��
  *
  * @Brief  : D??����y?�̡�??�쨺?: 0 ~ 6��?��???????��?
  *
  * @Input  : PCF_Format:     ��y?Y??��?
  *                           = PCF_Format_BIN
  *                           = PCF_Format_BCD
  *           PCF_Century:    ����?��??����?��
  *                           = PCF_Century_19xx
  *                           = PCF_Century_20xx
  *           PCF_DataStruct: ??��??��?��11????
  *
  * @Output : none
  *
  * @Return : none
  *****************************************************************************
**/
void PCF8563_SetRegister(unsigned char PCF_Format, unsigned char PCF_Century, _PCF8563_Register_Typedef* PCF_DataStruct)
{
	if (PCF_Format == PCF_Format_BIN)  //��???????��?��?D����a��a??��???
	{
		//
		//?D??��y?�̨�?��??����??��???��
		//
		IS_PCF8563_Data(PCF_DataStruct);
		//
		//D����a��a??��???
		//
		PCF_DataStruct->Years          = RTC_BinToBcd2(PCF_DataStruct->Years);
		PCF_DataStruct->Months_Century = RTC_BinToBcd2(PCF_DataStruct->Months_Century);
		PCF_DataStruct->Days           = RTC_BinToBcd2(PCF_DataStruct->Days);
		PCF_DataStruct->WeekDays       = RTC_BinToBcd2(PCF_DataStruct->WeekDays);
		
		PCF_DataStruct->Hours          = RTC_BinToBcd2(PCF_DataStruct->Hours);
		PCF_DataStruct->Minutes        = RTC_BinToBcd2(PCF_DataStruct->Minutes);
		PCF_DataStruct->Seconds        = RTC_BinToBcd2(PCF_DataStruct->Seconds);
		
		PCF_DataStruct->Day_Alarm      = RTC_BinToBcd2(PCF_DataStruct->Day_Alarm);
		PCF_DataStruct->WeekDays_Alarm = RTC_BinToBcd2(PCF_DataStruct->WeekDays_Alarm);
		
		PCF_DataStruct->Hour_Alarm     = RTC_BinToBcd2(PCF_DataStruct->Hour_Alarm);
		PCF_DataStruct->Minute_Alarm   = RTC_BinToBcd2(PCF_DataStruct->Minute_Alarm);
	}
	//
	//?D??����?��??
	//
	if (PCF_Century == PCF_Century_19xx)
	{
		PCF_DataStruct->Months_Century |= PCF_Century_SetBitC;
	}
	else
	{
		PCF_DataStruct->Months_Century &= ~PCF_Century_SetBitC;
	}
	//
	//1?��??����D??��?��??D??��?3?
	//
	PCF_DataStruct->Timer_Control    &= ~(1<<7);
	PCF_DataStruct->CLKOUT_Frequency &= ~(1<<7);
	PCF_DataStruct->WeekDays_Alarm   &= ~(1<<7);
	PCF_DataStruct->Day_Alarm        &= ~(1<<7);
	PCF_DataStruct->Hour_Alarm       &= ~(1<<7);
	PCF_DataStruct->Minute_Alarm     &= ~(1<<7);
	PCF_DataStruct->Control_Status_2 &= ~(3<<0);

	//
	//D�䨨?��y?Y��???��??��
	//
	//I2C_WriteBuf(PCF8563_Write, (unsigned char*)PCF_DataStruct, 17);
	HAL_I2C_Master_Transmit(&hi2c1, (uint16_t)PCF8563_Write, (uint8_t*)PCF_DataStruct, 17, 1000);

}

/**
  *****************************************************************************
  * @Name   : PCF8563?����???��??��
  *
  * @Brief  : D??����y?�̡�??�쨺?: 0 ~ 6��?��???????��?
  *
  * @Input  : PCF_Format:     ��y?Y??��?
  *                           = PCF_Format_BIN
  *                           = PCF_Format_BCD
  *
  * @Output : *PCF_Century:   ����?��??��?0��o21����?����?1:20����?��
  *           PCF_DataStruct: ??��??��?��11????
  *
  * @Return : none
  *****************************************************************************
**/
void PCF8563_GetRegister(unsigned char PCF_Format, unsigned char *PCF_Century, _PCF8563_Register_Typedef* PCF_DataStruct)
{
	//
	//?����?��?2???��??�¨�y?��
	//
	//I2C_ReadBuf(PCF8563_Write, PCF8563_Address_Control_Status_1,(unsigned char*)PCF_DataStruct, 16);
	HAL_I2C_Mem_Read(&hi2c1, (uint16_t)PCF8563_Write, PCF8563_Address_Control_Status_1, 1 ,(uint8_t*)PCF_DataStruct, 16, 1000);

	//
	//?D??����?��??��y?��
	//
	if (PCF_DataStruct->Months_Century & PCF_Century_SetBitC)
	{
		*PCF_Century = 1;
	}
	else
	{
		*PCF_Century = 0;
	}
	//
	//?����??TD��??
	//
	PCF_DataStruct->Years          &= PCF8563_Shield_Years;
	PCF_DataStruct->Months_Century &= PCF8563_Shield_Months_Century;
	PCF_DataStruct->Days           &= PCF8563_Shield_Days;
	PCF_DataStruct->WeekDays       &= PCF8563_Shield_WeekDays;
	
	PCF_DataStruct->Hours          &= PCF8563_Shield_Hours;
	PCF_DataStruct->Minutes        &= PCF8563_Shield_Minutes;
	PCF_DataStruct->Seconds        &= PCF8563_Shield_Seconds;
	
	PCF_DataStruct->Minute_Alarm   &= PCF8563_Shield_Minute_Alarm;
	PCF_DataStruct->Hour_Alarm     &= PCF8563_Shield_Hour_Alarm;
	PCF_DataStruct->Day_Alarm      &= PCF8563_Shield_Day_Alarm;
	PCF_DataStruct->WeekDays_Alarm &= PCF8563_Shield_WeekDays_Alarm;
	
	//
	//?D??D����a��?��y?Y??��?
	//
	if (PCF_Format == PCF_Format_BIN)
	{
		PCF_DataStruct->Years           = RTC_Bcd2ToBin(PCF_DataStruct->Years);
		PCF_DataStruct->Months_Century  = RTC_Bcd2ToBin(PCF_DataStruct->Months_Century);
		PCF_DataStruct->Days            = RTC_Bcd2ToBin(PCF_DataStruct->Days);
		PCF_DataStruct->WeekDays        = RTC_Bcd2ToBin(PCF_DataStruct->WeekDays);
		
		PCF_DataStruct->Hours           = RTC_Bcd2ToBin(PCF_DataStruct->Hours);
		PCF_DataStruct->Minutes         = RTC_Bcd2ToBin(PCF_DataStruct->Minutes);
		PCF_DataStruct->Seconds         = RTC_Bcd2ToBin(PCF_DataStruct->Seconds);
		
		PCF_DataStruct->Day_Alarm       = RTC_Bcd2ToBin(PCF_DataStruct->Day_Alarm);
		PCF_DataStruct->WeekDays_Alarm  = RTC_Bcd2ToBin(PCF_DataStruct->WeekDays_Alarm);
		
		PCF_DataStruct->Hour_Alarm      = RTC_Bcd2ToBin(PCF_DataStruct->Hour_Alarm);
		PCF_DataStruct->Minute_Alarm    = RTC_Bcd2ToBin(PCF_DataStruct->Minute_Alarm);
	}
}

/**
   ============================================================================
                      #### ����??D??��2������1|?��o����y ####
   ============================================================================
**/

/**
  *****************************************************************************
  * @Name   : PCF8563D�䨨?����??D??��
  *
  * @Brief  : none
  *
  * @Input  : PCF_Format:     ��y?Y??��?
  *                           = PCF_Format_BIN
  *                           = PCF_Format_BCD
  *           PCF_DataStruct: ����???��11????
  *
  * @Output : none
  *
  * @Return : none
  *****************************************************************************
**/
void PCF8563_SetTime(unsigned char PCF_Format, _PCF8563_Time_Typedef* PCF_DataStruct)
{
	if (PCF_Format == PCF_Format_BIN)  //��???????��?��?D����a��a??��???
	{
		//
		//?D??��y?�̨�?��??����??��???��
		//
		if (PCF_DataStruct->RTC_Hours > 23)    PCF_DataStruct->RTC_Hours   = 0;  //???��0D?����
		if (PCF_DataStruct->RTC_Minutes > 59)  PCF_DataStruct->RTC_Minutes = 0;  //???��0��??��
		if (PCF_DataStruct->RTC_Seconds > 59)  PCF_DataStruct->RTC_Seconds = 0;  //???��0??
		//
		//D����a��a??��???
		//
		PCF_DataStruct->RTC_Hours   = RTC_BinToBcd2(PCF_DataStruct->RTC_Hours);
		PCF_DataStruct->RTC_Minutes = RTC_BinToBcd2(PCF_DataStruct->RTC_Minutes);
		PCF_DataStruct->RTC_Seconds = RTC_BinToBcd2(PCF_DataStruct->RTC_Seconds);
	}
	//
	//??���䨺y?Y
	//
	buffer[0] = PCF8563_Address_Seconds;
	buffer[1] = PCF_DataStruct->RTC_Seconds;
	buffer[2] = PCF_DataStruct->RTC_Minutes;
	buffer[3] = PCF_DataStruct->RTC_Hours;
	//
	//D�䨨?��y?Y��???��??��
	//
	//I2C_WriteBuf(PCF8563_Write, buffer, 4);
	HAL_I2C_Master_Transmit(&hi2c1, (uint16_t)PCF8563_Write, (uint8_t*)buffer, 4, 1000);

}

/**
  *****************************************************************************
  * @Name   : PCF8563?����?����??D??��
  *
  * @Brief  : none
  *
  * @Input  : PCF_Format:     ��y?Y??��?
  *                           = PCF_Format_BIN
  *                           = PCF_Format_BCD
  *
  * @Output : PCF_DataStruct: ����???��11????
  *
  * @Return : none
  *****************************************************************************
**/
unsigned char PCF8563_GetTime(unsigned char PCF_Format, _PCF8563_Time_Typedef* PCF_DataStruct)
{
	unsigned char ret=0;
	//
	//?����???��??�¨�y?��
	//
	//ret = I2C_ReadBuf(PCF8563_Write, PCF8563_Address_Seconds,buffer, 3);
	ret = HAL_I2C_Mem_Read(&hi2c1, (uint16_t)PCF8563_Write, PCF8563_Address_Seconds, 1 ,(uint8_t*)buffer, 3, 1000);
	if ( ret != 0 ) return 0;
	//
	//?����??TD��??
	//
	buffer[0] &= PCF8563_Shield_Seconds;
	buffer[1] &= PCF8563_Shield_Minutes;
	buffer[2] &= PCF8563_Shield_Hours;
	
	//
	//?D??D����a��?��y?Y??��?
	//
	if (PCF_Format == PCF_Format_BIN)
	{
		PCF_DataStruct->RTC_Hours   = RTC_Bcd2ToBin(buffer[2]);
		PCF_DataStruct->RTC_Minutes = RTC_Bcd2ToBin(buffer[1]);
		PCF_DataStruct->RTC_Seconds = RTC_Bcd2ToBin(buffer[0]);
	}
	// else if (PCF_Format == PCF_Format_BCD)
	// {
		// //
		// //??���䨺y?Y
		// //
		// PCF_DataStruct->RTC_Hours   = buffer[2];
		// PCF_DataStruct->RTC_Minutes = buffer[1];
		// PCF_DataStruct->RTC_Seconds = buffer[0];
	// }
	return 1;
}

/**
   ============================================================================
                         #### ��??��D??��2������1|?��o����y ####
   ============================================================================
**/

/**
  *****************************************************************************
  * @Name   : PCF8563D�䨨?��??��D??��
  *
  * @Brief  : D??����y?�̡�??�쨺?: 0 ~ 6��?��???????��?
  *
  * @Input  : PCF_Format:     ��y?Y??��?
  *                           = PCF_Format_BIN
  *                           = PCF_Format_BCD
  *           PCF_Century:    ����?��??����?��
  *                           = PCF_Century_19xx
  *                           = PCF_Century_20xx
  *           PCF_DataStruct: ��??��?��11????
  *
  * @Output : none
  *
  * @Return : none
  *****************************************************************************
**/
void PCF8563_SetDate(unsigned char PCF_Format, _PCF8563_Date_Typedef* PCF_DataStruct)
{
	if (PCF_Format == PCF_Format_BIN)  //��???????��?��?D����a��a??��???
	{
		//
		//?D??��y?�̨�?��??����??��???��
		//
		if (PCF_DataStruct->RTC_Years > 99)    PCF_DataStruct->RTC_Years    = 0;  //???��00?��
		if (PCF_DataStruct->RTC_Months > 12)   PCF_DataStruct->RTC_Months   = 1;  //???��1??
		if (PCF_DataStruct->RTC_Days > 31)     PCF_DataStruct->RTC_Days     = 1;  //???��1��?
		if (PCF_DataStruct->RTC_WeekDays > 6)  PCF_DataStruct->RTC_WeekDays = 1;  //???��D??����?
		//
		//D����a��a??��???
		//
		PCF_DataStruct->RTC_Years    = RTC_BinToBcd2(PCF_DataStruct->RTC_Years);
		PCF_DataStruct->RTC_Months   = RTC_BinToBcd2(PCF_DataStruct->RTC_Months);
		PCF_DataStruct->RTC_Days     = RTC_BinToBcd2(PCF_DataStruct->RTC_Days);
		PCF_DataStruct->RTC_WeekDays = RTC_BinToBcd2(PCF_DataStruct->RTC_WeekDays);
	}
	//
	//��y?Y??����
	//
	buffer[0] = PCF8563_Address_Days;
	buffer[1] = PCF_DataStruct->RTC_Days;
	buffer[2] = PCF_DataStruct->RTC_WeekDays;
	buffer[3] = PCF_DataStruct->RTC_Months;
	buffer[4] = PCF_DataStruct->RTC_Years;
	//
	//D�䨨?��y?Y��???��??��
	//
	//I2C_WriteBuf(PCF8563_Write, buffer, 5);
	HAL_I2C_Master_Transmit(&hi2c1, (uint16_t)PCF8563_Write, (uint8_t*)buffer, 5, 1000);

}

/**
  *****************************************************************************
  * @Name   : PCF8563?����?��??��D??��
  *
  * @Brief  : D??����y?�̡�??�쨺?: 0 ~ 6��?��???????��?
  *
  * @Input  : PCF_Format:     ��y?Y??��?
  *                           = PCF_Format_BIN
  *                           = PCF_Format_BCD
  *
  * @Output : 
  *           PCF_DataStruct: ��??��?��11????
  *
  * @Return : none
  *****************************************************************************
**/
void PCF8563_GetDate(unsigned char PCF_Format, _PCF8563_Date_Typedef* PCF_DataStruct)
{
	//
	//?����?��?2???��??�¨�y?��
	//
	//I2C_ReadBuf(PCF8563_Write, PCF8563_Address_Days, buffer, 4);
	HAL_I2C_Mem_Read(&hi2c1, (uint16_t)PCF8563_Write, PCF8563_Address_Days, 1 ,(uint8_t*)buffer, 4, 1000);
	//
	//?����??TD��??
	//
	buffer[0] &= PCF8563_Shield_Days;
	buffer[1] &= PCF8563_Shield_WeekDays;
	buffer[2] &= PCF8563_Shield_Months_Century;
	buffer[3] &= PCF8563_Shield_Years;
	
	//
	//?D??D����a��?��y?Y??��?
	//
	if (PCF_Format == PCF_Format_BIN)
	{
		PCF_DataStruct->RTC_Years    = RTC_Bcd2ToBin(buffer[3]);
		PCF_DataStruct->RTC_Months   = RTC_Bcd2ToBin(buffer[2]);
		PCF_DataStruct->RTC_Days     = RTC_Bcd2ToBin(buffer[0]);
		PCF_DataStruct->RTC_WeekDays = RTC_Bcd2ToBin(buffer[1]);		
	}
	else if (PCF_Format == PCF_Format_BCD)
	{
		//
		//??���䨺y?Y
		//
		PCF_DataStruct->RTC_Years    = buffer[3];
		PCF_DataStruct->RTC_Months   = buffer[2];
		PCF_DataStruct->RTC_Days     = buffer[0];
		PCF_DataStruct->RTC_WeekDays = buffer[1];
	}
}

/**
   ============================================================================
                         #### ??��?D??��2������1|?��o����y ####
   ============================================================================
**/

/**
  *****************************************************************************
  * @Name   : PCF8563D�䨨???��?D??��
  *
  * @Brief  : D??����y?�̡�??�쨺?: 0 ~ 6��?��???????��?
  *
  * @Input  : PCF_Format:     ��y?Y??��?
  *                           = PCF_Format_BIN
  *                           = PCF_Format_BCD
  *           PCF_DataStruct: ??��??��11????
  *
  * @Output : none
  *
  * @Return : none
  *****************************************************************************
**/
void PCF8563_SetAlarm(unsigned char PCF_Format, _PCF8563_Alarm_Typedef* PCF_DataStruct)
{
	unsigned char Alarm_State = 0;
	unsigned char Alarm_Interrupt = 0;  //????/���䨬???��??��??��??D???o��?
	
	if (PCF_Format == PCF_Format_BIN)  //��???????��?��?D����a��a??��???
	{
		//
		//?D??��y?�̨�?��??����??��???��
		//
		if (PCF_DataStruct->RTC_AlarmMinutes > 59)  PCF_DataStruct->RTC_AlarmMinutes  = 0;  //???��0��??��
		if (PCF_DataStruct->RTC_AlarmHours > 23)    PCF_DataStruct->RTC_AlarmHours    = 0;  //???��0D?����
		if (PCF_DataStruct->RTC_AlarmDays > 31)     PCF_DataStruct->RTC_AlarmDays     = 1;  //???��1��?
		if (PCF_DataStruct->RTC_AlarmWeekDays > 6)  PCF_DataStruct->RTC_AlarmWeekDays = 1;  //???��D??����?
		//
		//D����a��a??��???
		//
		PCF_DataStruct->RTC_AlarmMinutes  = RTC_BinToBcd2(PCF_DataStruct->RTC_AlarmMinutes);
		PCF_DataStruct->RTC_AlarmHours    = RTC_BinToBcd2(PCF_DataStruct->RTC_AlarmHours);
		PCF_DataStruct->RTC_AlarmDays     = RTC_BinToBcd2(PCF_DataStruct->RTC_AlarmDays);
		PCF_DataStruct->RTC_AlarmWeekDays = RTC_BinToBcd2(PCF_DataStruct->RTC_AlarmWeekDays);
	}
	//
	//?D??��?��??a????��?????
	//
	if (PCF_DataStruct->RTC_AlarmNewState == RTC_AlarmNewState_Open)  //??�䨰?a??��?
	{
		Alarm_State = 1;
	}
	else if (PCF_DataStruct->RTC_AlarmNewState == RTC_AlarmNewState_Open_INT_Enable)  //�䨰?a??��?2��䨰?a?D??��?3?
	{
		Alarm_State = 2;
	}
	else if (PCF_DataStruct->RTC_AlarmNewState == RTC_AlarmNewState_Close)  //1?��???��?��?2��?��1?��??D??��?3?
	{
		Alarm_State = 3;
	}
	//
	//?����?????/���䨬???��??��2?��
	//
	Alarm_Interrupt = PCF8563_Read_Byte(PCF8563_Address_Control_Status_2);
	Alarm_Interrupt &= PCF_Alarm_INT_Close;  //?��1?��??D??��?3?
	Alarm_Interrupt &= PCF_Control_ClearAF;;  //??3y����??
	PCF8563_Write_Byte(PCF8563_Address_Control_Status_2, Alarm_Interrupt);
	
	//
	//?��?Y?a??����D��??DD?����|2������
	//
	if (Alarm_State == 1 || Alarm_State == 2)  //�䨰?a??��?
	{
		if (PCF_DataStruct->RTC_AlarmType & RTC_AlarmType_Days)      PCF_DataStruct->RTC_AlarmDays     &= PCF_Alarm_DaysOpen;  //��??��??��?
		if (PCF_DataStruct->RTC_AlarmType & RTC_AlarmType_Hours)     PCF_DataStruct->RTC_AlarmHours    &= PCF_Alarm_HoursOpen;  //D?����??��?
		if (PCF_DataStruct->RTC_AlarmType & RTC_AlarmType_Minutes)   PCF_DataStruct->RTC_AlarmMinutes  &= PCF_Alarm_MinutesOpen;  //��??��??��?
		if (PCF_DataStruct->RTC_AlarmType & RTC_AlarmType_WeekDays)  PCF_DataStruct->RTC_AlarmWeekDays &= PCF_Alarm_WeekDaysOpen;  //��??��??��?
	}
	if (Alarm_State == 3)  //1?��?
	{
		if (PCF_DataStruct->RTC_AlarmType & RTC_AlarmType_Days)      PCF_DataStruct->RTC_AlarmDays     |= PCF_Alarm_DaysClose;  //��??��??��?
		if (PCF_DataStruct->RTC_AlarmType & RTC_AlarmType_Hours)     PCF_DataStruct->RTC_AlarmHours    |= PCF_Alarm_HoursClose;  //D?����??��?
		if (PCF_DataStruct->RTC_AlarmType & RTC_AlarmType_Minutes)   PCF_DataStruct->RTC_AlarmMinutes  |= PCF_Alarm_MinutesClose;  //��??��??��?
		if (PCF_DataStruct->RTC_AlarmType & RTC_AlarmType_WeekDays)  PCF_DataStruct->RTC_AlarmWeekDays |= PCF_Alarm_WeekDaysClose;  //��??��??��?
	}
	//
	//?D??��?��??a???D??��?3?
	//
	if (Alarm_State == 2)
	{
		Alarm_Interrupt |= PCF_Alarm_INT_Open;
		Alarm_Interrupt &= PCF_Control_ClearAF;;  //??3y����??
	}
	//
	//??���䨺y?Y
	//
	buffer[0] = PCF8563_Alarm_Minutes;
	buffer[1] = PCF_DataStruct->RTC_AlarmMinutes;
	buffer[2] = PCF_DataStruct->RTC_AlarmHours;
	buffer[3] = PCF_DataStruct->RTC_AlarmDays;
	buffer[4] = PCF_DataStruct->RTC_AlarmWeekDays;
	//
	//D�䨨?��y?Y��???��??��
	//
	//I2C_WriteBuf(PCF8563_Write, buffer, 5);
	HAL_I2C_Master_Transmit(&hi2c1, (uint16_t)PCF8563_Write, (uint8_t*)buffer, 5, 1000);

	//
	//D�䨨?????/���䨬???��??��2��y?��
	//
	buffer[0] = PCF8563_Address_Control_Status_2;
	buffer[1] = Alarm_Interrupt;
	//I2C_WriteBuf(PCF8563_Write, buffer, 2);
	HAL_I2C_Master_Transmit(&hi2c1, (uint16_t)PCF8563_Write, (uint8_t*)buffer, 2, 1000);

}

/**
  *****************************************************************************
  * @Name   : PCF8563?����???��?D??��
  *
  * @Brief  : D??����y?�̡�??�쨺?: 0 ~ 6��?��???????��?��???��?����??��???��???��??�¨�y?�̡�??a1????��?D??��?3?��2?���?2?����??
  *
  * @Input  : PCF_Format:     ��y?Y??��?
  *                           = PCF_Format_BIN
  *                           = PCF_Format_BCD
  *
  * @Output : PCF_DataStruct: ??��??��11????
  *
  * @Return : none
  *****************************************************************************
**/
void PCF8563_GetAlarm(unsigned char PCF_Format, _PCF8563_Alarm_Typedef* PCF_DataStruct)
{
	//
	//?����?��?2???��??�¨�y?��
	//
	//I2C_ReadBuf(PCF8563_Write, PCF8563_Alarm_Minutes,buffer, 4);
	HAL_I2C_Mem_Read(&hi2c1, (uint16_t)PCF8563_Write, PCF8563_Alarm_Minutes, 1 ,(uint8_t*)buffer, 4, 1000);
	//
	//?����??TD��??��?��??������?��?�̨�?2?����??
	//
	buffer[0] &= PCF8563_Shield_Minute_Alarm;  //��??������?��?��
	buffer[1] &= PCF8563_Shield_Hour_Alarm;  //D?��������?��?��
	buffer[2] &= PCF8563_Shield_Day_Alarm;  //��??������?��?��
	buffer[3] &= PCF8563_Shield_WeekDays_Alarm;  //D??������?��?��
	
	//
	//?D??D����a��?��y?Y??��?
	//
	if (PCF_Format == PCF_Format_BIN)
	{
		PCF_DataStruct->RTC_AlarmDays     = RTC_Bcd2ToBin(buffer[2]);
		PCF_DataStruct->RTC_AlarmHours    = RTC_Bcd2ToBin(buffer[1]);
		PCF_DataStruct->RTC_AlarmMinutes  = RTC_Bcd2ToBin(buffer[0]);
		PCF_DataStruct->RTC_AlarmWeekDays = RTC_Bcd2ToBin(buffer[3]);
	}
	else if (PCF_Format == PCF_Format_BCD)
	{
		//
		//??���䨺y?Y
		//
		PCF_DataStruct->RTC_AlarmDays     = buffer[2];
		PCF_DataStruct->RTC_AlarmHours    = buffer[1];
		PCF_DataStruct->RTC_AlarmMinutes  = buffer[0];
		PCF_DataStruct->RTC_AlarmWeekDays = buffer[3];
	}
}
/**
  *****************************************************************************
  * @Name   : PCF85633?��??��������?����??
  *
  * @Brief  : ?��PCF8563�̨���?��?1����¨�����?����??��???3?��?��?����??o����??��
  *
  * @Input  : NONE
  *
  * @Output : NONE
  *
  * @Return : none
  *****************************************************************************
**/  
void PCF8563_SetFunc(void) 
{ 
//    _PCF8563_Date_Typedef 	Date_InitStructure; 
    _PCF8563_Time_Typedef 	Timer_InitStruct;

  //  Date_InitStructure.RTC_Years    = 17; 
  //  Date_InitStructure.RTC_Months   = 1; 
  //  Date_InitStructure.RTC_Days     = 14; 
 //   Date_InitStructure.RTC_WeekDays = 6;

    Timer_InitStruct.RTC_Hours = HOUR; 
    Timer_InitStruct.RTC_Minutes = MIN; 
    Timer_InitStruct.RTC_Seconds = 02; 

    PCF8563_SetMode(PCF_Mode_Normal); 
    PCF8563_Stop(); 
    PCF8563_SetTime(PCF_Format_BIN,&Timer_InitStruct); 
  //  PCF8563_SetDate(PCF_Format_BIN,&Date_InitStructure); 
    PCF8563_Start(); 
} 


void Init_pcf8563(void)
{
	 PCF8563_Check();
  //PCF8563_SetFunc();
}
