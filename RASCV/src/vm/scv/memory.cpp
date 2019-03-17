/*
	EPOCH Super Cassette Vision Emulator 'eSCV'

	Author : Takeda.Toshiya
	Date   : 2006.08.21 -

	[ memory ]
*/

#include "memory.h"

#define MEM_WAIT_VDP 0

#define SET_BANK(s, e, w, r) { \
	int sb = (s) >> 7, eb = (e) >> 7; \
	for(int i = sb; i <= eb; i++) { \
		if((w) == wdmy) { \
			wbank[i] = wdmy; \
		} else { \
			wbank[i] = (w) + 0x80 * (i - sb); \
		} \
		if((r) == rdmy) { \
			rbank[i] = rdmy; \
		} else { \
			rbank[i] = (r) + 0x80 * (i - sb); \
		} \
	} \
}

void MEMORY::initialize()
{
	memset(bios, 0xff, sizeof(bios));
	memset(cart, 0xff, sizeof(cart));
	memset(sram, 0xff, sizeof(sram));
	memset(rdmy, 0xff, sizeof(rdmy));
	
	// load bios
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("BIOS.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(bios, 0x1000, 1);
		fio->Fclose();
	}
	delete fio;
	
	// $0000-$0fff : cpu internal rom
	// $2000-$3fff : vram
	// $8000-$ff7f : cartridge rom
	// ($e000-$ff7f : 8kb sram)
	// $ff80-$ffff : cpu internam ram
	SET_BANK(0x0000, 0x0fff, wdmy, bios);
	SET_BANK(0x1000, 0x1fff, wdmy, rdmy);
	SET_BANK(0x2000, 0x3fff, vram, vram);
	SET_BANK(0x4000, 0x7fff, wdmy, rdmy);
	SET_BANK(0x8000, 0xff7f, wdmy, cart);
	SET_BANK(0xff80, 0xffff, wreg, wreg);
	
	// cart is not opened
	memset(&header, 0, sizeof(header));
	inserted = false;
}

void MEMORY::release()
{
	close_cart();
}

void MEMORY::reset()
{
	// clear memory
	memset(vram, 0, sizeof(vram));
	for(int i = 0x1000; i < 0x1200; i += 32) {
		static const uint8_t tmp[32] = {
			0x00,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
			0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff
		};
		memcpy(vram + i, tmp, 32);
	}
	memset(wreg, 0, sizeof(wreg));
	
	// initialize memory bank
	set_bank(0);
}

void MEMORY::write_data8(uint32_t addr, uint32_t data)
{
	addr &= 0xffff;
	if(addr == 0x3600) {
		d_sound->write_data8(addr, data);
	}
	if((addr & 0xfe00) == 0x3400) {
		wbank[0x68][addr & 3] = data;
	} else {
		wbank[addr >> 7][addr & 0x7f] = data;
	}
}

uint32_t MEMORY::read_data8(uint32_t addr)
{
	addr &= 0xffff;
	return rbank[addr >> 7][addr & 0x7f];
}

void MEMORY::write_data8w(uint32_t addr, uint32_t data, int* wait)
{
	*wait = (0x2000 <= addr && addr < 0x3600) ? MEM_WAIT_VDP : 0;
	write_data8(addr, data);
}

uint32_t MEMORY::read_data8w(uint32_t addr, int* wait)
{
	*wait = (0x2000 <= addr && addr < 0x3600) ? MEM_WAIT_VDP : 0;
	return read_data8(addr);
}

void MEMORY::write_io8(uint32_t addr, uint32_t data)
{
	set_bank((data >> 5) & 3);
}

void MEMORY::set_bank(uint8_t bank)
{
//	if(cur_bank != bank) {
		SET_BANK(0x8000, 0xff7f, wdmy, cart + 0x8000 * bank);
		if(header.ctype == 1 && (bank & 1)) {
			SET_BANK(0xe000, 0xff7f, sram, sram);
		}
		cur_bank = bank;
//	}
}

void MEMORY::open_cart(const _TCHAR* file_path)
{
	// close cart and initialize memory
	close_cart();
	
	// get save file path
	my_tcscpy_s(save_path, _MAX_PATH, file_path);
	int len = _tcslen(save_path);
	if(save_path[len - 4] == _T('.')) {
		save_path[len - 3] = _T('S');
		save_path[len - 2] = _T('A');
		save_path[len - 1] = _T('V');
	} else {
		my_stprintf_s(save_path, _MAX_PATH, _T("%s.SAV"), file_path);
	}
	
	// open cart and backuped sram
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		// load header
		fio->Fread(&header, sizeof(header), 1);
		if(!(header.id[0] == 'S' && header.id[1] == 'C' && header.id[2] == 'V' && header.id[3] == 0x1a)) {
			// failed to load header
			memset(&header, 0, sizeof(header));
			fio->Fseek(0, FILEIO_SEEK_SET);
		}
		
		// load rom image, PC5=0, PC6=0
		fio->Fread(cart, 0x8000, 1);
		memcpy(cart + 0x8000, cart, 0x8000);
		
		// load rom image, PC5=1, PC6=0
		if(header.ctype == 0) {
			fio->Fread(cart + 0xe000, 0x2000, 1);
		} else if(header.ctype == 2) {
			fio->Fread(cart + 0x8000, 0x8000, 1);
		}
		memcpy(cart + 0x10000, cart, 0x10000);
		
		// load rom image, PC5=0/1, PC6=1
		if(header.ctype == 2) {
			fio->Fread(cart + 0x10000, 0x10000, 1);
		} else if(header.ctype == 3) {
			fio->Fread(cart + 0x10000, 0x8000, 1);
			memcpy(cart + 0x18000, cart + 0x10000, 0x8000);
		}
		fio->Fclose();
		
		// load backuped sram
		// note: shogi nyumon has sram but it is not battery-backuped
		if(header.ctype == 1 && cart[1] != 0x48) {
			if(fio->Fopen(save_path, FILEIO_READ_BINARY)) {
				fio->Fread(sram, sizeof(sram), 1);
				fio->Fclose();
			}
			sram_crc32 = get_crc32(sram, sizeof(sram));
		}
		inserted = true;
	}
	delete fio;
}

void MEMORY::close_cart()
{
	// save backuped sram
	// note: shogi nyumon has sram but it is not battery-backuped
	if(inserted && header.ctype == 1 && cart[1] != 0x48) {
		if(sram_crc32 != get_crc32(sram, sizeof(sram))) {
			FILEIO* fio = new FILEIO();
			if(fio->Fopen(save_path, FILEIO_WRITE_BINARY)) {
				fio->Fwrite(sram, 0x2000, 1);
				fio->Fclose();
			}
			delete fio;
		}
	}
	inserted = false;
	
	// initialize memory
	memset(&header, 0, sizeof(header));
	memset(cart, 0xff, sizeof(cart));
	memset(sram, 0xff, sizeof(sram));
}

#define STATE_VERSION	2

bool MEMORY::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateArray(save_path, sizeof(save_path), 1);
	state_fio->StateArray(header.id, sizeof(header.id), 1);
	state_fio->StateValue(header.ctype);
	state_fio->StateArray(header.dummy, sizeof(header.dummy), 1);
	state_fio->StateValue(inserted);
	state_fio->StateValue(sram_crc32);
	state_fio->StateArray(vram, sizeof(vram), 1);
	state_fio->StateArray(wreg, sizeof(wreg), 1);
	state_fio->StateArray(sram, sizeof(sram), 1);
	state_fio->StateValue(cur_bank);
	
	// post process
	if(loading) {
		set_bank(cur_bank);
	}
	return true;
}

