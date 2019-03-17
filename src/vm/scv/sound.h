/*
	EPOCH Super Cassette Vision Emulator 'eSCV'

	Author : Takeda.Toshiya
	Date   : 2006.08.21 -

	[ uPD1771C ]
*/

#ifndef _SOUND_H_
#define _SOUND_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SOUND_CLOCK  1522400.0
#define NOISE_CLOCK      760.0
#define SQUARE_CLOCK  174000.0

#define PCM_PERIOD 120000

#define MAX_TONE   24000
#define MAX_NOISE  16000
#define MAX_SQUARE  8000
#define MAX_PCM    20000

#define MAX_PARAM 0x8000

typedef struct {
	int count;
	int diff;
	int period;
	int timbre;
	int volume;
	int output;
	int ptr;
} channel_t;

class SOUND : public DEVICE
{
private:
	DEVICE* d_cpu;
	
	// sound gen
	channel_t tone;
	channel_t noise;
	channel_t square1;
	channel_t square2;
	channel_t square3;
	channel_t pcm;
	void clear_channel(channel_t *ch);
	
	int pcm_table[MAX_PARAM * 8];
	uint32_t cmd_addr;
	int pcm_len;
	
	int volume_table[32];
	int detune_table[32];
	
	int psg_volume_l, psg_volume_r;
	int pcm_volume_l, pcm_volume_r;
	
	// command buffer
	int param_cnt, param_ptr, register_id;
	uint8_t params[MAX_PARAM];
	
	void process_pcm(uint8_t data);
	void process_cmd();
	
public:
	SOUND(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Sound"));
	}
	~SOUND() {}
	
	// common functions
	void reset();
	void write_data8(uint32_t addr, uint32_t data);
	void write_io8(uint32_t addr, uint32_t data);
	void event_callback(int event_id, int err);
	void mix(int32_t* buffer, int cnt);
	void set_volume(int ch, int decibel_l, int decibel_r);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_cpu(DEVICE* device)
	{
		d_cpu = device;
	}
	void initialize_sound(int rate);
};

#endif

