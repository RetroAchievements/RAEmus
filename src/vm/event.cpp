/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.11.29-

	[ event manager ]
*/

#include "event.h"

#define EVENT_MIX	0

void EVENT::initialize()
{
	// load config
	if(!(0 <= config.cpu_power && config.cpu_power <= 4)) {
		config.cpu_power = 0;
	}
	power = config.cpu_power;
	
	// initialize sound buffer
	sound_buffer = NULL;
	sound_tmp = NULL;
	
	dont_skip_frames = 0;
	prev_skip = next_skip = false;
	sound_changed = false;
	
	vline_start_clock = 0;
	cur_vline = 0;
	vclocks[0] = (int)((double)d_cpu[0].cpu_clocks / (double)FRAMES_PER_SEC / (double)LINES_PER_FRAME + 0.5); // temporary
}

void EVENT::initialize_sound(int rate, int samples)
{
	// initialize sound
	sound_samples = samples;
	sound_tmp_samples = samples * 2;
	sound_buffer = (uint16_t*)malloc(sound_samples * sizeof(uint16_t) * 2);
	memset(sound_buffer, 0, sound_samples * sizeof(uint16_t) * 2);
	sound_tmp = (int32_t*)malloc(sound_tmp_samples * sizeof(int32_t) * 2);
	memset(sound_tmp, 0, sound_tmp_samples * sizeof(int32_t) * 2);
	buffer_ptr = 0;
	mix_counter = 1;
	mix_limit = (int)((double)(emu->get_sound_rate() / 2000.0)); // per 0.5ms.
	
	// register event
	this->register_event(this, EVENT_MIX, 1000000.0 / rate, true, NULL);
}

void EVENT::release()
{
	// release sound
	if(sound_buffer) {
		free(sound_buffer);
	}
	if(sound_tmp) {
		free(sound_tmp);
	}
}

void EVENT::reset()
{
	// clear events except loop event
	for(int i = 0; i < MAX_EVENT; i++) {
		if(event[i].active && event[i].loop_clock == 0) {
			cancel_event(NULL, i);
		}
	}
	
	event_remain = 0;
	cpu_remain = cpu_accum = cpu_done = 0;
	
	// reset sound
	if(sound_buffer) {
		memset(sound_buffer, 0, sound_samples * sizeof(uint16_t) * 2);
	}
	if(sound_tmp) {
		memset(sound_tmp, 0, sound_tmp_samples * sizeof(int32_t) * 2);
	}
//	buffer_ptr = 0;
	
#ifdef _DEBUG_LOG
	initialize_done = true;
#endif
}

void EVENT::drive()
{
	// raise pre frame events to update timing settings
	for(int i = 0; i < frame_event_count; i++) {
		frame_event[i]->event_pre_frame();
	}
	
	// generate clocks per line
	if(frames_per_sec != next_frames_per_sec || lines_per_frame != next_lines_per_frame) {
		frames_per_sec = next_frames_per_sec;
		lines_per_frame = next_lines_per_frame;
		
		int sum = (int)((double)d_cpu[0].cpu_clocks / frames_per_sec + 0.5);
		int remain = sum;
		
		for(int i = 0; i < lines_per_frame; i++) {
			assert(i < MAX_LINES);
			vclocks[i] = (int)(sum / lines_per_frame);
			remain -= vclocks[i];
		}
		for(int i = 0; i < remain; i++) {
			int index = (int)((double)lines_per_frame * (double)i / (double)remain);
			assert(index < MAX_LINES);
			vclocks[index]++;
		}
		for(int i = 1; i < dcount_cpu; i++) {
			d_cpu[i].update_clocks = (int)(1024.0 * (double)d_cpu[i].cpu_clocks / (double)d_cpu[0].cpu_clocks + 0.5);
		}
		for(DEVICE* device = vm->first_device; device; device = device->next_device) {
			if(device->get_event_manager_id() == this_device_id) {
				device->update_timing(d_cpu[0].cpu_clocks, frames_per_sec, lines_per_frame);
			}
		}
	}
	
	// run virtual machine for 1 frame period
	for(int i = 0; i < frame_event_count; i++) {
		frame_event[i]->event_frame();
	}
	for(cur_vline = 0; cur_vline < lines_per_frame; cur_vline++) {
		vline_start_clock = get_current_clock();
		
		// run virtual machine per line
		for(int i = 0; i < vline_event_count; i++) {
			vline_event[i]->event_vline(cur_vline, vclocks[cur_vline]);
		}
		
		if(event_remain < 0) {
			if(-event_remain > vclocks[cur_vline]) {
				update_event(vclocks[cur_vline]);
			} else {
				update_event(-event_remain);
			}
		}
		event_remain += vclocks[cur_vline];
		cpu_remain += vclocks[cur_vline] << power;
		
		while(event_remain > 0) {
			int event_done = event_remain;
			if(cpu_remain > 0) {
				int cpu_done_tmp;
				if(dcount_cpu == 1) {
					// run one opecode on primary cpu
					cpu_done_tmp = d_cpu[0].device->run(-1);
				} else {
					// sync to sub cpus
					if(cpu_done == 0) {
						// run one opecode on primary cpu
						cpu_done = d_cpu[0].device->run(-1);
					}
					
					// sub cpu runs continuously and no events will be fired while the given clocks,
					// so I need to give small enough clocks...
					cpu_done_tmp = (cpu_done < 4) ? cpu_done : 4;
					cpu_done -= cpu_done_tmp;
					
					for(int i = 1; i < dcount_cpu; i++) {
						// run sub cpus
						d_cpu[i].accum_clocks += d_cpu[i].update_clocks * cpu_done_tmp;
						int sub_clock = d_cpu[i].accum_clocks >> 10;
						if(sub_clock) {
							d_cpu[i].accum_clocks -= sub_clock << 10;
							d_cpu[i].device->run(sub_clock);
						}
					}
				}
				cpu_remain -= cpu_done_tmp;
				cpu_accum += cpu_done_tmp;
				event_done = cpu_accum >> power;
				cpu_accum -= event_done << power;
			}
			if(event_done > 0) {
				if(event_done > event_remain) {
					update_event(event_remain);
				} else {
					update_event(event_done);
				}
				event_remain -= event_done;
			}
		}
	}
}

void EVENT::update_event(int clock)
{
	uint64_t event_clocks_tmp = event_clocks + clock;
	
	while(first_fire_event != NULL && first_fire_event->expired_clock <= event_clocks_tmp) {
		event_t *event_handle = first_fire_event;
		uint64_t expired_clock = event_handle->expired_clock;
		
		first_fire_event = event_handle->next;
		if(first_fire_event != NULL) {
			first_fire_event->prev = NULL;
		}
		if(event_handle->loop_clock != 0) {
			event_handle->accum_clocks += event_handle->loop_clock;
			uint64_t clock_tmp = event_handle->accum_clocks >> 10;
			event_handle->accum_clocks -= clock_tmp << 10;
			event_handle->expired_clock += clock_tmp;
			insert_event(event_handle);
		} else {
			event_handle->active = false;
			event_handle->next = first_free_event;
			first_free_event = event_handle;
		}
		event_clocks = expired_clock;
		event_handle->device->event_callback(event_handle->event_id, 0);
	}
	event_clocks = event_clocks_tmp;
}

uint32_t EVENT::get_current_clock()
{
	return (uint32_t)(event_clocks & 0xffffffff);
}

uint32_t EVENT::get_passed_clock(uint32_t prev)
{
	uint32_t current = get_current_clock();
	return (current > prev) ? current - prev : current + (0xffffffff - prev) + 1;
}

double EVENT::get_passed_usec(uint32_t prev)
{
	return 1000000.0 * get_passed_clock(prev) / d_cpu[0].cpu_clocks;
}

uint32_t EVENT::get_passed_clock_since_vline()
{
	return get_passed_clock(vline_start_clock);
}

double EVENT::get_passed_usec_since_vline()
{
	return get_passed_usec(vline_start_clock);
}

uint32_t EVENT::get_cpu_pc(int index)
{
	return d_cpu[index].device->get_pc();
}

void EVENT::register_event(DEVICE* device, int event_id, double usec, bool loop, int* register_id)
{
#ifdef _DEBUG_LOG
	if(!initialize_done && !loop) {
		this->out_debug_log(_T("EVENT: non-loop event is registered before initialize is done\n"));
	}
#endif
	
	// register event
	if(first_free_event == NULL) {
#ifdef _DEBUG_LOG
		this->out_debug_log(_T("EVENT: too many events !!!\n"));
#endif
		if(register_id != NULL) {
			*register_id = -1;
		}
		return;
	}
	event_t *event_handle = first_free_event;
	first_free_event = first_free_event->next;
	
	if(register_id != NULL) {
		*register_id = event_handle->index;
	}
	event_handle->active = true;
	event_handle->device = device;
	event_handle->event_id = event_id;
	uint64_t clock;
	if(loop) {
		event_handle->loop_clock = (uint64_t)(1024.0 * (double)d_cpu[0].cpu_clocks / 1000000.0 * usec + 0.5);
		event_handle->accum_clocks = event_handle->loop_clock;
		clock = event_handle->accum_clocks >> 10;
		event_handle->accum_clocks -= clock << 10;
	} else {
		clock = (uint64_t)((double)d_cpu[0].cpu_clocks / 1000000.0 * usec + 0.5);
		event_handle->loop_clock = 0;
		event_handle->accum_clocks = 0;
	}
	event_handle->expired_clock = event_clocks + clock;
	
	insert_event(event_handle);
}

void EVENT::register_event_by_clock(DEVICE* device, int event_id, uint64_t clock, bool loop, int* register_id)
{
#ifdef _DEBUG_LOG
	if(!initialize_done && !loop) {
		this->out_debug_log(_T("EVENT: device (name=%s, id=%d) registeres non-loop event before initialize is done\n"), device->this_device_name, device->this_device_id);
	}
#endif
	
	// register event
	if(first_free_event == NULL) {
#ifdef _DEBUG_LOG
		this->out_debug_log(_T("EVENT: too many events !!!\n"));
#endif
		if(register_id != NULL) {
			*register_id = -1;
		}
		return;
	}
	event_t *event_handle = first_free_event;
	first_free_event = first_free_event->next;
	
	if(register_id != NULL) {
		*register_id = event_handle->index;
	}
	event_handle->active = true;
	event_handle->device = device;
	event_handle->event_id = event_id;
	event_handle->expired_clock = event_clocks + clock;
	event_handle->loop_clock = loop ? (clock << 10) : 0;
	event_handle->accum_clocks = 0;
	
	insert_event(event_handle);
}

void EVENT::insert_event(event_t *event_handle)
{
	if(first_fire_event == NULL) {
		first_fire_event = event_handle;
		event_handle->prev = event_handle->next = NULL;
	} else {
		for(event_t *insert_pos = first_fire_event; insert_pos != NULL; insert_pos = insert_pos->next) {
			if(insert_pos->expired_clock > event_handle->expired_clock) {
				if(insert_pos->prev != NULL) {
					// insert
					insert_pos->prev->next = event_handle;
					event_handle->prev = insert_pos->prev;
					event_handle->next = insert_pos;
					insert_pos->prev = event_handle;
					break;
				} else {
					// add to head
					first_fire_event = event_handle;
					event_handle->prev = NULL;
					event_handle->next = insert_pos;
					insert_pos->prev = event_handle;
					break;
				}
			} else if(insert_pos->next == NULL) {
				// add to tail
				insert_pos->next = event_handle;
				event_handle->prev = insert_pos;
				event_handle->next = NULL;
				break;
			}
		}
	}
}

void EVENT::cancel_event(DEVICE* device, int register_id)
{
	// cancel registered event
	if(0 <= register_id && register_id < MAX_EVENT) {
		event_t *event_handle = &event[register_id];
		if(device != NULL && device != event_handle->device) {
			this->out_debug_log(_T("EVENT: device (name=%s, id=%d) tries to calcel event that is not its own !!!\n"), device->this_device_name, device->this_device_id);
			return;
		}
		if(event_handle->active) {
			if(event_handle->prev != NULL) {
				event_handle->prev->next = event_handle->next;
			} else {
				first_fire_event = event_handle->next;
			}
			if(event_handle->next != NULL) {
				event_handle->next->prev = event_handle->prev;
			}
			event_handle->active = false;
			event_handle->next = first_free_event;
			first_free_event = event_handle;
		}
	}
}

void EVENT::register_frame_event(DEVICE* device)
{
	if(frame_event_count < MAX_EVENT) {
		for(int i = 0; i < frame_event_count; i++) {
			if(frame_event[i] == device) {
#ifdef _DEBUG_LOG
				this->out_debug_log(_T("EVENT: device (name=%s, id=%d) has already registered frame event !!!\n"), device->this_device_name, device->this_device_id);
#endif
				return;
			}
		}
		frame_event[frame_event_count++] = device;
#ifdef _DEBUG_LOG
	} else {
		this->out_debug_log(_T("EVENT: too many frame events !!!\n"));
#endif
	}
}

void EVENT::register_vline_event(DEVICE* device)
{
	if(vline_event_count < MAX_EVENT) {
		for(int i = 0; i < vline_event_count; i++) {
			if(vline_event[i] == device) {
#ifdef _DEBUG_LOG
				this->out_debug_log(_T("EVENT: device (name=%s, id=%d) has already registered vline event !!!\n"), device->this_device_name, device->this_device_id);
#endif
				return;
			}
		}
		vline_event[vline_event_count++] = device;
#ifdef _DEBUG_LOG
	} else {
		this->out_debug_log(_T("EVENT: too many vline events !!!\n"));
#endif
	}
}

uint32_t EVENT::get_event_remaining_clock(int register_id)
{
	if(0 <= register_id && register_id < MAX_EVENT) {
		event_t *event_handle = &event[register_id];
		if(event_handle->active && event->expired_clock > event_clocks) {
			return (uint32_t)(event->expired_clock - event_clocks);
		}
	}
	return 0;
}

double EVENT::get_event_remaining_usec(int register_id)
{
	return 1000000.0 * get_event_remaining_clock(register_id) / d_cpu[0].cpu_clocks;
}

void EVENT::touch_sound()
{
	if(!(config.sound_strict_rendering || (need_mix > 0))) {
		int samples = mix_counter;
		if(samples >= (sound_tmp_samples - buffer_ptr)) {
			samples = sound_tmp_samples - buffer_ptr;
		}
		if(samples > 0) {
			mix_sound(samples);
			mix_counter -= samples;
		}
	}
}

void EVENT::set_realtime_render(DEVICE* device, bool flag)
{
	assert(device != NULL && device->this_device_id < MAX_DEVICE);
	if(dev_need_mix[device->this_device_id] != flag) {
		if(flag) {
			need_mix++;
		} else {
			assert(need_mix > 0);
			need_mix--;
			if(need_mix < 0) need_mix = 0;
		}
		dev_need_mix[device->this_device_id] = flag;
	}
}

void EVENT::event_callback(int event_id, int err)
{
	// mix sound
	if(prev_skip && dont_skip_frames == 0 && !sound_changed) {
		buffer_ptr = 0;
	}
	int remain = sound_tmp_samples - buffer_ptr;
	
	if(remain > 0) {
		int samples = mix_counter;
		
		if(config.sound_strict_rendering || (need_mix > 0)) {
			if(samples < 1) {
				samples = 1;
			}
		}
		if(samples >= remain) {
			samples = remain;
		}
		if(config.sound_strict_rendering || (need_mix > 0)) {
			if(samples > 0) {
				mix_sound(samples);
			}
			mix_counter = 1;
		} else {
			if(samples > 0 && mix_counter >= mix_limit) {
				mix_sound(samples);
				mix_counter -= samples;
			}
			mix_counter++;
		}
	}
}

void EVENT::mix_sound(int samples)
{
	if(samples > 0) {
		int32_t* buffer = sound_tmp + buffer_ptr * 2;
		memset(buffer, 0, samples * sizeof(int32_t) * 2);
		for(int i = 0; i < dcount_sound; i++) {
			d_sound[i]->mix(buffer, samples);
		}
		if(!sound_changed) {
			for(int i = 0; i < samples * 2; i += 2) {
				if(buffer[i] != sound_tmp[0] || buffer[i + 1] != sound_tmp[1]) {
					sound_changed = true;
					break;
				}
			}
		}
		buffer_ptr += samples;
	} else {
		// notify to sound devices
		for(int i = 0; i < dcount_sound; i++) {
			d_sound[i]->mix(sound_tmp + buffer_ptr * 2, 0);
		}
	}
}

uint16_t* EVENT::create_sound(int* extra_frames)
{
	if(prev_skip && dont_skip_frames == 0 && !sound_changed) {
		memset(sound_buffer, 0, sound_samples * sizeof(uint16_t) * 2);
		*extra_frames = 0;
		return sound_buffer;
	}
	int frames = 0;
	
	// drive extra frames to fill the sound buffer
	while(sound_samples > buffer_ptr) {
		drive();
		frames++;
	}
#ifdef LOW_PASS_FILTER
	// low-pass filter
	for(int i = 0; i < sound_samples - 1; i++) {
		sound_tmp[i * 2    ] = (sound_tmp[i * 2    ] + sound_tmp[i * 2 + 2]) / 2; // L
		sound_tmp[i * 2 + 1] = (sound_tmp[i * 2 + 1] + sound_tmp[i * 2 + 3]) / 2; // R
	}
#endif
	// copy to buffer
	for(int i = 0; i < sound_samples * 2; i++) {
		int dat = sound_tmp[i];
		uint16_t highlow = (uint16_t)(dat & 0x0000ffff);
		
		if((dat > 0) && (highlow >= 0x8000)) {
			sound_buffer[i] = 0x7fff;
			continue;
		}
		if((dat < 0) && (highlow < 0x8000)) {
			sound_buffer[i] = 0x8000;
			continue;
		}
		sound_buffer[i] = highlow;
	}
	if(buffer_ptr > sound_samples) {
		buffer_ptr -= sound_samples;
		memcpy(sound_tmp, sound_tmp + sound_samples * 2, buffer_ptr * sizeof(int32_t) * 2);
	} else {
		buffer_ptr = 0;
	}
	*extra_frames = frames;
	return sound_buffer;
}

int EVENT::get_sound_buffer_ptr()
{
	return buffer_ptr;
}

void EVENT::request_skip_frames()
{
	next_skip = true;
}

bool EVENT::is_frame_skippable()
{
	bool value = next_skip;
	
	if(sound_changed || (prev_skip && !next_skip)) {
		dont_skip_frames = (int)frames_per_sec;
	}
	if(dont_skip_frames > 0) {
		value = false;
		dont_skip_frames--;
	}
	prev_skip = next_skip;
	next_skip = false;
	sound_changed = false;
	
	return value;
}

void EVENT::update_config()
{
	if(power != config.cpu_power) {
		power = config.cpu_power;
		cpu_accum = 0;
	}
}

#define STATE_VERSION	3

bool EVENT::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(dcount_cpu)) {
		return false;
	}
	for(int i = 0; i < dcount_cpu; i++) {
		state_fio->StateValue(d_cpu[i].cpu_clocks);
		state_fio->StateValue(d_cpu[i].update_clocks);
		state_fio->StateValue(d_cpu[i].accum_clocks);
	}
	state_fio->StateArray(vclocks, sizeof(vclocks), 1);
	state_fio->StateValue(event_remain);
	state_fio->StateValue(cpu_remain);
	state_fio->StateValue(cpu_accum);
	state_fio->StateValue(cpu_done);
	state_fio->StateValue(event_clocks);
	for(int i = 0; i < MAX_EVENT; i++) {
		if(loading) {
			event[i].device = vm->get_device(state_fio->FgetInt32_LE());
		} else {
			state_fio->FputInt32_LE(event[i].device != NULL ? event[i].device->this_device_id : -1);
		}
		state_fio->StateValue(event[i].event_id);
		state_fio->StateValue(event[i].expired_clock);
		state_fio->StateValue(event[i].loop_clock);
		state_fio->StateValue(event[i].accum_clocks);
		state_fio->StateValue(event[i].active);
		if(loading) {
			event[i].next = (event_t *)get_event(state_fio->FgetInt32_LE());
			event[i].prev = (event_t *)get_event(state_fio->FgetInt32_LE());
		} else {
			state_fio->FputInt32_LE(event[i].next != NULL ? event[i].next->index : -1);
			state_fio->FputInt32_LE(event[i].prev != NULL ? event[i].prev->index : -1);
		}
	}
	if(loading) {
		first_free_event = (event_t *)get_event(state_fio->FgetInt32_LE());
		first_fire_event = (event_t *)get_event(state_fio->FgetInt32_LE());
	} else {
		state_fio->FputInt32_LE(first_free_event != NULL ? first_free_event->index : -1);
		state_fio->FputInt32_LE(first_fire_event != NULL ? first_fire_event->index : -1);
	}
	state_fio->StateValue(frames_per_sec);
	state_fio->StateValue(next_frames_per_sec);
	state_fio->StateValue(lines_per_frame);
	state_fio->StateValue(next_lines_per_frame);
	state_fio->StateArray(dev_need_mix, sizeof(dev_need_mix), 1);
	state_fio->StateValue(need_mix);
	
	// post process
	if(loading) {
		if(sound_buffer) {
			memset(sound_buffer, 0, sound_samples * sizeof(uint16_t) * 2);
		}
		if(sound_tmp) {
			memset(sound_tmp, 0, sound_tmp_samples * sizeof(int32_t) * 2);
		}
		buffer_ptr = 0;
		mix_counter = 1;
		mix_limit = (int)((double)(emu->get_sound_rate() / 2000.0));  // per 0.5ms.
	}
	return true;
}

void* EVENT::get_event(int index)
{
	if(index >= 0 && index < MAX_EVENT) {
		return &event[index];
	}
	return NULL;
}
