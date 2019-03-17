/*
	EPOCH Super Cassette Vision Emulator 'eSCV'

	Author : Takeda.Toshiya
	Date   : 2006.08.21 -

	[ i/o ]
*/

#ifndef _IO_H_
#define _IO_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class IO : public DEVICE
{
private:
	DEVICE* d_mem;
	DEVICE* d_sound;
	
	const uint8_t* key;
	const uint32_t* joy;
	uint8_t pa, pb, pc;
	
public:
	IO(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("I/O Bus"));
	}
	~IO() {}
	
	// common functions
	void initialize();
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_mem(DEVICE* device)
	{
		d_mem = device;
	}
	void set_context_sound(DEVICE* device)
	{
		d_sound = device;
	}
};

#endif
