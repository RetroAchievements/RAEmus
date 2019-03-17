/*
	EPOCH Super Cassette Vision Emulator 'eSCV'

	Author : Takeda.Toshiya
	Date   : 2006.08.21 -

	[ memory ]
*/

#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class MEMORY : public DEVICE
{
private:
	DEVICE* d_sound;
	
	// memory
	_TCHAR save_path[_MAX_PATH];
	
	struct {
		char id[4];	// SCV^Z
		uint8_t ctype;	// 0=16KB,32KB,32K+8KB ROM, bankswitched by PC5
				// 1=32KB ROM+8KB SRAM, bank switched by PC5
				// 2=32KB+32KB,32KB+32KB+32KB+32KB ROM, bank switched by PC5,PC6
				// 3=32KB+32KB ROM, bank switched by PC6
		uint8_t dummy[11];
	} header;
	bool inserted;
	uint32_t sram_crc32;
	
	uint8_t* wbank[0x200];
	uint8_t* rbank[0x200];
	uint8_t bios[0x1000];
	uint8_t vram[0x2000];
	uint8_t wreg[0x80];
	uint8_t cart[0x8000*4];
	uint8_t sram[0x2000];
	uint8_t wdmy[0x80];
	uint8_t rdmy[0x80];
	
	uint8_t cur_bank;
	void set_bank(uint8_t bank);
	
public:
	MEMORY(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Memory Bus"));
	}
	~MEMORY() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void write_data8(uint32_t addr, uint32_t data);
	uint32_t read_data8(uint32_t addr);
	void write_data8w(uint32_t addr, uint32_t data, int* wait);
	uint32_t read_data8w(uint32_t addr, int* wait);
	void write_io8(uint32_t addr, uint32_t data);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void open_cart(const _TCHAR* file_path);
	void close_cart();
	bool is_cart_inserted()
	{
		return inserted;
	}
	void set_context_sound(DEVICE* device)
	{
		d_sound = device;
	}
	uint8_t* get_font()
	{
		return bios + 0x200;
	}
	uint8_t* get_vram()
	{
		return vram;
	}
    uint8_t* get_sram()
    {
        return sram;
    }
};

#endif
