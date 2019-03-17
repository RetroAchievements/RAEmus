/*
	Skelton for retropc emulator

	Origin : MESS UPD7810 Core
	Author : Takeda.Toshiya
	Date   : 2006.08.21 -

	[ uPD7801 ]
*/

#ifndef _UPD7801_H_
#define _UPD7801_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

#define SIG_UPD7801_INTF0	0
#define SIG_UPD7801_INTF1	1
#define SIG_UPD7801_INTF2	2
#define SIG_UPD7801_WAIT	3
#define SIG_UPD7801_SI		4
#define SIG_UPD7801_SCK		5

// virtual i/o port address
#define P_A	0
#define P_B	1
#define P_C	2

#ifdef USE_DEBUGGER
class DEBUGGER;
#endif

class UPD7801 : public DEVICE
{
private:
	/* ---------------------------------------------------------------------------
	contexts
	--------------------------------------------------------------------------- */
	
	outputs_t outputs_so;
	DEVICE *d_mem, *d_io;
#ifdef USE_DEBUGGER
	DEBUGGER *d_debugger;
	DEVICE *d_mem_stored, *d_io_stored;
#endif
	
	/* ---------------------------------------------------------------------------
	registers
	--------------------------------------------------------------------------- */
	
#ifdef USE_DEBUGGER
	uint64_t total_count;
	uint64_t prev_total_count;
#endif
	int count, period, scount, tcount;
	bool wait;
	
	pair32_t regs[8];
	uint16_t SP, PC, prevPC;
	uint8_t PSW, IRR, IFF, SIRQ, HALT, MK, MB, MC, TM0, TM1, SR;
	// for port c
	uint8_t SAK, TO, HLDA, PORTC;
	// for serial i/o
	bool SI, SCK;
	int sio_count;
	
	/* ---------------------------------------------------------------------------
	virtual machine interface
	--------------------------------------------------------------------------- */
	
	// memory
	inline uint8_t RM8(uint16_t addr);
	inline void WM8(uint16_t addr, uint8_t val);
	inline uint16_t RM16(uint16_t addr);
	inline void WM16(uint16_t addr, uint16_t val);
	inline uint8_t FETCH8();
	inline uint16_t FETCH16();
	inline uint16_t FETCHWA();
	inline uint8_t POP8();
	inline void PUSH8(uint8_t val);
	inline uint16_t POP16();
	inline void PUSH16(uint16_t val);
	
	// i/o
	inline uint8_t IN8(int port);
	inline void OUT8(int port, uint8_t val);
	inline void UPDATE_PORTC(uint8_t IOM);
	
	/* ---------------------------------------------------------------------------
	opecode
	--------------------------------------------------------------------------- */
	
	void run_one_opecode();
#ifdef USE_DEBUGGER
	void run_one_opecode_debugger();
#endif
	void OP();
	void OP48();
	void OP4C();
	void OP4D();
	void OP60();
	void OP64();
	void OP70();
	void OP74();
	
public:
	UPD7801(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
#ifdef USE_DEBUGGER
		total_count = prev_total_count = 0;
#endif
		initialize_output_signals(&outputs_so);
		SI = SCK = false;
		set_device_name(_T("uPD7801 CPU"));
	}
	~UPD7801() {}
	
	// common functions
	void initialize();
	void reset();
	int run(int clock);
	void write_signal(int id, uint32_t data, uint32_t mask);
	uint32_t get_pc()
	{
		return prevPC;
	}
	uint32_t get_next_pc()
	{
		return PC;
	}
#ifdef USE_DEBUGGER
	bool is_cpu()
	{
		return true;
	}
	bool is_debugger_available()
	{
		return true;
	}
	void *get_debugger()
	{
		return d_debugger;
	}
	uint32_t get_debug_prog_addr_mask()
	{
		return 0xffff;
	}
	uint32_t get_debug_data_addr_mask()
	{
		return 0xffff;
	}
	void write_debug_data8(uint32_t addr, uint32_t data);
	uint32_t read_debug_data8(uint32_t addr);
	void write_debug_io8(uint32_t addr, uint32_t data);
	uint32_t read_debug_io8(uint32_t addr);
	bool write_debug_reg(const _TCHAR *reg, uint32_t data);
	bool get_debug_regs_info(_TCHAR *buffer, size_t buffer_len);
	int debug_dasm(uint32_t pc, _TCHAR *buffer, size_t buffer_len);
#endif
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_mem(DEVICE* device)
	{
		d_mem = device;
	}
	void set_context_io(DEVICE* device)
	{
		d_io = device;
	}
#ifdef USE_DEBUGGER
	void set_context_debugger(DEBUGGER* device)
	{
		d_debugger = device;
	}
#endif
	void set_context_so(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_so, device, id, mask);
	}
};

#endif
