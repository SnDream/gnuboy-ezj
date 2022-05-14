#ifndef __EZJ_H__
#define __EZJ_H__

#include "defs.h"
#include "mem.h"
#include "emu.h"
#include "hw.h"

#define EZJ_GET_U32(list, offset)  0  \
	| (list)[(offset) + 0] << (8 * 0) \
	| (list)[(offset) + 1] << (8 * 1) \
	| (list)[(offset) + 2] << (8 * 2) \
	| (list)[(offset) + 3] << (8 * 3);

#define SRAM2LIST(addr) ((addr) & 0x1FFF)

#define EZJ_SD_FILE			"./ezj_sd.dat"
#define EZJ_SRAM_FILE		"./ezj_sram.dat"
#define EZJ_STAGE1_FILE		"./stage1.gb"

#define SD_SECTOR_SIZE 512

#define EZJ_REG(reg) ((reg) & 0xFF)

#define EZJ_FW_VER (5)

#define eUNLK1          0x7f00
#define eUNLK1_MAGIC        0xe1
#define eUNLK2          0x7f10
#define eUNLK2_MAGIC        0xe2
#define eUNLK3          0x7f20
#define eUNLK3_MAGIC        0xe3

#define eSDSE           0x7f30
#define eSDSE_UNMAP         0x00
#define eSDSE_SECTOR        0x01
#define eSDSE_STATUS        0x03

#define eUNK1           0x7f31
#define eUNK2           0x7f32

#define eLDRSE          0x7f36
#define eLDRSE_UNMAP        0x00
#define eLDRSE_LDROM        0x01
#define eLDRSE_STATUS       0x03

#define eROMSE          0x7f37
#define eROMSE_NOMBC        0x00
#define eROMSE_MBC1         0x01
#define eROMSE_MBC2         0x02
#define eROMSE_MBC3         0x03
#define eROMSE_MBC3T        0x83
#define eROMSE_MBC5         0x04
#define eROMSE_FALLBACK     0x06

#define eSDSECTOR0      0x7fb0
#define eSDSECTOR1      0x7fb1
#define eSDSECTOR2      0x7fb2
#define eSDSECTOR3      0x7fb3

#define eSDWR           0x7fb4
#define eSDWR_R             0x00
#define eSDWR_R1            0x01
#define eSDWR_R2            0x02
#define eSDWR_R3            0x03
#define eSDWR_R4            0x04
#define eSDWR_W             0x80
#define eSDWR_W1            0x81

#define eSRAMSE         0x7fc0
#define eSRAMSE_UNMAP       0x00
#define eSRAMSE_UNK         0x02
#define eSRAMSE_SAVE        0x03
#define eSRAMSE_FW          0x04
#define eSRAMSE_FIUP        0x05
#define eSRAMSE_RTC         0x06

#define eROMBANKMASK0   0x7fc1
#define eROMBANKMASK1   0x7fc2

#define eROMHEADERSUM   0x7fc3

#define eSAVEBANKMASK   0x7fc4

#define eRTCW           0x7fd0
#define eRTCW_W             0x01

#define eFIUPSE         0x7fd2
#define eFIUPSE_UNMAP       0x00
#define eFIUPSE_STATUS      0x01

#define eGBTYPE         0x7fd3

#define eUNK3           0x7fd4

#define eRST            0x7fe0
#define eRST_EN             0x80

#define eLOCK1          0x7ff0
#define eLOCK1_MAGIC        0xe4

#define sSDSTATUS       0xa000
#define sSDSTATUS_DONE      0x01
#define sSDSTATUS_BUSY      0xe1

#define sSDSECTER       0xa000
#define sSDSECTER0      0xa000
#define sSDSECTER1      0xa800
#define sSDSECTER2      0xb000
#define sSDSECTER3      0xb800

#define sLDRSTAT        0xa000
#define sLDRSTAT_BUSY       0x00
#define sLDRSTAT_DONE       0x02

#define sLDRCMD00       0xa000
#define sLDRCMDSECT     0xa004
#define sLDRCMDSIZE     0xa1f0
#define sLDRCMD7D       0xa1f4
#define sLDRCMD7E       0xa1f8
#define sLDRCMD7F       0xa1fc

#define sSYSSAVEBAK     0xa000
#define sSYSSAVEBKN     0xa001
#define sSYSSAVEFLEN    0xa00f
#define sSYSSAVENAME    0xa010
#define sSYSAUTOSAVE    0xa200
#define sSYSCARTINIT    0xa201
#define sSYSROMNAME     0xa300

#define sRTCSEC         0xa008
#define sRTCMIN         0xa009
#define sRTCHOUR        0xa00a
#define sRTCDAY         0xa00b
#define sRTCWEEK        0xa00c
#define sRTCMONTH       0xa00d
#define sRTCYEAR        0xa00e

#define EZJ_STAGE1		0x01
#define EZJ_STAGE2		0x02
#define EZJ_STAGE3		0x03

struct ezj_mbc {
	word rombank;
	byte rambank;
	byte enableram;
	byte rtc;

	byte mbc1_sel;
};

struct ezj
{
	int activate;
	int enable;
	int lock;
	int reset;

	byte rom_stage1[0x2][0x4000];
	byte rom[0x200][0x4000];
	byte ram[64][0x2000];

	un64 sd_size;
	byte *sd_card;

	byte stage;
	byte rom_se;
	struct ezj_mbc mbc;

	byte sram_se;
	byte sram_sd_buf[4][512];
	byte sram_ldr_buf[0x2000];
	byte sram_unk_buf[0x2000];
	byte sram_fw;
	byte sram_fiup_buf[0x2000];

	byte sd_se;
	byte sd_busy;

	byte ldr_se;
	byte ldr_start;
	byte ldr_busy;

	byte unk1;
	byte unk2;
	byte unk3;

	un32 sd_sector;

	word rom_bank_mask;

	byte rom_header_sum;

	byte save_bank_mask;

	byte gb_type;
};

extern struct ezj ezj;

extern void ezj_init();
extern void ezj_reset();
extern void ezj_exit();
extern void ezj_write(int a, byte v);
extern byte ezj_read(int a);

#endif
