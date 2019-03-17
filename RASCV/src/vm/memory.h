/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2010.09.16-

	[ memory ]
*/

#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

#ifndef MEMORY_ADDR_MAX
#define MEMORY_ADDR_MAX 0x10000
#endif
#ifndef MEMORY_BANK_SIZE
#define MEMORY_BANK_SIZE 0x1000
#endif

class MEMORY : public DEVICE
{
private:
	typedef struct {
		DEVICE* dev;
		uint8_t* memory;
		int wait;
	} bank_t;
	
	bank_t *rd_table;
	bank_t *wr_table;
	
	int addr_shift;
	
	uint8_t *rd_dummy;
	uint8_t *wr_dummy;
	
public:
	MEMORY(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		addr_max = MEMORY_ADDR_MAX;
		bank_size = MEMORY_BANK_SIZE;
		
		rd_table = wr_table = NULL;
		rd_dummy = wr_dummy = NULL;
		
		set_device_name(_T("Generic Memory Bus"));
	}
	~MEMORY() {}
	
	// common functions
	void initialize();
	void release();
	uint32_t read_data8(uint32_t addr);
	void write_data8(uint32_t addr, uint32_t data);
	uint32_t read_data16(uint32_t addr);
	void write_data16(uint32_t addr, uint32_t data);
	uint32_t read_data32(uint32_t addr);
	void write_data32(uint32_t addr, uint32_t data);
	uint32_t read_data8w(uint32_t addr, int* wait);
	void write_data8w(uint32_t addr, uint32_t data, int* wait);
	uint32_t read_data16w(uint32_t addr, int* wait);
	void write_data16w(uint32_t addr, uint32_t data, int* wait);
	uint32_t read_data32w(uint32_t addr, int* wait);
	void write_data32w(uint32_t addr, uint32_t data, int* wait);
#ifdef MEMORY_DISABLE_DMA_MMIO
	uint32_t read_dma_data8(uint32_t addr);
	void write_dma_data8(uint32_t addr, uint32_t data);
	uint32_t read_dma_data16(uint32_t addr);
	void write_dma_data16(uint32_t addr, uint32_t data);
	uint32_t read_dma_data32(uint32_t addr);
	void write_dma_data32(uint32_t addr, uint32_t data);
#endif
	
	// unique functions
	void set_memory_r(uint32_t start, uint32_t end, uint8_t *memory);
	void set_memory_w(uint32_t start, uint32_t end, uint8_t *memory);
	void set_memory_rw(uint32_t start, uint32_t end, uint8_t *memory)
	{
		set_memory_r(start, end, memory);
		set_memory_w(start, end, memory);
	}
	void set_memory_mapped_io_r(uint32_t start, uint32_t end, DEVICE *device);
	void set_memory_mapped_io_w(uint32_t start, uint32_t end, DEVICE *device);
	void set_memory_mapped_io_rw(uint32_t start, uint32_t end, DEVICE *device)
	{
		set_memory_mapped_io_r(start, end, device);
		set_memory_mapped_io_w(start, end, device);
	}
	void set_wait_r(uint32_t start, uint32_t end, int wait);
	void set_wait_w(uint32_t start, uint32_t end, int wait);
	void set_wait_rw(uint32_t start, uint32_t end, int wait)
	{
		set_wait_r(start, end, wait);
		set_wait_w(start, end, wait);
	}
	void unset_memory_r(uint32_t start, uint32_t end);
	void unset_memory_w(uint32_t start, uint32_t end);
	void unset_memory_rw(uint32_t start, uint32_t end)
	{
		unset_memory_r(start, end);
		unset_memory_w(start, end);
	}
	int read_bios(const _TCHAR *file_name, uint8_t *buffer, int size);
	bool write_bios(const _TCHAR *file_name, uint8_t *buffer, int size);
	bool read_image(const _TCHAR *file_path, uint8_t *buffer, int size);
	bool write_image(const _TCHAR *file_path, uint8_t *buffer, int size);
	
	int addr_max;
	int bank_size;
};

#endif

