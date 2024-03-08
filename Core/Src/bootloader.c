#include "bootloader.h"
#include "main.h"
#include <stdint.h>
#include "common_macros.h"
#define CRC_SUCCESS 1U
#define CRC_FAIL 0U
extern CRC_HandleTypeDef hcrc;
extern UART_HandleTypeDef huart2;
uint8_t ExcuteMemoryWrite(uint8_t* Buffer,uint32_t Address,uint8_t Length)
{
	uint8_t ErrorStatus=HAL_OK;
	if(Address>=FLASH_BASE&&Address<=FLASH_END)
	{
		uint8_t Iterator ;
		HAL_FLASH_Unlock();
		for(Iterator=0;Iterator<Length;Iterator++)
		{
			ErrorStatus=HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, Address+Iterator, Buffer[Iterator]);
		}
		HAL_FLASH_Lock();
	}
	else
	{
		/*writing in sram*/
		uint8_t Iterator ;
		uint8_t* DestinationAddress=(uint8_t*)Address;
		for(Iterator=0;Iterator<Length;Iterator++)
			{
				DestinationAddress[Iterator]=Buffer[Iterator];
			}
	}
}
static uint8_t ValidateAddress(uint32_t Address)
{
	/*Address is valid with in flash or ram*/
	uint8_t state;
	if(Address>=FLASH_BASE&&Address<=FLASH_END)
	{
		state=VALID_ADDRESS;
	}
	else if(Address>=SRAM_BASE&&Address<=(SRAM_BASE+128*1024))
	{
		state=VALID_ADDRESS;
	}
	else
		state=INVALID_ADDRESS;
	return state;
}
static uint8_t excuteFlashErase(uint8_t sectorNumber,uint8_t NumberOfSectors)
{
	 HAL_StatusTypeDef ErrorState =HAL_OK;
	 if(NumberOfSectors>8 && NumberOfSectors!=0xFF )
	 {
		 ErrorState=HAL_ERROR;
	 }
	 else if(sectorNumber>7 && NumberOfSectors!=0xFF  )
	 {
		 ErrorState=HAL_ERROR;
	 }
	 else
	 {
		 FLASH_EraseInitTypeDef MyErease;
		 uint32_t SectorError;
		 if(sectorNumber==0xFF)
		 {
			 MyErease.TypeErase=FLASH_TYPEERASE_MASSERASE;

		 }
		 else
		 {
			 uint8_t RemaningSector =8-sectorNumber;
			 if(NumberOfSectors>RemaningSector)
			 {
				 NumberOfSectors=RemaningSector;
			 }
			 else
			 {

			 }
			 MyErease.TypeErase=FLASH_TYPEERASE_SECTORS;
			 MyErease.Sector=sectorNumber;
			 MyErease.NbSectors=NumberOfSectors;

		 }
		 MyErease.VoltageRange=FLASH_VOLTAGE_RANGE_3;
		 MyErease.Banks=FLASH_BANK_1;
		 HAL_FLASH_Unlock();
		 ErrorState =HAL_FLASHEx_Erase(&MyErease, &SectorError);
		 HAL_FLASH_Lock();
	 }
	 return ErrorState;
}
static uint8_t VerifyCrc(uint8_t* DataArr,uint8_t Length,uint32_t HostCRC)
{
	uint8_t Iterator;
	uint8_t CrcStatus;
	uint32_t AccumulateCRC,Temp;
	for(Iterator=0;Iterator<Length;Iterator++)
	{
		Temp =DataArr[Iterator];
		AccumulateCRC=HAL_CRC_Accumulate(&hcrc, &Temp, 1);
	}
	/*reset for crc*/
__HAL_CRC_DR_RESET(&hcrc);
if(AccumulateCRC==HostCRC)
{
	CrcStatus =CRC_SUCCESS;
}
else
{
	CrcStatus =CRC_FAIL;
}
	return CrcStatus;
}

static void SendAck(uint8_t ReplyLength)
{
	uint8_t AckBuffer[]={BL_ACK,ReplyLength};
	HAL_UART_Transmit(&huart2, AckBuffer,2, HAL_MAX_DELAY);
}
static void SendNack()
{
	uint8_t NackBuffer[]={BL_NACK};
	HAL_UART_Transmit(&huart2, NackBuffer,1, HAL_MAX_DELAY);
}

/*
 *this function is is used to get version of bootloader
 * */
void BL_HandleGetVerCmd(uint8_t* CmdPacket)
{
uint8_t BootLoaderVer,CrcStatus,CmdLength;
uint32_t HostCrc;
CmdLength =CmdPacket[0]+1;
HostCrc =*((uint32_t*)(CmdPacket+CmdLength-4));
CrcStatus =VerifyCrc(CmdPacket, CmdLength-4,HostCrc);
if(CrcStatus==CRC_SUCCESS)
{
	SendAck(1);
	BootLoaderVer =BL_VERSION;
	HAL_UART_Transmit(&huart2, &BootLoaderVer, 1, HAL_MAX_DELAY);
}
else
{
	SendNack();
}


}
void BL_HandleGetHelpCmd(uint8_t* CmdPacket)
{
	uint8_t CrcStatus,CmdLength;
	uint32_t HostCrc;
	CmdLength =CmdPacket[0]+1;
	HostCrc =*((uint32_t*)(CmdPacket+CmdLength-4));
	CrcStatus =VerifyCrc(CmdPacket, CmdLength-4,HostCrc);
	if(CrcStatus==CRC_SUCCESS)
	{
		uint8_t  BootLoaderCommands[]=
		{

				BL_GET_VER 			,
				BL_GET_HELP			,
				BL_GET_CID			,
				BL_GET_RDP_STATUS	,
				BL_GOTO_ADDR		,
				BL_FLASH_ERASE		,
				BL_MEM_WRITE		,
				BL_EN_WR_PROTECT	,
				BL_MEM_READ			,
				BL_READ_SECTOR_STATUS,
				BL_OTP_READ		    ,
				BL_DIS_WR_PROTECT
		};
		SendAck(sizeof(BootLoaderCommands));
		HAL_UART_Transmit(&huart2, BootLoaderCommands, sizeof(BootLoaderCommands),HAL_MAX_DELAY);
	}
	else
	{
		SendNack();
	}

}
void BL_HandleGetCIDCmd(uint8_t* CmdPacket)
{
	uint8_t CrcStatus,CmdLength;
		uint32_t HostCrc;
		CmdLength =CmdPacket[0]+1;
		HostCrc =*((uint32_t*)(CmdPacket+CmdLength-4));
		CrcStatus =VerifyCrc(CmdPacket, CmdLength-4,HostCrc);
		if(CrcStatus==CRC_SUCCESS)
		{
			uint16_t DeviceId =DBMCU_IDCODE &0X0FFF;
			SendAck(2);
			HAL_UART_Transmit(&huart2, (uint8_t*)&DeviceId, 2, HAL_MAX_DELAY);
		}
		else
		{
			SendNack();
		}

}
void BL_HandleGetRdpStatusCmd(uint8_t* CmdPacket)
{
	uint8_t CrcStatus,CmdLength;
		uint32_t HostCrc;
		CmdLength =CmdPacket[0]+1;
		HostCrc =*((uint32_t*)(CmdPacket+CmdLength-4));
		CrcStatus =VerifyCrc(CmdPacket, CmdLength-4,HostCrc);
		if(CrcStatus==CRC_SUCCESS)
		{
			uint8_t RdPStatus =(uint8_t)((RDP_USER_OPTION_WORD>>8)&&0xff);
			SendAck(1);
			HAL_UART_Transmit(&huart2, &RdPStatus, 1, HAL_MAX_DELAY);
		}
		else
		{
			SendNack();
		}

}
void BL_HandleGoToAddressCmd(uint8_t* CmdPacket)
{
	uint8_t CrcStatus,CmdLength;
		uint32_t HostCrc;
		CmdLength =CmdPacket[0]+1;
		HostCrc =*((uint32_t*)(CmdPacket+CmdLength-4));
		CrcStatus =VerifyCrc(CmdPacket, CmdLength-4,HostCrc);
		if(CrcStatus==CRC_SUCCESS)
		{
			uint8_t validState ;
			SendAck(1);
			/*abstract address*/
			uint32_t Address =*((uint32_t*)&CmdPacket[2]);
			validState =ValidateAddress(Address);
			if(validState==VALID_ADDRESS)
			{
				HAL_UART_Transmit(&huart2, &validState, 1,HAL_MAX_DELAY);
				void (*functionPtr)(void)=NULL;
				/*increament address by 1 to make t bit =1*/
				Address++;
				functionPtr=Address;
				functionPtr();
			}
		}
		else
		{
			SendNack();
		}

}
void BL_HandleFlashEraseCmd(uint8_t* CmdPacket)
{
	uint8_t CrcStatus,CmdLength;
		uint32_t HostCrc;
		CmdLength =CmdPacket[0]+1;
		HostCrc =*((uint32_t*)(CmdPacket+CmdLength-4));
		CrcStatus =VerifyCrc(CmdPacket, CmdLength-4,HostCrc);
		if(CrcStatus==CRC_SUCCESS)
		{
			uint8_t EreaseState;

			SendAck(1);

			EreaseState= excuteFlashErase(CmdPacket[2], CmdPacket[3]);
			HAL_UART_Transmit(&huart2, &EreaseState, 1, HAL_MAX_DELAY);

		}
		else
		{
			SendNack();
		}

}
void BL_HandleMemWriteCmd(uint8_t* CmdPacket)
{
	uint8_t CrcStatus,CmdLength;
		uint32_t HostCrc;
		CmdLength =CmdPacket[0]+1;
		HostCrc =*((uint32_t*)(CmdPacket+CmdLength-4));
		CrcStatus =VerifyCrc(CmdPacket, CmdLength-4,HostCrc);
		if(CrcStatus==CRC_SUCCESS)
		{
			SendAck(1);
			uint8_t WritingStatus;
			uint32_t Address =*((uint32_t*)&CmdPacket[2]);
			uint8_t AddressStatus =ValidateAddress(Address);
			if(AddressStatus==VALID_ADDRESS)
			{
				uint8_t PayloadLength =CmdPacket[6];
				WritingStatus=ExcuteMemoryWrite(&CmdPacket[7],Address,PayloadLength);
			}
			else
			{
				WritingStatus=1;
			}
			HAL_UART_Transmit(&huart2, &WritingStatus, 1, HAL_MAX_DELAY);

		}
		else
		{
			SendNack();
		}

}
void BL_HandleMemReadCmd(uint8_t* CmdPacket)
{
	uint8_t CrcStatus,CmdLength;
		uint32_t HostCrc;
		CmdLength =CmdPacket[0]+1;
		HostCrc =*((uint32_t*)(CmdPacket+CmdLength-4));
		CrcStatus =VerifyCrc(CmdPacket, CmdLength-4,HostCrc);
		if(CrcStatus==CRC_SUCCESS)
		{
			uint8_t Length;
			uint32_t Address,Iterator;
			Length=CmdPacket[6];
			Address= *((uint32_t*)&CmdPacket[2]);
			SendAck(Length);
			for(Iterator=0;Iterator<Length;Iterator++)
			{
				uint8_t Value=*((uint8_t*)Address+Iterator);
				HAL_UART_Transmit(&huart2, &Value, 1, HAL_MAX_DELAY);
			}

		}
		else
		{
			SendNack();
		}

}
void BL_HandleEnRwProtectionCmd(uint8_t* CmdPacket)
{
		uint8_t CrcStatus,CmdLength;
			uint32_t HostCrc;
			CmdLength =CmdPacket[0]+1;
			HostCrc =*((uint32_t*)(CmdPacket+CmdLength-4));
			CrcStatus =VerifyCrc(CmdPacket, CmdLength-4,HostCrc);
			if(CrcStatus==CRC_SUCCESS)
			{
				SendAck(1);
				if(CmdPacket[3]==1)
				{
					uint8_t status=0;
					/*WRITE PROTECTION*/
					HAL_FLASH_OB_Unlock();
					while(BIT_IS_SET(FLASH_SR_REG,16));
					CLEAR_BIT(OPTCR_REG,31);
					NWRP_Byte=~CmdPacket[2];
					SET_BIT(OPTCR_REG,1);
					while(BIT_IS_SET(FLASH_SR_REG,16));
					HAL_FLASH_OB_Lock();
					HAL_UART_Transmit(&huart2,&status, 1, HAL_MAX_DELAY);
				}
				else
				{

				}
			}
			else
			{
				SendNack();
			}
}
void BL_HandleReadSectorStatusCmd(uint8_t* CmdPacket)
{
	uint8_t CrcStatus,CmdLength,optionBytesStatus;
		uint32_t HostCrc;
		CmdLength =CmdPacket[0]+1;
		HostCrc =*((uint32_t*)(CmdPacket+CmdLength-4));
		CrcStatus =VerifyCrc(CmdPacket, CmdLength-4,HostCrc);
		if(CrcStatus==CRC_SUCCESS)
		{
			SendAck(1);
			optionBytesStatus=NWRP_WORD;
			HAL_UART_Transmit(&huart2, &optionBytesStatus, 1, HAL_MAX_DELAY);
		}
		else
		{
			SendNack();
		}

}
void BL_HandleOtpReadCmd(uint8_t* CmdPacket)
{
	uint8_t CrcStatus,CmdLength;
		uint32_t HostCrc;
		CmdLength =CmdPacket[0]+1;
		HostCrc =*((uint32_t*)(CmdPacket+CmdLength-4));
		CrcStatus =VerifyCrc(CmdPacket, CmdLength-4,HostCrc);
		if(CrcStatus==CRC_SUCCESS)
		{
			SendAck(1);
		}
		else
		{
			SendNack();
		}

}
void BL_HandleDisRwProtectionCmd(uint8_t* CmdPacket)
{
	uint8_t CrcStatus,CmdLength;
		uint32_t HostCrc;
		CmdLength =CmdPacket[0]+1;
		HostCrc =*((uint32_t*)(CmdPacket+CmdLength-4));
		CrcStatus =VerifyCrc(CmdPacket, CmdLength-4,HostCrc);
		if(CrcStatus==CRC_SUCCESS)
		{
			uint8_t status=0;
			SendAck(1);
			HAL_FLASH_OB_Unlock();
			while(BIT_IS_SET(FLASH_SR_REG,16));
			CLEAR_BIT(OPTCR_REG,31);
			OPTCR_REG|=(0xff<<16);
			SET_BIT(OPTCR_REG,1);
			while(BIT_IS_SET(FLASH_SR_REG,16));
			HAL_FLASH_OB_Lock();
			HAL_UART_Transmit(&huart2,&status, 1, HAL_MAX_DELAY);
		}
		else
		{
			SendNack();
		}

}

