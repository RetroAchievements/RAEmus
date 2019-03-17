/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2015.11.26-

	[ win32 sound ]
*/

#include "osd.h"
#include "../fileio.h"

#define DSOUND_BUFFER_SIZE (DWORD)(sound_samples * 8)
#define DSOUND_BUFFER_HALF (DWORD)(sound_samples * 4)

void OSD::initialize_sound(int rate, int samples)
{
	sound_rate = rate;
	sound_samples = samples;
	sound_available = sound_started = sound_muted = now_record_sound = false;
	rec_sound_buffer_ptr = 0;
	
	// initialize direct sound
	PCMWAVEFORMAT pcmwf;
	DSBUFFERDESC dsbd;
	WAVEFORMATEX wfex;
	
	if(FAILED(DirectSoundCreate(NULL, &lpds, NULL))) {
		return;
	}
	if(FAILED(lpds->SetCooperativeLevel(main_window_handle, DSSCL_PRIORITY))) {
		return;
	}
	
	// primary buffer
	ZeroMemory(&dsbd, sizeof(dsbd));
	dsbd.dwSize = sizeof(dsbd);
	dsbd.dwFlags = DSBCAPS_PRIMARYBUFFER;
	if(FAILED(lpds->CreateSoundBuffer(&dsbd, &lpdsPrimaryBuffer, NULL))) {
		return;
	}
	ZeroMemory(&wfex, sizeof(wfex));
	wfex.wFormatTag = WAVE_FORMAT_PCM;
	wfex.nChannels = 2;
	wfex.wBitsPerSample = 16;
	wfex.nSamplesPerSec = sound_rate;
	wfex.nBlockAlign = wfex.nChannels * wfex.wBitsPerSample / 8;
	wfex.nAvgBytesPerSec = wfex.nSamplesPerSec * wfex.nBlockAlign;
	if(FAILED(lpdsPrimaryBuffer->SetFormat(&wfex))) {
		return;
	}
	
	// secondary buffer
	ZeroMemory(&pcmwf, sizeof(pcmwf));
	pcmwf.wf.wFormatTag = WAVE_FORMAT_PCM;
	pcmwf.wf.nChannels = 2;
	pcmwf.wBitsPerSample = 16;
	pcmwf.wf.nSamplesPerSec = sound_rate;
	pcmwf.wf.nBlockAlign = pcmwf.wf.nChannels * pcmwf.wBitsPerSample / 8;
	pcmwf.wf.nAvgBytesPerSec = pcmwf.wf.nSamplesPerSec * pcmwf.wf.nBlockAlign;
	ZeroMemory(&dsbd, sizeof(dsbd));
	dsbd.dwSize = sizeof(dsbd);
	dsbd.dwFlags = DSBCAPS_STICKYFOCUS | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_LOCSOFTWARE;
	dsbd.dwBufferBytes = DSOUND_BUFFER_SIZE;
	dsbd.lpwfxFormat = (LPWAVEFORMATEX)&pcmwf;
	if(FAILED(lpds->CreateSoundBuffer(&dsbd, &lpdsSecondaryBuffer, NULL))) {
		return;
	}
	
	sound_available = sound_first_half = true;
}

void OSD::release_sound()
{
	// release direct sound
	if(lpdsPrimaryBuffer) {
		lpdsPrimaryBuffer->Release();
		lpdsPrimaryBuffer = NULL;
	}
	if(lpdsSecondaryBuffer) {
		lpdsSecondaryBuffer->Release();
		lpdsSecondaryBuffer = NULL;
	}
	if(lpds) {
		lpds->Release();
		lpds = NULL;
	}
	
	// stop recording
	stop_record_sound();
}

void OSD::update_sound(int* extra_frames)
{
	*extra_frames = 0;
#ifdef USE_DEBUGGER
//	if(now_debugging) {
//		return;
//	}
#endif
	sound_muted = false;
	
	if(sound_available) {
		DWORD play_c, write_c, offset, size1, size2;
		WORD *ptr1, *ptr2;
		
		// start play
		if(!sound_started) {
			lpdsSecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);
			sound_started = true;
			return;
		}
		
		// check current position
		if(FAILED(lpdsSecondaryBuffer->GetCurrentPosition(&play_c, &write_c))) {
			return;
		}
		if(sound_first_half) {
			if(play_c < DSOUND_BUFFER_HALF) {
				return;
			}
			offset = 0;
		} else {
			if(play_c >= DSOUND_BUFFER_HALF) {
				return;
			}
			offset = DSOUND_BUFFER_HALF;
		}
		
		// sound buffer must be updated
		uint16_t* sound_buffer = vm->create_sound(extra_frames);
		if(now_record_sound) {
			// record sound
			if(sound_samples > rec_sound_buffer_ptr) {
				int samples = sound_samples - rec_sound_buffer_ptr;
				int length = samples * sizeof(uint16_t) * 2; // stereo
				rec_sound_fio->Fwrite(sound_buffer + rec_sound_buffer_ptr * 2, length, 1);
				rec_sound_bytes += length;
				if(now_record_video) {
					// sync video recording
					static double frames = 0;
					static int prev_samples = -1;
					static double prev_fps = -1;
					double fps = vm->get_frame_rate();
					if(prev_samples != samples || prev_fps != fps) {
						prev_samples = samples;
						prev_fps = fps;
						frames = fps * (double)samples / (double)sound_rate;
					}
					rec_video_frames -= frames;
					if(rec_video_frames > 2) {
						rec_video_run_frames -= (rec_video_frames - 2);
					} else if(rec_video_frames < -2) {
						rec_video_run_frames -= (rec_video_frames + 2);
					}
//					rec_video_run_frames -= rec_video_frames;
				}
			}
			rec_sound_buffer_ptr = 0;
		}
		if(lpdsSecondaryBuffer->Lock(offset, DSOUND_BUFFER_HALF, (void **)&ptr1, &size1, (void**)&ptr2, &size2, 0) == DSERR_BUFFERLOST) {
			lpdsSecondaryBuffer->Restore();
		}
		if(sound_buffer) {
			if(ptr1) {
				CopyMemory(ptr1, sound_buffer, size1);
			}
			if(ptr2) {
				CopyMemory(ptr2, sound_buffer + size1, size2);
			}
		}
		lpdsSecondaryBuffer->Unlock(ptr1, size1, ptr2, size2);
		sound_first_half = !sound_first_half;
	}
}

void OSD::mute_sound()
{
	if(sound_available && !sound_muted) {
		// check current position
		DWORD size1, size2;
		WORD *ptr1, *ptr2;
		
		if(lpdsSecondaryBuffer->Lock(0, DSOUND_BUFFER_SIZE, (void **)&ptr1, &size1, (void**)&ptr2, &size2, 0) == DSERR_BUFFERLOST) {
			lpdsSecondaryBuffer->Restore();
		}
		if(ptr1) {
			ZeroMemory(ptr1, size1);
		}
		if(ptr2) {
			ZeroMemory(ptr2, size2);
		}
		lpdsSecondaryBuffer->Unlock(ptr1, size1, ptr2, size2);
	}
	sound_muted = true;
}

void OSD::stop_sound()
{
	if(sound_available && sound_started) {
		lpdsSecondaryBuffer->Stop();
		sound_started = false;
	}
}

void OSD::start_record_sound()
{
	if(!now_record_sound) {
		// create wave file
		create_date_file_path(sound_file_path, _MAX_PATH, _T("wav"));
		rec_sound_fio = new FILEIO();
		if(rec_sound_fio->Fopen(sound_file_path, FILEIO_WRITE_BINARY)) {
			// write dummy wave header
			wav_header_t wav_header;
			wav_chunk_t wav_chunk;
			memset(&wav_header, 0, sizeof(wav_header));
			memset(&wav_chunk, 0, sizeof(wav_chunk));
			rec_sound_fio->Fwrite(&wav_header, sizeof(wav_header), 1);
			rec_sound_fio->Fwrite(&wav_chunk, sizeof(wav_chunk), 1);
			
			rec_sound_bytes = 0;
			rec_sound_buffer_ptr = vm->get_sound_buffer_ptr();
			now_record_sound = true;
		} else {
			// failed to open the wave file
			delete rec_sound_fio;
		}
	}
}

void OSD::stop_record_sound()
{
	if(now_record_sound) {
		if(rec_sound_bytes == 0) {
			rec_sound_fio->Fclose();
			FILEIO::RemoveFile(sound_file_path);
		} else {
			// update wave header
			wav_header_t wav_header;
			wav_chunk_t wav_chunk;
			
			memcpy(wav_header.riff_chunk.id, "RIFF", 4);
			wav_header.riff_chunk.size = rec_sound_bytes + sizeof(wav_header) + sizeof(wav_chunk) - 8;
			memcpy(wav_header.wave, "WAVE", 4);
			memcpy(wav_header.fmt_chunk.id, "fmt ", 4);
			wav_header.fmt_chunk.size = 16;
			wav_header.format_id = 1;
			wav_header.channels = 2;
			wav_header.sample_bits = 16;
			wav_header.sample_rate = sound_rate;
			wav_header.block_size = wav_header.channels * wav_header.sample_bits / 8;
			wav_header.data_speed = wav_header.sample_rate * wav_header.block_size;
			
			memcpy(wav_chunk.id, "data", 4);
			wav_chunk.size = rec_sound_bytes;
			
			rec_sound_fio->Fseek(0, FILEIO_SEEK_SET);
			rec_sound_fio->Fwrite(&wav_header, sizeof(wav_header), 1);
			rec_sound_fio->Fwrite(&wav_chunk, sizeof(wav_chunk), 1);
			rec_sound_fio->Fclose();
		}
		delete rec_sound_fio;
		now_record_sound = false;
	}
}

void OSD::restart_record_sound()
{
	bool tmp = now_record_sound;
	stop_record_sound();
	if(tmp) {
		start_record_sound();
	}
}

