#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stddef.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "ezj.h"
#include "mem.h"

extern int debug_trace;

struct ezj ezj;

#define EZJ_DEBUG					1

#if EZJ_DEBUG
#define EZJ_D_REG_CALL				1
#define EZJ_D_REG_CALL_LOCK			0
#define EZJ_D_REG_CALL_SDWR			1
#define EZJ_D_WR					1
#define EZJ_D_LDR_LOAD_ROM			1

#define dprt(args...)				printf(args)
#if EZJ_D_REG_CALL
#define dprt_reg_call(args...)		printf(args)
#endif
#if EZJ_D_REG_CALL_LOCK
#define dprt_reg_call_lock(args...)	printf(args)
#endif
#if EZJ_D_REG_CALL_SDWR
#define dprt_reg_call_sdwr(args...)	printf(args)
#endif
#if EZJ_D_WR
#define dprt_wr(args...)			printf(args)
#endif
#if EZJ_D_LDR_LOAD_ROM
#define dprt_ldr_load_rom(args...)	printf(args)
#endif

#endif

#ifndef dprt
#define dprt(args...)
#endif
#ifndef dprt_reg_call
#define dprt_reg_call(args...)
#endif
#ifndef dprt_reg_call_lock
#define dprt_reg_call_lock(args...)
#endif
#ifndef dprt_reg_call_sdwr
#define dprt_reg_call_sdwr(args...)
#endif
#ifndef dprt_wr
#define dprt_wr(args...)
#endif
#ifndef dprt_ldr_load_rom
#define dprt_ldr_load_rom(args...)
#endif

void ezj_reg_call_eUNLK1(byte v)
{
	dprt_reg_call_lock("%s %02X\n", __func__, v);
	if (ezj.lock == 3 && v == eUNLK1_MAGIC) {
		ezj.lock--;
		return;
	}
	ezj.lock = 3;
}

void ezj_reg_call_eUNLK2(byte v)
{
	dprt_reg_call_lock("%s %02X\n", __func__, v);
	if (ezj.lock == 2 && v == eUNLK2_MAGIC) {
		ezj.lock--;
		return;
	}
	ezj.lock = 3;
}

void ezj_reg_call_eUNLK3(byte v)
{
	dprt_reg_call_lock("%s %02X\n", __func__, v);
	if (ezj.lock == 1 && v == eUNLK3_MAGIC) {
		ezj.lock--;
		return;
	}
	ezj.lock = 3;
}

void ezj_reg_call_eSDSE(byte v)
{
	dprt_reg_call("%s %02X\n", __func__, v);
	switch (v) {
	case eSDSE_UNMAP:
	case eSDSE_SECTOR:
	case eSDSE_STATUS:
		ezj.sd_se = v;
		break;
	default:
		return;
	}
}

void ezj_reg_call_eUNK1(byte v)
{
	dprt_reg_call("%s %02X\n", __func__, v);
	ezj.unk1 = v;
}

void ezj_reg_call_eUNK2(byte v)
{
	dprt_reg_call("%s %02X\n", __func__, v);
	switch (v) {
	case 0x80:
		ezj.stage = EZJ_STAGE2;
		break;
	case 0x00:
		ezj.stage = EZJ_STAGE3;
		break;
	}
	ezj.unk2 = v;
}

void ezj_reg_call_eLDRSE(byte v)
{
	dprt_reg_call("%s %02X\n", __func__, v);
	switch (v) {
	case eLDRSE_UNMAP:
	case eLDRSE_LDROM:
	case eLDRSE_STATUS:
		ezj.ldr_se = v;
		break;
	default:
		return;
	}
}

void ezj_reg_call_eROMSE(byte v)
{
	dprt_reg_call("%s %02X\n", __func__, v);
	switch (v) {
		case eROMSE_NOMBC:
		case eROMSE_MBC1:
		case eROMSE_MBC2:
		case eROMSE_MBC3:
		case eROMSE_MBC3T:
		case eROMSE_MBC5:
		case eROMSE_FALLBACK:
		ezj.rom_se = v;
	}
	return;
}

void ezj_reg_call_eSDSECTOR0(byte v)
{
	dprt_reg_call("%s %02X\n", __func__, v);
	ezj.sd_sector &= 0xFFFFFF00;
	ezj.sd_sector |= (un32)v << (0 * 8);
}

void ezj_reg_call_eSDSECTOR1(byte v)
{
	dprt_reg_call("%s %02X\n", __func__, v);
	ezj.sd_sector &= 0xFFFF00FF;
	ezj.sd_sector |= (un32)v << (1 * 8);
}

void ezj_reg_call_eSDSECTOR2(byte v)
{
	dprt_reg_call("%s %02X\n", __func__, v);
	ezj.sd_sector &= 0xFF00FFFF;
	ezj.sd_sector |= (un32)v << (2 * 8);
}

void ezj_reg_call_eSDSECTOR3(byte v)
{
	dprt_reg_call("%s %02X\n", __func__, v);
	ezj.sd_sector &= 0x00FFFFFF;
	ezj.sd_sector |= (un32)v << (3 * 8);
}


void ezj_reg_call_eSDWR(byte v)
{
	dprt_reg_call("%s %02X\n", __func__, v);

	byte num;
	un64 offset;
	num = v & 0x03;

	offset = ezj.sd_sector * SD_SECTOR_SIZE;
	if (offset >= ezj.sd_size) return;
	if ((offset + num * SD_SECTOR_SIZE) >= ezj.sd_size) return;

	switch (v & 0x80) {
	case 0x00:
		dprt_reg_call_sdwr("read sd %08llX, size %d\n", offset, num * SD_SECTOR_SIZE);
		memcpy(&ezj.sram_sd_buf, &ezj.sd_card[offset], num * SD_SECTOR_SIZE);
		break;
	case 0x80:
		dprt_reg_call_sdwr("write sd %08llX, size %d\n", offset, num * SD_SECTOR_SIZE);
		memcpy(&ezj.sd_card[offset], &ezj.sram_sd_buf, num * SD_SECTOR_SIZE);
		break;
	}

	ezj.sd_busy = (v & 0xf) * 4;
}

void ezj_reg_call_eSRAMSE(byte v)
{
	dprt_reg_call("%s %02X\n", __func__, v);
	ezj.sram_se = v;
}

void ezj_reg_call_eROMBANKMASK0(byte v)
{
	dprt_reg_call("%s %02X\n", __func__, v);
	ezj.rom_bank_mask &= 0x0100;
	ezj.rom_bank_mask |= ((word)v & 0xFF) << (0 * 8);
}

void ezj_reg_call_eROMBANKMASK1(byte v)
{
	dprt_reg_call("%s %02X\n", __func__, v);
	ezj.rom_bank_mask &= 0x00FF;
	ezj.rom_bank_mask |= ((word)v & 0x01) << (1 * 8);
}

void ezj_reg_call_eROMHEADERSUM(byte v)
{
	dprt_reg_call("%s %02X\n", __func__, v);
	ezj.rom_header_sum = v;
}

void ezj_reg_call_eSAVEBANKMASK(byte v)
{
	dprt_reg_call("%s %02X\n", __func__, v);
	ezj.save_bank_mask = v;
}

void ezj_reg_call_eRTCW(byte v)
{
	dprt_reg_call("%s %02X\n", __func__, v);
	return;
}

void ezj_reg_call_eFIUPSE(byte v)
{
	dprt_reg_call("%s %02X\n", __func__, v);
	return;
}

void ezj_reg_call_eGBTYPE(byte v)
{
	dprt_reg_call("%s %02X\n", __func__, v);
	ezj.gb_type = v;
}

void ezj_reg_call_eUNK3(byte v)
{
	dprt_reg_call("%s %02X\n", __func__, v);
	ezj.unk3 = v;
}

void ezj_reg_call_eRST(byte v)
{
	dprt_reg_call("%s %02X\n", __func__, v);
	ezj.stage = EZJ_STAGE3;
	ezj.reset = 0;
	emu_reset();
	return;
}

void ezj_reg_call_eLOCK1(byte v)
{
	dprt_reg_call_lock("%s %02X\n", __func__, v);
	if (ezj.lock == 0 && v == eLOCK1_MAGIC) {
		ezj.lock = 3;
	}
}

#define EZJ_REG_CALL(name) [EZJ_REG(name)] = ezj_reg_call_ ## name

void (*ezj_reg_call_list[256])(byte) = {
	EZJ_REG_CALL(eUNLK1),
	EZJ_REG_CALL(eUNLK2),
	EZJ_REG_CALL(eUNLK3),
	EZJ_REG_CALL(eSDSE),
	EZJ_REG_CALL(eUNK1),
	EZJ_REG_CALL(eUNK2),
	EZJ_REG_CALL(eLDRSE),
	EZJ_REG_CALL(eROMSE),
	EZJ_REG_CALL(eSDSECTOR0),
	EZJ_REG_CALL(eSDSECTOR1),
	EZJ_REG_CALL(eSDSECTOR2),
	EZJ_REG_CALL(eSDSECTOR3),
	EZJ_REG_CALL(eSDWR),
	EZJ_REG_CALL(eSRAMSE),
	EZJ_REG_CALL(eROMBANKMASK0),
	EZJ_REG_CALL(eROMBANKMASK1),
	EZJ_REG_CALL(eROMHEADERSUM),
	EZJ_REG_CALL(eSAVEBANKMASK),
	EZJ_REG_CALL(eRTCW),
	EZJ_REG_CALL(eFIUPSE),
	EZJ_REG_CALL(eGBTYPE),
	EZJ_REG_CALL(eUNK3),
	EZJ_REG_CALL(eRST),
	EZJ_REG_CALL(eLOCK1),
};

inline int ezj_reg_access(int a)
{
	if ((a & 0xFF00) != 0x7F00) return 0;
	switch (a) {
	case eUNLK1:
	case eUNLK2:
	case eUNLK3:
	case eLOCK1:
		return 1;
	default:
		return !(ezj.lock);
	}
}

void ezj_reg_call(int addr, byte value)
{
#if 0
	dprt_wr("EZJ %04X = %02X\n", addr, value);
#endif
	if (ezj_reg_call_list[EZJ_REG(addr)]) {
		ezj_reg_call_list[EZJ_REG(addr)](value);
	} else {
		dprt_wr("UNK REG\n");
		ezj_reg_call_eUNLK1(0);
	}
}

void ezj_ldr_load_rom()
{
	int i;
	un32 sector;
	un32 sector_num;
	un64 offset;
	un64 size;
	un32 target;

#if EZJ_D_LDR_LOAD_ROM
	for (i = 0; i < 512; i++) {
		dprt_ldr_load_rom("%02X ", ezj.sram_ldr_buf[i]);
		if ((i + 1) % 16 == 0) dprt_ldr_load_rom("\n");
	}
#endif

	target = EZJ_GET_U32(ezj.sram_ldr_buf, SRAM2LIST(sLDRCMD00));
	for (i = SRAM2LIST(sLDRCMDSECT); i < SRAM2LIST(sLDRCMDSIZE); i += 8) {
		sector = EZJ_GET_U32(ezj.sram_ldr_buf, i + 0);
		sector_num = EZJ_GET_U32(ezj.sram_ldr_buf, i + 4);
		offset = sector * SD_SECTOR_SIZE;
		if (sector_num == 0xffffffff) break;
		size = sector_num * SD_SECTOR_SIZE;
		dprt_ldr_load_rom("S%04X %04X %04X %04X\n", sector, sector_num, (un32)offset, (un32)size);
		if ((target + size) > (sizeof(ezj.rom))) {
			dprt_ldr_load_rom("out of target\n");
			break;
		}
		if ((offset + size) > (ezj.sd_size)) {
			dprt_ldr_load_rom("out of source\n");
			break;
		}
		dprt_ldr_load_rom("copy sd %08llX to rom %08X, size %lld\n", offset, target, size);
		memcpy(&(ezj.rom[0][target]), &(ezj.sd_card[offset]), size);
		target += size;
		ezj.ldr_busy++;
	}
	size = EZJ_GET_U32(ezj.sram_ldr_buf, SRAM2LIST(sLDRCMDSIZE)) - target;
	if ((target + size) > sizeof(ezj.rom)) {
		dprt_ldr_load_rom("out of target\n");
		return;
	}
	if ((offset + size) > (ezj.sd_size)) {
		dprt_ldr_load_rom("out of source\n");
		return;
	}
	dprt_ldr_load_rom("copy sd %08llX to rom %08X, size %lld\n", offset, target, size);
	memcpy(&(ezj.rom[0][target]), &(ezj.sd_card[offset]), size);
}

void ezj_load_stage1()
{
	int fd;
	ssize_t read_ret;

	memset(ezj.rom_stage1, 0xFF, sizeof(ezj.rom_stage1));
	fd = open(EZJ_STAGE1_FILE, O_RDONLY);
	if (fd >= 0) {
		read_ret = read(fd, ezj.rom_stage1, sizeof(ezj.rom_stage1));
		close(fd);
	}
}

void ezj_init()
{
	int ret, fd;
	un64 size;
	ssize_t read_ret;
	struct stat sd_stat;
	byte *sd_data;

	fd = open(EZJ_SD_FILE, O_RDWR);
	if (fd < 0) {
		exit(SIGTERM);
	}
	ret = fstat(fd, &sd_stat);
	size = sd_stat.st_size;
	if (ret != 0 || size < 512) {
		exit(SIGTERM);
	}
	sd_data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (sd_data == MAP_FAILED) {
		exit(SIGTERM);
	}
	close(fd);

	fd = open(EZJ_SRAM_FILE, O_RDONLY);
	if (fd >= 0) {
		read_ret = read(fd, ezj.ram, sizeof(ezj.ram));
		close(fd);
	}

	ezj_load_stage1();

	ezj.sd_size = size;
	ezj.sd_card = sd_data;

	ezj.stage = EZJ_STAGE1;

	ezj.sram_fw = EZJ_FW_VER;

	ezj.enable = 1;
	ezj.reset = 1;
}

void ezj_reset()
{
	if (ezj.reset) {
		ezj.lock = 3;

		ezj.sram_se = eSRAMSE_UNMAP;
		memset(ezj.rom, 0xFF, sizeof(ezj.rom));
		memset(ezj.sram_sd_buf, 0xFF, sizeof(ezj.sram_sd_buf));
		memset(ezj.sram_ldr_buf, 0xFF, sizeof(ezj.sram_ldr_buf));
		memset(ezj.sram_unk_buf, 0xFF, sizeof(ezj.sram_unk_buf));
		memset(ezj.sram_fiup_buf, 0xFF, sizeof(ezj.sram_fiup_buf));

		ezj.unk1 = 0;
		ezj.unk2 = 0;
		ezj.unk3 = 0;

		ezj.stage = EZJ_STAGE1;
		ezj.sram_se = eSRAMSE_UNMAP;
		ezj.rom_se = eROMSE_NOMBC;

		ezj.rom_bank_mask = 0x1FF;
		ezj.save_bank_mask = 0x3F;
		
	} else {
		ezj.reset = 1;
	}

	ezj.sd_se = 0;
	ezj.sd_busy = 0;

	ezj.ldr_se = 0;
	ezj.ldr_busy = 0;

	ezj.sd_sector = 0;

	ezj.rom_header_sum = 0;

	ezj.mbc.rombank = 0x01;
	ezj.mbc.rambank = 0x00;
	ezj.mbc.enableram = 0x00;
	ezj.mbc.rtc = 0x00;

	byte c = ezj_read(0x0143);
	hw.cgb = ((c == 0x80) || (c == 0xc0));
}

void ezj_exit()
{
	int fd;
	ssize_t write_ret;
	munmap(ezj.sd_card, ezj.sd_size);
	fd = open(EZJ_SRAM_FILE, O_WRONLY | O_CREAT, 0644);
	if (fd >= 0) {
		write_ret = write(fd, ezj.ram, sizeof(ezj.ram));
	}
}

void ezj_write_sys(int a, byte v)
{
	int ra;
	byte ha = (a>>12) & 0xF;

	if (ezj_reg_access(a)) {
		return ezj_reg_call(a, v);
	}
	switch(ha) {
	case 0x2:
		ezj.mbc.rombank = (ezj.mbc.rombank & 0x100) | (((word)v << 0) & 0x0FF);
		return;
	case 0x3:
		ezj.mbc.rombank = (ezj.mbc.rombank & 0x0FF) | (((word)v << 0) & 0x100);
		return;
	case 0x4:
	case 0x5:
		ezj.mbc.rambank = v & 0x3F;
		return;
	case 0xA:
	case 0xB:
		ra = a & 0x1FFF;
		if (ezj.sd_se) {
			switch (ezj.sd_se) {
			case eSDSE_SECTOR:
				// dprt_wr("SDST w%04X = %02X\n", a, v);
				ezj.sram_sd_buf[0][ra] = v;
				return;
			case eSDSE_STATUS:
				ezj.sram_sd_buf[0][ra] = v;
				// dprt_wr("SDAT w%04X = %02X\n", a, v);
				return;
			}
		} else if (ezj.ldr_se) {
			switch (ezj.ldr_se) {
			case eLDRSE_LDROM:
				ezj.sram_ldr_buf[ra] = v;
				ezj.ldr_start = 1;
				return;
			}
		} else if (ezj.sram_se) {
			switch (ezj.sram_se) {
			case eSRAMSE_UNK:
				ezj.sram_unk_buf[ra] = v;
				return;
			case eSRAMSE_SAVE:
				ezj.ram[ezj.mbc.rambank][ra] = v;
				// dprt_wr("SAVE r%04X = %02X, %02X\n", a, v, ezj.mbc.rambank);
				return;
			case eSRAMSE_FW:
				return;
			case eSRAMSE_FIUP:
				ezj.sram_fiup_buf[ra] = v;
				return;
			case eSRAMSE_RTC:
				return;
			}
		}
		dprt_wr("UNKA %04X = %02X\n", a, v);
		return;
	default:
		dprt_wr("UNKW %04X = %02X\n", a, v);
		return;
	}
}

void ezj_write_mbc(int a, byte v)
{
	int ra;
	byte ha = a>>12;

	if (ezj_reg_access(a)) {
		return ezj_reg_call(a, v);
	}
	switch(ezj.rom_se) {
	case eROMSE_NOMBC:
	case eROMSE_FALLBACK:
		return;
	case eROMSE_MBC1:
		switch (ha & 0xE) {
		case 0x0:
			ezj.mbc.enableram = ((v & 0x0F) == 0x0A);
			return;
		case 0x2:
			v &= 0x1F;
			ezj.mbc.rombank = (v ? v : 0x01) | (ezj.mbc.rombank & 0x60);
			ezj.mbc.rombank &= ezj.rom_bank_mask;
			return;
		case 0x4:
			v &= 0x03;
			if (ezj.mbc.mbc1_sel) {
				ezj.mbc.rambank = v & ezj.save_bank_mask;
			} else {
				mbc.rombank = (mbc.rombank & 0x1F) | ((word)v << 5);
				mbc.rombank &= ezj.rom_bank_mask;
				return;
			}
			return;
		case 0x6:
			ezj.mbc.mbc1_sel = v & 0x1;
			return;
		case 0xA:
			if (ezj.mbc.enableram) {
				ezj.ram[ezj.mbc.rambank][a & 0x1FFF] = v;
			}
			return;
		}
	case eROMSE_MBC2:
		switch (ha & 0xC) {
		case 0x0:
			if (a & 0x0100) {
				ezj.mbc.enableram = ((v & 0x0F) == 0x0A);
				return;
			} else {
				ezj.mbc.rombank = (v & 0x0F) & ezj.rom_bank_mask;
				return;
			}
		case 0xA:
			if (ezj.mbc.enableram) {
				ezj.ram[ezj.mbc.rambank][a & 0x1FFF] = v;
			}
			return;
		}
	case eROMSE_MBC3:
		switch (ha & 0xE) {
		case 0x0:
			ezj.mbc.enableram = ((v & 0x0F) == 0x0A);
			return;
		case 0x2:
			v &= 0x7F;
			ezj.mbc.rombank = (v ? v : 0x01) & ezj.rom_bank_mask ;
			return;
		case 0x4:
			ezj.mbc.rambank = (v & 0x0F) & ezj.save_bank_mask;
			return;
		case 0xA:
			if (ezj.mbc.enableram) {
				ezj.ram[ezj.mbc.rambank][a & 0x1FFF] = v;
			}
			return;
		}
	case eROMSE_MBC3T:
		switch (ha & 0xE) {
		case 0x0:
			ezj.mbc.enableram = ((v & 0x0F) == 0x0A);
			return;
		case 0x2:
			v &= 0x7F;
			ezj.mbc.rombank = (v ? v : 0x01) & ezj.rom_bank_mask ;
			return;
		case 0x4:
			/* TODO RTC */
			ezj.mbc.rambank = (v & 0x0F) & ezj.save_bank_mask;
			return;
		case 0x6:
			/* TODO RTC */
			return;
		case 0xA:
			if (ezj.mbc.enableram) {
				ezj.ram[ezj.mbc.rambank][a & 0x1FFF] = v;
			}
			return;
		}
	case eROMSE_MBC5:
		switch (ha & 0xF) {
		case 0x0:
		case 0x1:
			ezj.mbc.enableram = ((v & 0x0F) == 0x0A);
			return;
		case 0x2:
			ezj.mbc.rombank = (ezj.mbc.rombank & 0x100) | (((word)v << 0) & 0x0FF);
			ezj.mbc.rombank &= ezj.rom_bank_mask;
			return;
		case 0x3:
			ezj.mbc.rombank = (ezj.mbc.rombank & 0x0FF) | (((word)v << 8) & 0x100);
			ezj.mbc.rombank &= ezj.rom_bank_mask;
			return;
		case 0x4:
		case 0x5:
			ezj.mbc.rambank = (v & 0x0F) & ezj.save_bank_mask;
			return;
		case 0xA:
			if (ezj.mbc.enableram) {
				ezj.ram[ezj.mbc.rambank][a & 0x1FFF] = v;
			}
			return;
		}
	}
	dprt_wr("UNKW %04X = %02X\n", a, v);
	return;
}

byte ezj_read_sys(int a)
{
	byte ha = (a>>12) & 0xE;
	int ra;

	switch (ha) {
	case 0x0:
	case 0x2:
			return ezj.stage == EZJ_STAGE1 ? ezj.rom_stage1[0][a] : ezj.rom[0][a];
	case 0x4:
	case 0x6:
			return ezj.stage == EZJ_STAGE1 ? ezj.rom_stage1[0][a] : ezj.rom[ezj.mbc.rombank][a & 0x3FFF];
	case 0xA:
		ra = a & 0x1FFF;
		if (ezj.sd_se) {
			switch (ezj.sd_se) {
			case eSDSE_SECTOR:
				// dprt_wr("SDAT r%04X = %02X\n", a, ezj.sram_sd_buf[0][ra]);
				return ezj.sram_sd_buf[0][ra];
			case eSDSE_STATUS:
				// dprt_wr("SDAT r%04X , %02X\n", a, ezj.sd_busy);
				if (ezj.sd_busy) ezj.sd_busy--;
				return ezj.sd_busy ? sSDSTATUS_BUSY : sSDSTATUS_DONE;
			}
		} else if (ezj.ldr_se) {
			switch (ezj.ldr_se) {
			case eLDRSE_LDROM:
				return ezj.sram_ldr_buf[ra];
			case eLDRSE_STATUS:
				if (ezj.ldr_start) {
					ezj_ldr_load_rom();
					ezj.ldr_start = 0;
				}
				if (ezj.ldr_busy) ezj.ldr_busy--;
				return ezj.ldr_busy ? sLDRSTAT_BUSY : sLDRSTAT_DONE;
			}
		} else if (ezj.sram_se) {
			switch (ezj.sram_se) {
			case eSRAMSE_UNK:
				return ezj.sram_unk_buf[ra];
			case eSRAMSE_SAVE:
				dprt_wr("SAVE r%04X = %02X, %02X\n", a, ezj.ram[ezj.mbc.rambank][ra], ezj.mbc.rambank);
				return ezj.ram[ezj.mbc.rambank][ra];
			case eSRAMSE_FW:
				return ezj.sram_fw;
			case eSRAMSE_FIUP:
				return ezj.sram_fiup_buf[ra];
			case eSRAMSE_RTC:
				switch(a) {
				case sRTCSEC: return 0x00;
				case sRTCMIN: return 0x27;
				case sRTCHOUR: return 0x01;
				case sRTCDAY: return 0x14;
				case sRTCWEEK: return 0x06;
				case sRTCMONTH: return 0x05;
				case sRTCYEAR: return 0x22;
				default: return 0xFF;
				}
			}
		}
		dprt_wr("UNKA r%04X\n", a);
		return 0xFF;
	default:
		dprt_wr("UNKR r%04X\n", a);
		return 0xFF;
	}
}

byte ezj_read_mbc(int a)
{
	byte ha = (a>>12) & 0xE;
	int ra;

	switch (ha) {
	case 0x0:
	case 0x2:
		return ezj.rom[0][a];
	case 0x4:
	case 0x6:
		return ezj.rom[ezj.mbc.rombank][a & 0x3FFF];
	case 0xA:
		return ezj.mbc.enableram ? ezj.ram[ezj.mbc.rambank][a & 0x1FFF] : 0xFF;
	default:
		dprt_wr("UNKR r%04X\n", a);
		return 0xFF;
	}
}

void ezj_write(int a, byte v)
{
	a &= 0xFFFF;
	switch (ezj.stage) {
	case EZJ_STAGE1:
	case EZJ_STAGE2:
		return ezj_write_sys(a, v);
	case EZJ_STAGE3:
		return ezj_write_mbc(a, v);
	}
}


byte ezj_read(int a)
{
	a &= 0xFFFF;
	switch (ezj.stage) {
	case EZJ_STAGE1:
	case EZJ_STAGE2:
		return ezj_read_sys(a);
	case EZJ_STAGE3:
		return ezj_read_mbc(a);
	}
}
