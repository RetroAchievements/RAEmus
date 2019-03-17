/*
	EPOCH Super Cassette Vision Emulator 'eSCV'

	Author : Takeda.Toshiya
	Date   : 2006.08.21 -

	[ i/o ]
*/

#include "io.h"
#include "../upd7801.h"

void IO::initialize()
{
	key = emu->get_key_buffer();
	joy = emu->get_joy_buffer();
}

void IO::reset()
{
	pa = 0xff;
	pb = pc = 0;
}

void IO::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr) {
	case P_A:
		pa = data;
		break;
	case P_B:
		pb = data;
		break;
	case P_C:
		if((uint8_t)(data & 0x60) != (pc & 0x60)) {
			d_mem->write_io8(addr, data);
		}
		if((uint8_t)(data & 0x8) != (pc & 0x8)) {
			d_sound->write_io8(addr, data);
		}
		pc = data;
		break;
	}
}

uint32_t IO::read_io8(uint32_t addr)
{
	uint32_t val = 0xff;
	
	switch(addr) {
	case P_A:
		return pa;
	case P_B:
		if(!(pa & 0x01)) {
			if(joy[0] & 0x04) val &= ~0x01;	// P1-L
			if(joy[0] & 0x01) val &= ~0x02;	// P1-U
			if(joy[0] & 0x10) val &= ~0x04;	// P1-TR1
			if(joy[1] & 0x04) val &= ~0x08;	// P2-L
			if(joy[1] & 0x01) val &= ~0x10;	// P2-U
			if(joy[1] & 0x10) val &= ~0x20;	// P2-TR1
		}
		if(!(pa & 0x02)) {
			if(joy[0] & 0x02) val &= ~0x01;	// P1-D
			if(joy[0] & 0x08) val &= ~0x02;	// P1-R
			if(joy[0] & 0x20) val &= ~0x04;	// P1-TR2
			if(joy[1] & 0x02) val &= ~0x08;	// P2-D
			if(joy[1] & 0x08) val &= ~0x10;	// P2-R
			if(joy[1] & 0x20) val &= ~0x20;	// P2-TR2
		}
		if(!(pa & 0x04)) {
			if(key[0x30] || key[0x60]) val &= ~0x40;	// 0
			if(key[0x31] || key[0x61]) val &= ~0x80;	// 1
		}
		if(!(pa & 0x08)) {
			if(key[0x32] || key[0x62]) val &= ~0x40;	// 2
			if(key[0x33] || key[0x63]) val &= ~0x80;	// 3
		}
		if(!(pa & 0x10)) {
			if(key[0x34] || key[0x64]) val &= ~0x40;	// 4
			if(key[0x35] || key[0x65]) val &= ~0x80;	// 5
		}
		if(!(pa & 0x20)) {
			if(key[0x36] || key[0x66]) val &= ~0x40;	// 6
			if(key[0x37] || key[0x67]) val &= ~0x80;	// 7
		}
		if(!(pa & 0x40)) {
			if(key[0x38] || key[0x68]) val &= ~0x40;	// 8
			if(key[0x39] || key[0x69]) val &= ~0x80;	// 9
		}
		if(!(pa & 0x80)) {
			if(key[0x08]) val &= ~0x40;	// CL
			if(key[0x0d]) val &= ~0x80;	// EN
		}
		return val;
	case P_C:
		// pc0	in	pause
		// pc1	in	open
		// pc2	in	gnd
		// pc3	out	sound
		// pc4	out	open
		// pc5	out	cart (bank switch)
		// pc6	out	cart
		// pc7	in	cart
		return (pc & 0x78) | 0x82 | (key[0x20] ? 0 : 1);
	}
	return 0xff;
}

#define STATE_VERSION	2

bool IO::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(pa);
	state_fio->StateValue(pb);
	state_fio->StateValue(pc);
	return true;
}

