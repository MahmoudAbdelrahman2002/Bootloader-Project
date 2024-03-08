/*
 * bootloader.h
 *
 *  Created on: Feb 23, 2024
 *      Author: Hp
 */

#ifndef INC_BOOTLOADER_H_
#define INC_BOOTLOADER_H_
#include "stdint.h"


#define BL_GET_VER 				0X51
#define BL_GET_HELP				0X52
#define BL_GET_CID				0X53
#define BL_GET_RDP_STATUS		0x54
#define BL_GOTO_ADDR			0x55
#define BL_FLASH_ERASE			0x56
#define BL_MEM_WRITE			0x57
#define BL_EN_WR_PROTECT		0x58
#define BL_MEM_READ				0x59
#define BL_READ_SECTOR_STATUS 	0X5A
#define BL_OTP_READ		    	0x5B
#define BL_DIS_WR_PROTECT   	0X5C
#define BL_ACK   0XA5
#define BL_NACK   0X7F
#define BL_VERSION 1U
#define DBMCU_IDCODE  (*(volatile uint32_t*)0XE0042000)
#define RDP_USER_OPTION_WORD (*(volatile uint32_t*)0X1FFFC000)
#define NWRP_WORD (*(volatile uint8_t*)0x1FFFC008)
#define OPTCR_REG (*(volatile uint32_t*)0x40023C14)
#define NWRP_Byte (*(volatile uint8_t*)0x40023C16)
#define FLASH_SR_REG (*(volatile uint32_t*)0x40023C0c)
#define VALID_ADDRESS 0U
#define INVALID_ADDRESS 1U


void BL_HandleGetVerCmd(uint8_t* CmdPacket);
void BL_HandleGetHelpCmd(uint8_t* CmdPacket);
void BL_HandleGetCIDCmd(uint8_t* CmdPacket);
void BL_HandleGetRdpStatusCmd(uint8_t* CmdPacket);
void BL_HandleGoToAddressCmd(uint8_t* CmdPacket);
void BL_HandleFlashEraseCmd(uint8_t* CmdPacket);
void BL_HandleMemWriteCmd(uint8_t* CmdPacket);
void BL_HandleMemReadCmd(uint8_t* CmdPacket);
void BL_HandleEnRwProtectionCmd(uint8_t* CmdPacket);
void BL_HandleReadSectorStatusCmd(uint8_t* CmdPacket);
void BL_HandleOtpReadCmd(uint8_t* CmdPacket);
void BL_HandleDisRwProtectionCmd(uint8_t* CmdPacket);









#endif /* INC_BOOTLOADER_H_ */
