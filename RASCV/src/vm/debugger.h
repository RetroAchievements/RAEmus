/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2014.09.02-

	[ debugger ]
*/

#ifndef _DEBUGGER_H_
#define _DEBUGGER_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

#ifdef USE_DEBUGGER

#define MAX_BREAK_POINTS	16
#define MAX_COMMAND_LENGTH	1024
#define MAX_COMMAND_HISTORY	32
#define MAX_CPU_TRACE		1024

typedef struct {
	struct {
		uint32_t addr, mask;
		int status;	// 0 = none, 1 = enabled, other = disabled
		bool check_point;
	} table[MAX_BREAK_POINTS], stored[MAX_BREAK_POINTS];
	bool hit, restart;
	uint32_t hit_addr;
} break_point_t;

class DEBUGGER : public DEVICE
{
private:
	DEVICE *d_parent, *d_mem, *d_io;
	DEBUGGER *d_child;
	
	void check_mem_break_points(break_point_t *bp, uint32_t addr, int length)
	{
		for(int i = 0; i < MAX_BREAK_POINTS; i++) {
			if(bp->table[i].status == 1) {
				if(addr >= bp->table[i].addr && addr < bp->table[i].addr + length) {
					bp->hit = now_suspended = true;
					bp->hit_addr = bp->table[i].addr;
					bp->restart = bp->table[i].check_point;
					break;
				}
			}
		}
		if(!now_suspended && d_child != NULL) {
			if(d_child->is_cpu()) {
				d_child->check_break_points();
			}
			now_suspended = d_child->hit();
		}
	}
	void check_io_break_points(break_point_t *bp, uint32_t addr)
	{
		for(int i = 0; i < MAX_BREAK_POINTS; i++) {
			if(bp->table[i].status == 1) {
				if((addr & bp->table[i].mask) == (bp->table[i].addr & bp->table[i].mask)) {
					bp->hit = now_suspended = true;
					bp->hit_addr = addr;
					bp->restart = bp->table[i].check_point;
					break;
				}
			}
		}
		if(!now_suspended && d_child != NULL) {
			if(d_child->is_cpu()) {
				d_child->check_break_points();
			}
			now_suspended = d_child->hit();
		}
	}
public:
	DEBUGGER(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		memset(&bp, 0, sizeof(bp));
		memset(&rbp, 0, sizeof(rbp));
		memset(&wbp, 0, sizeof(wbp));
		memset(&ibp, 0, sizeof(ibp));
		memset(&obp, 0, sizeof(obp));
		first_symbol = last_symbol = NULL;
		my_tcscpy_s(file_path, _MAX_PATH, _T("debug.bin"));
		now_debugging = now_going = now_suspended = now_waiting = false;
		now_device_debugging = false;
		d_parent = NULL;
		d_child = NULL;
		memset(history, 0, sizeof(history));
		history_ptr = 0;
		memset(cpu_trace, 0xff, sizeof(cpu_trace));
		prev_cpu_trace = 0xffffffff;
		cpu_trace_ptr = 0;
		set_device_name(_T("Debugger"));
	}
	~DEBUGGER() {}
	
	// common functions
	void initialize()
	{
		for(DEVICE* device = vm->first_device; device; device = device->next_device) {
			if(device->get_debugger() == this) {
				d_parent = device;
				break;
			}
		}
		assert(d_parent != NULL);
	}
	void release()
	{
		release_symbols();
	}
	void write_data8(uint32_t addr, uint32_t data)
	{
		d_mem->write_data8(addr, data);
		check_mem_break_points(&wbp, addr, 1);
	}
	uint32_t read_data8(uint32_t addr)
	{
		uint32_t val = d_mem->read_data8(addr);
		check_mem_break_points(&rbp, addr, 1);
		return val;
	}
	void write_data16(uint32_t addr, uint32_t data)
	{
		d_mem->write_data16(addr, data);
		check_mem_break_points(&wbp, addr, 2);
	}
	uint32_t read_data16(uint32_t addr)
	{
		uint32_t val = d_mem->read_data16(addr);
		check_mem_break_points(&rbp, addr, 2);
		return val;
	}
	void write_data32(uint32_t addr, uint32_t data)
	{
		d_mem->write_data32(addr, data);
		check_mem_break_points(&wbp, addr, 4);
	}
	uint32_t read_data32(uint32_t addr)
	{
		uint32_t val = d_mem->read_data32(addr);
		check_mem_break_points(&rbp, addr, 4);
		return val;
	}
	void write_data8w(uint32_t addr, uint32_t data, int* wait)
	{
		d_mem->write_data8w(addr, data, wait);
		check_mem_break_points(&wbp, addr, 1);
	}
	uint32_t read_data8w(uint32_t addr, int* wait)
	{
		uint32_t val = d_mem->read_data8w(addr, wait);
		check_mem_break_points(&rbp, addr, 1);
		return val;
	}
	void write_data16w(uint32_t addr, uint32_t data, int* wait)
	{
		d_mem->write_data16w(addr, data, wait);
		check_mem_break_points(&wbp, addr, 2);
	}
	uint32_t read_data16w(uint32_t addr, int* wait)
	{
		uint32_t val = d_mem->read_data16w(addr, wait);
		check_mem_break_points(&rbp, addr, 2);
		return val;
	}
	void write_data32w(uint32_t addr, uint32_t data, int* wait)
	{
		d_mem->write_data32w(addr, data, wait);
		check_mem_break_points(&wbp, addr, 4);
	}
	uint32_t read_data32w(uint32_t addr, int* wait)
	{
		uint32_t val = d_mem->read_data32w(addr, wait);
		check_mem_break_points(&rbp, addr, 4);
		return val;
	}
	uint32_t fetch_op(uint32_t addr, int *wait)
	{
		uint32_t val = d_mem->fetch_op(addr, wait);
		check_mem_break_points(&rbp, addr, 1);
		return val;
	}
	void write_io8(uint32_t addr, uint32_t data)
	{
		d_io->write_io8(addr, data);
		check_io_break_points(&obp, addr);
	}
	uint32_t read_io8(uint32_t addr)
	{
		uint32_t val = d_io->read_io8(addr);
		check_io_break_points(&ibp, addr);
		return val;
	}
	void write_io16(uint32_t addr, uint32_t data)
	{
		d_io->write_io16(addr, data);
		check_io_break_points(&obp, addr);
	}
	uint32_t read_io16(uint32_t addr)
	{
		uint32_t val = d_io->read_io16(addr);
		check_io_break_points(&ibp, addr);
		return val;
	}
	void write_io32(uint32_t addr, uint32_t data)
	{
		d_io->write_io32(addr, data);
		check_io_break_points(&obp, addr);
	}
	uint32_t read_io32(uint32_t addr)
	{
		uint32_t val = d_io->read_io32(addr);
		check_io_break_points(&ibp, addr);
		return val;
	}
	void write_io8w(uint32_t addr, uint32_t data, int* wait)
	{
		d_io->write_io8w(addr, data, wait);
		check_io_break_points(&obp, addr);
	}
	uint32_t read_io8w(uint32_t addr, int* wait)
	{
		uint32_t val = d_io->read_io8w(addr, wait);
		check_io_break_points(&ibp, addr);
		return val;
	}
	void write_io16w(uint32_t addr, uint32_t data, int* wait)
	{
		d_io->write_io16w(addr, data, wait);
		check_io_break_points(&obp, addr);
	}
	uint32_t read_io16w(uint32_t addr, int* wait)
	{
		uint32_t val = d_io->read_io16w(addr, wait);
		check_io_break_points(&ibp, addr);
		return val;
	}
	void write_io32w(uint32_t addr, uint32_t data, int* wait)
	{
		d_io->write_io32w(addr, data, wait);
		check_io_break_points(&obp, addr);
	}
	uint32_t read_io32w(uint32_t addr, int* wait)
	{
		uint32_t val = d_io->read_io32w(addr, wait);
		check_io_break_points(&ibp, addr);
		return val;
	}
	void write_via_debugger_data8(uint32_t addr, uint32_t data)
	{
		d_mem->write_via_debugger_data8(addr, data);
		check_mem_break_points(&wbp, addr, 1);
	}
	uint32_t read_via_debugger_data8(uint32_t addr)
	{
		uint32_t val = d_mem->read_via_debugger_data8(addr);
		check_mem_break_points(&rbp, addr, 1);
		return val;
	}
	void write_via_debugger_data16(uint32_t addr, uint32_t data)
	{
		d_mem->write_via_debugger_data16(addr, data);
		check_mem_break_points(&wbp, addr, 2);
	}
	uint32_t read_via_debugger_data16(uint32_t addr)
	{
		uint32_t val = d_mem->read_via_debugger_data16(addr);
		check_mem_break_points(&rbp, addr, 2);
		return val;
	}
	void write_via_debugger_data32(uint32_t addr, uint32_t data)
	{
		d_mem->write_via_debugger_data32(addr, data);
		check_mem_break_points(&wbp, addr, 4);
	}
	uint32_t read_via_debugger_data32(uint32_t addr)
	{
		uint32_t val = d_mem->read_via_debugger_data32(addr);
		check_mem_break_points(&rbp, addr, 4);
		return val;
	}
	void write_via_debugger_data8w(uint32_t addr, uint32_t data, int* wait)
	{
		d_mem->write_via_debugger_data8w(addr, data, wait);
		check_mem_break_points(&wbp, addr, 1);
	}
	uint32_t read_via_debugger_data8w(uint32_t addr, int* wait)
	{
		uint32_t val = d_mem->read_via_debugger_data8w(addr, wait);
		check_mem_break_points(&rbp, addr, 1);
		return val;
	}
	void write_via_debugger_data16w(uint32_t addr, uint32_t data, int* wait)
	{
		d_mem->write_via_debugger_data16w(addr, data, wait);
		check_mem_break_points(&wbp, addr, 2);
	}
	uint32_t read_via_debugger_data16w(uint32_t addr, int* wait)
	{
		uint32_t val = d_mem->read_via_debugger_data16w(addr, wait);
		check_mem_break_points(&rbp, addr, 2);
		return val;
	}
	void write_via_debugger_data32w(uint32_t addr, uint32_t data, int* wait)
	{
		d_mem->write_via_debugger_data32w(addr, data, wait);
		check_mem_break_points(&wbp, addr, 4);
	}
	uint32_t read_via_debugger_data32w(uint32_t addr, int* wait)
	{
		uint32_t val = d_mem->read_via_debugger_data32w(addr, wait);
		check_mem_break_points(&rbp, addr, 4);
		return val;
	}
	void write_via_debugger_io8(uint32_t addr, uint32_t data)
	{
		d_io->write_via_debugger_io8(addr, data);
		check_io_break_points(&obp, addr);
	}
	uint32_t read_via_debugger_io8(uint32_t addr)
	{
		uint32_t val = d_io->read_via_debugger_io8(addr);
		check_io_break_points(&ibp, addr);
		return val;
	}
	void write_via_debugger_io16(uint32_t addr, uint32_t data)
	{
		d_io->write_via_debugger_io16(addr, data);
		check_io_break_points(&obp, addr);
	}
	uint32_t read_via_debugger_io16(uint32_t addr)
	{
		uint32_t val = d_io->read_via_debugger_io16(addr);
		check_io_break_points(&ibp, addr);
		return val;
	}
	void write_via_debugger_io32(uint32_t addr, uint32_t data)
	{
		d_io->write_via_debugger_io32(addr, data);
		check_io_break_points(&obp, addr);
	}
	uint32_t read_via_debugger_io32(uint32_t addr)
	{
		uint32_t val = d_io->read_via_debugger_io32(addr);
		check_io_break_points(&ibp, addr);
		return val;
	}
	void write_via_debugger_io8w(uint32_t addr, uint32_t data, int* wait)
	{
		d_io->write_via_debugger_io8w(addr, data, wait);
		check_io_break_points(&obp, addr);
	}
	uint32_t read_via_debugger_io8w(uint32_t addr, int* wait)
	{
		uint32_t val = d_io->read_via_debugger_io8w(addr, wait);
		check_io_break_points(&ibp, addr);
		return val;
	}
	void write_via_debugger_io16w(uint32_t addr, uint32_t data, int* wait)
	{
		d_io->write_via_debugger_io16w(addr, data, wait);
		check_io_break_points(&obp, addr);
	}
	uint32_t read_via_debugger_io16w(uint32_t addr, int* wait)
	{
		uint32_t val = d_io->read_via_debugger_io16w(addr, wait);
		check_io_break_points(&ibp, addr);
		return val;
	}
	void write_via_debugger_io32w(uint32_t addr, uint32_t data, int* wait)
	{
		d_io->write_via_debugger_io32w(addr, data, wait);
		check_io_break_points(&obp, addr);
	}
	uint32_t read_via_debugger_io32w(uint32_t addr, int* wait)
	{
		uint32_t val = d_io->read_via_debugger_io32w(addr, wait);
		check_io_break_points(&ibp, addr);
		return val;
	}
	bool is_debugger()
	{
		return true;
	}
	
	// unique functions
	void set_context_mem(DEVICE* device)
	{
		d_mem = device;
	}
	void set_context_io(DEVICE* device)
	{
		d_io = device;
	}
	void set_context_child(DEBUGGER* device)
	{
		d_child = device;
	}
	void check_break_points(uint32_t addr)
	{
		check_mem_break_points(&bp, addr, 1);
	}
	void check_break_points()
	{
		check_mem_break_points(&bp, d_parent->get_next_pc(), 1);
	}
	void store_break_points()
	{
		if(d_child != NULL) {
			d_child->store_break_points();
		}
		memcpy( bp.stored,  bp.table, sizeof( bp.table));
		memcpy(rbp.stored, rbp.table, sizeof(rbp.table));
		memcpy(wbp.stored, wbp.table, sizeof(wbp.table));
		memcpy(ibp.stored, ibp.table, sizeof(ibp.table));
		memcpy(obp.stored, obp.table, sizeof(obp.table));
		memset( bp.table, 0, sizeof( bp.table));
		memset(rbp.table, 0, sizeof(rbp.table));
		memset(wbp.table, 0, sizeof(wbp.table));
		memset(ibp.table, 0, sizeof(ibp.table));
		memset(obp.table, 0, sizeof(obp.table));
	}
	void restore_break_points()
	{
		if(d_child != NULL) {
			d_child->restore_break_points();
		}
		memcpy( bp.table,  bp.stored, sizeof( bp.table));
		memcpy(rbp.table, rbp.stored, sizeof(rbp.table));
		memcpy(wbp.table, wbp.stored, sizeof(wbp.table));
		memcpy(ibp.table, ibp.stored, sizeof(ibp.table));
		memcpy(obp.table, obp.stored, sizeof(obp.table));
	}
	bool hit()
	{
		if(d_child != NULL && d_child->hit()) {
			return true;
		}
		return (bp.hit || rbp.hit || wbp.hit || ibp.hit || obp.hit);
	}
	bool restartable()
	{
		if(d_child != NULL && !d_child->restartable()) {
			return false;
		}
		if( bp.hit && ! bp.restart) return false;
		if(rbp.hit && !rbp.restart) return false;
		if(wbp.hit && !wbp.restart) return false;
		if(ibp.hit && !ibp.restart) return false;
		if(obp.hit && !obp.restart) return false;
		return true;
	}
	void clear_hit()
	{
		if(d_child != NULL) {
			d_child->clear_hit();
		}
		bp.hit = rbp.hit = wbp.hit = ibp.hit = obp.hit = false;
	}
	void add_symbol(uint32_t addr, const _TCHAR *name)
	{
		symbol_t *symbol = (symbol_t *)calloc(sizeof(symbol_t), 1);
		symbol->addr = addr;
		symbol->name = (_TCHAR *)calloc(sizeof(_TCHAR), _tcslen(name) + 1);
		my_tcscpy_s(symbol->name, _tcslen(name) + 1, name);
		
		if(first_symbol == NULL) {
			first_symbol = symbol;
		} else {
			last_symbol->next_symbol = symbol;
		}
		last_symbol = symbol;
	}
	void release_symbols()
	{
		for(symbol_t* symbol = first_symbol; symbol;) {
			symbol_t *next_symbol = symbol->next_symbol;
			if(symbol->name != NULL) {
				free(symbol->name);
			}
			free(symbol);
			symbol = next_symbol;
		}
		first_symbol = last_symbol = NULL;
	}
	void add_cpu_trace(uint32_t pc)
	{
		if(prev_cpu_trace != pc) {
			cpu_trace[cpu_trace_ptr++] = prev_cpu_trace = pc;
			cpu_trace_ptr &= (MAX_CPU_TRACE - 1);
		}
	}
	break_point_t bp, rbp, wbp, ibp, obp;
	symbol_t *first_symbol, *last_symbol;
	_TCHAR file_path[_MAX_PATH];
	bool now_debugging, now_going, now_suspended, now_waiting;
	bool now_device_debugging; // for non-cpu devices
	_TCHAR history[MAX_COMMAND_HISTORY][MAX_COMMAND_LENGTH + 1];
	int history_ptr;
	uint32_t cpu_trace[MAX_CPU_TRACE], prev_cpu_trace;
	int cpu_trace_ptr;
};

#endif
#endif

