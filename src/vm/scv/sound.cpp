/*
	EPOCH Super Cassette Vision Emulator 'eSCV'

	Author : Takeda.Toshiya
	Date   : 2006.08.21 -

	[ uPD1771C ]
*/

#include <math.h>
#include "sound.h"
#include "sound_tbl.h"
#include "../upd7801.h"

//#define SOUND_DEBUG
#define ACK_WAIT 100

void SOUND::reset()
{
	touch_sound();
	
	clear_channel(&tone);
	clear_channel(&noise);
	clear_channel(&square1);
	clear_channel(&square2);
	clear_channel(&square3);
	clear_channel(&pcm);
	
	memset(params, 0, sizeof(params));
	param_cnt = param_ptr = 0;
	register_id = -1;
	cmd_addr = 0;
}

void SOUND::write_data8(uint32_t addr, uint32_t data)
{
	if(register_id != -1) {
		return; // ignore new commands before return ack
	}
	if(!param_cnt) {
		// new command
		touch_sound();
		switch(data) {
		case 0x00: param_cnt = 1;         break; // note off
		case 0x01: param_cnt = 10;        break; // noises & square
		case 0x02: param_cnt = 4;         break; // tone
		case 0x1f: param_cnt = MAX_PARAM; break; // pcm
		}
		param_ptr = 0;
		cmd_addr  = get_cpu_pc(0); // for patch
#ifdef SOUND_DEBUG
		this->out_debug_log(_T("PC=%4x\tSOUND\t"), cmd_addr);
#endif
	}

#ifdef SOUND_DEBUG
	this->out_debug_log(_T("%2x "), data);
#endif
	if(param_cnt) {
		touch_sound();
		params[param_ptr++] = data;
		if(params[0] == 0x1f) {
			// pcm command
			if(param_ptr == 6) {
				memset(pcm_table, 0, sizeof(pcm_table));
				pcm_len = pcm.ptr = 0;
			} else if(param_ptr >= 7) {
				// 0xfe,0x00 : end of pcm, intf1 must not be done except star speeder
				if(params[param_ptr - 2] == 0xfe && data == 0x00 && cmd_addr != 0xa765) {
					param_cnt = 1;
				} else {
					process_pcm(params[param_ptr - 2]);
				}
			}
		}
		if(--param_cnt) {
			if(register_id != -1) {
				cancel_event(this, register_id);
			}
			register_event(this, 0, ACK_WAIT, false, &register_id);
		}
	}
	if(!param_cnt) {
		// process command
		process_cmd();
#ifdef SOUND_DEBUG
		this->out_debug_log(_T("\n"));
#endif
	}
}

void SOUND::write_io8(uint32_t addr, uint32_t data)
{
	// PC3 : L->H
	if(data & 0x08) {
		// note off
		touch_sound();
		clear_channel(&tone);
		clear_channel(&noise);
		clear_channel(&square1);
		clear_channel(&square2);
		clear_channel(&square3);
		
		if(cmd_addr == 0x8402) {
			// y2 monster land
			bool pause = (get_cpu_pc(0) == 0x96c);
			if(pause || !(params[0] == 0x1f && param_ptr > 5)) {
				// terminate command
				if(register_id != -1) {
					cancel_event(this, register_id);
				}
				memset(params, 0, sizeof(params));
				param_cnt = param_ptr = 0;
				
				// terminate pcm when pause
				if(pause) {
					clear_channel(&pcm);
				}
//			} else if(register_id == -1) {
//				vm->register_callback(this, 0, 100, false, &register_id);
			}
		} else {
			if(params[0]) {
				// terminate command
				memset(params, 0, sizeof(params));
				param_cnt = param_ptr = 0;
//				clear_channel(&pcm);
			}
//			if(register_id == -1) {
//				vm->register_callback(this, 0, 100, false, &register_id);
//			}
		}
#ifdef SOUND_DEBUG
		this->out_debug_log(_T("PC3\n"));
#endif
	}
}

void SOUND::event_callback(int event_id, int err)
{
	if(pcm.count && param_ptr == 5 && params[0] == 0x1f && params[1] == 0x04 && params[2] == 0x64) {
		// wait previous pcm
		register_event(this, 0, ACK_WAIT, false, &register_id);
		return;
	}
	d_cpu->write_signal(SIG_UPD7801_INTF1, 1, 1);
	register_id = -1;
}

void SOUND::initialize_sound(int rate)
{
	tone.diff    = (int)((SOUND_CLOCK  / rate) * 128.0 * 16.0 + 0.5);
	noise.diff   = (int)((NOISE_CLOCK  / rate) * 128.0 * 16.0 + 0.5);
	square1.diff = (int)((SQUARE_CLOCK / rate) * 128.0 * 16.0 + 0.5);
	square2.diff = (int)((SQUARE_CLOCK / rate) * 128.0 * 16.0 + 0.5);
	square3.diff = (int)((SQUARE_CLOCK / rate) * 128.0 * 16.0 + 0.5);
	pcm.diff     = (int)((SOUND_CLOCK  / rate) * 128.0 * 16.0 + 0.5);
	
	// create volume table
	double vol = MAX_TONE;
	for(int i = 0; i < 32; i++) {
		volume_table[31 - i] = (int) vol;
		vol /= 1.12201845439369;//1.258925412;
	}
	volume_table[0] = 0;
	
	// create detune table
	for(int i = 0; i < 32; i++) {
		detune_table[i] = (int) (detune_rate[i] * 256 / 100 + 0.5);
	}
	
	// reset device
//	reset();
}

void SOUND::process_cmd()
{
	if(params[0] == 0x00) {
		// note off
		touch_sound();
		clear_channel(&tone);
		clear_channel(&noise);
		clear_channel(&square1);
		clear_channel(&square2);
		clear_channel(&square3);
	} else if(params[0] == 0x01) {
		// noise & square
		touch_sound();
		
		noise.timbre = params[1] >> 5;
		noise.period = params[2] << 8;
		noise.volume = (MAX_NOISE * (params[3] > 0x1f ? 0x1f : params[3])) / 0x1f;
		noise.output = (noise_table[noise.ptr] * noise.volume) >> 8;
		
		square1.period = params[4] << 8;
		square1.volume = (MAX_SQUARE * (params[7] > 0x7f ? 0x7f : params[7])) / 0x7f;
		square1.output = (square_table[square1.ptr] * square1.volume) >> 8;
		
		square2.period = params[5] << 8;
		square2.volume = (MAX_SQUARE * (params[8] > 0x7f ? 0x7f : params[8])) / 0x7f;
		square2.output = (square_table[square2.ptr] * square2.volume) >> 8;
		
		square3.period = params[6] << 8;
		square3.volume = (MAX_SQUARE * (params[9] > 0x7f ? 0x7f : params[9])) / 0x7f;
		square3.output = (square_table[square3.ptr] * square3.volume) >> 8;
		
		// tone off
		clear_channel(&tone);
	} else if(params[0] == 0x02) { // note on : $02, timbre, period, volume ?
		touch_sound();
		
		tone.timbre = params[1] >> 5;
		tone.period = (params[2] * detune_table[params[1] & 0x1f]);
		tone.volume = volume_table[params[3] & 0x1f];
		tone.output = (timbre_table[tone.timbre][tone.ptr] * tone.volume) >> 8;
		
		// noise & square off
		clear_channel(&noise);
		clear_channel(&square1);
		clear_channel(&square2);
		clear_channel(&square3);
	}
	
	// clear command buffer
	memset(params, 0, sizeof(params));
	param_cnt = param_ptr = 0;
}

void SOUND::process_pcm(uint8_t data)
{
	// add pcm wave to buffer
	pcm_table[pcm_len++] = (data & 0x80) ? MAX_PCM : 0;
	pcm_table[pcm_len++] = (data & 0x40) ? MAX_PCM : 0;
	pcm_table[pcm_len++] = (data & 0x20) ? MAX_PCM : 0;
	pcm_table[pcm_len++] = (data & 0x10) ? MAX_PCM : 0;
	pcm_table[pcm_len++] = (data & 0x08) ? MAX_PCM : 0;
	pcm_table[pcm_len++] = (data & 0x04) ? MAX_PCM : 0;
	pcm_table[pcm_len++] = (data & 0x02) ? MAX_PCM : 0;
	pcm_table[pcm_len++] = (data & 0x01) ? MAX_PCM : 0;
	
	if(!pcm.count) {
		pcm.count  = PCM_PERIOD;
		pcm.output = pcm_table[pcm_len - 8];
	}
}

void SOUND::clear_channel(channel_t *ch)
{
	ch->count  = 0;
	ch->volume = 0;
	ch->ptr    = 0;
	ch->output = 0;
}

void SOUND::mix(int32_t* buffer, int cnt)
{
	// create sound buffer
	for(int i = 0; i < cnt; i++) {
		int vol = 0, vol_l, vol_r;
		// mix pcm
		if(pcm.count) {
			pcm.count -= pcm.diff;
			while(pcm.count <= 0) {
				pcm.count += PCM_PERIOD;
				// low-pass filter for the next sample
				if(++pcm.ptr < pcm_len) {
					pcm.output =  (pcm_table[pcm.ptr] + pcm_table[pcm.ptr + 1] + pcm_table[pcm.ptr + 2] + pcm_table[pcm.ptr + 3]) >> 2;
				} else {
					pcm.count = 0;
					break;
				}
			}
			vol = pcm.output;
			vol_l = apply_volume(vol, pcm_volume_l);
			vol_r = apply_volume(vol, pcm_volume_r);
		} else {
			// mix tone
			if(tone.volume && tone.period) {
				tone.count -= tone.diff;
				while(tone.count <= 0) {
					tone.count  += tone.period;
					tone.ptr     = (tone.ptr + 1) & 0xff;
					tone.output  = (timbre_table[tone.timbre][tone.ptr] * tone.volume) >> 8;
				}
				vol += tone.output;
			}
			if(noise.volume && noise.period) {
				noise.count -= noise.diff;
				while(noise.count <= 0) {
					noise.count  += noise.period;
					noise.ptr     = (noise.ptr + 1) & 0xff;
//					noise.output  = (noise_table[noise.timbre][noise.ptr] * noise.volume) >> 8;
					noise.output  = (noise_table[noise.ptr] * noise.volume) >> 8;
				}
				vol += noise.output;
			}
			if(square1.volume && square1.period) {
				square1.count -= square1.diff;
				while(square1.count <= 0) {
					square1.count  += square1.period;
					square1.ptr     = (square1.ptr + 1) & 0xff;
					square1.output  = (square_table[square1.ptr] * square1.volume) >> 8;
				}
				vol += square1.output;
			}
			if(square2.volume && square2.period) {
				square2.count -= square2.diff;
				while(square2.count <= 0) {
					square2.count  += square2.period;
					square2.ptr     = (square2.ptr + 1) & 0xff;
					square2.output  = (square_table[square2.ptr] * square2.volume) >> 8;
				}
				vol += square2.output;
			}
			if(square3.volume && square3.period) {
				square3.count -= square3.diff;
				while(square3.count <= 0) {
					square3.count  += square3.period;
					square3.ptr     = (square3.ptr + 1) & 0xff;
					square3.output  = (square_table[square3.ptr] * square3.volume) >> 8;
				}
				vol += square3.output;
			}
			vol_l = apply_volume(vol, psg_volume_l);
			vol_r = apply_volume(vol, psg_volume_r);
		}
		*buffer++ += vol_l; // L
		*buffer++ += vol_r; // R
	}
}

void SOUND::set_volume(int ch, int decibel_l, int decibel_r)
{
	if(ch == 0) {
		psg_volume_l = decibel_to_volume(decibel_l);
		psg_volume_r = decibel_to_volume(decibel_r);
	} else if(ch == 1) {
		pcm_volume_l = decibel_to_volume(decibel_l);
		pcm_volume_r = decibel_to_volume(decibel_r);
	}
}

#define STATE_VERSION	2

void process_state_channel(channel_t* val, FILEIO* state_fio)
{
	state_fio->StateValue(val->count);
//	state_fio->StateValue(val->diff);
	state_fio->StateValue(val->period);
	state_fio->StateValue(val->timbre);
	state_fio->StateValue(val->volume);
	state_fio->StateValue(val->output);
	state_fio->StateValue(val->ptr);
}

bool SOUND::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	process_state_channel(&tone, state_fio);
	process_state_channel(&noise, state_fio);
	process_state_channel(&square1, state_fio);
	process_state_channel(&square2, state_fio);
	process_state_channel(&square3, state_fio);
	process_state_channel(&pcm, state_fio);
	state_fio->StateArray(pcm_table, sizeof(pcm_table), 1);
	state_fio->StateValue(cmd_addr);
	state_fio->StateValue(pcm_len);
	state_fio->StateValue(param_cnt);
	state_fio->StateValue(param_ptr);
	state_fio->StateValue(register_id);
	state_fio->StateArray(params, sizeof(params), 1);
	return true;
}

