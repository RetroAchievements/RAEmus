/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2008.12.29 -

	[ i/o bus ]
*/

#ifndef _IO_H_
#define _IO_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

#ifndef IO_ADDR_MAX
#define IO_ADDR_MAX 0x100
#endif

class IO : public DEVICE
{
private:
	// i/o map
	typedef struct {
		DEVICE* dev;
		uint32_t addr;
		int wait;
		bool is_flipflop;
	} wr_bank_t;
	
	typedef struct {
		DEVICE* dev;
		uint32_t addr;
		int wait;
		bool value_registered;
		uint32_t value;
	} rd_bank_t;
	
	wr_bank_t *wr_table;
	rd_bank_t *rd_table;
	
	void write_port8(uint32_t addr, uint32_t data, bool is_dma);
	uint32_t read_port8(uint32_t addr, bool is_dma);
	void write_port16(uint32_t addr, uint32_t data, bool is_dma);
	uint32_t read_port16(uint32_t addr, bool is_dma);
	void write_port32(uint32_t addr, uint32_t data, bool is_dma);
	uint32_t read_port32(uint32_t addr, bool is_dma);
	
public:
	IO(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
#ifdef _IO_DEBUG_LOG
		cpu_index = 0;
#endif
		addr_max = IO_ADDR_MAX;
		
		wr_table = NULL;
		rd_table = NULL;
		
		set_device_name(_T("Generic I/O Bus"));
	}
	~IO() {}
	
	// common functions
	void initialize();
	void release();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_io16(uint32_t addr, uint32_t data);
	uint32_t read_io16(uint32_t addr);
	void write_io32(uint32_t addr, uint32_t data);
	uint32_t read_io32(uint32_t addr);
	void write_io8w(uint32_t addr, uint32_t data, int* wait);
	uint32_t read_io8w(uint32_t addr, int* wait);
	void write_io16w(uint32_t addr, uint32_t data, int* wait);
	uint32_t read_io16w(uint32_t addr, int* wait);
	void write_io32w(uint32_t addr, uint32_t data, int* wait);
	uint32_t read_io32w(uint32_t addr, int* wait);
	void write_dma_io8(uint32_t addr, uint32_t data);
	uint32_t read_dma_io8(uint32_t addr);
	void write_dma_io16(uint32_t addr, uint32_t data);
	uint32_t read_dma_io16(uint32_t addr);
	void write_dma_io32(uint32_t addr, uint32_t data);
	uint32_t read_dma_io32(uint32_t addr);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_iomap_single_r(uint32_t addr, DEVICE* device);
	void set_iomap_single_w(uint32_t addr, DEVICE* device);
	void set_iomap_single_rw(uint32_t addr, DEVICE* device);
	void set_iomap_alias_r(uint32_t addr, DEVICE* device, uint32_t alias);
	void set_iomap_alias_w(uint32_t addr, DEVICE* device, uint32_t alias);
	void set_iomap_alias_rw(uint32_t addr, DEVICE* device, uint32_t alias);
	void set_iomap_range_r(uint32_t s, uint32_t e, DEVICE* device);
	void set_iomap_range_w(uint32_t s, uint32_t e, DEVICE* device);
	void set_iomap_range_rw(uint32_t s, uint32_t e, DEVICE* device);
	
	void set_iovalue_single_r(uint32_t addr, uint32_t value);
	void set_iovalue_range_r(uint32_t s, uint32_t e, uint32_t value);
	void set_flipflop_single_rw(uint32_t addr, uint32_t value);
	void set_flipflop_range_rw(uint32_t s, uint32_t e, uint32_t value);
	
	void set_iowait_single_r(uint32_t addr, int wait);
	void set_iowait_single_w(uint32_t addr, int wait);
	void set_iowait_single_rw(uint32_t addr, int wait);
	void set_iowait_range_r(uint32_t s, uint32_t e, int wait);
	void set_iowait_range_w(uint32_t s, uint32_t e, int wait);
	void set_iowait_range_rw(uint32_t s, uint32_t e, int wait);
	
#ifdef _IO_DEBUG_LOG
	int cpu_index;
#endif
	int addr_max;
};

#endif

