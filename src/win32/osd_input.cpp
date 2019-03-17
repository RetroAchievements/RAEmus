/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2015.11.26-

	[ win32 input ]
*/

#include "osd.h"
#include "../fifo.h"

static const uint8_t vk_dik[256] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x0f, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x00,
	0x00, 0x00, 0x00, 0xc5, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x79, 0x7b, 0x00, 0x00,
	0x39, 0xc9, 0xd1, 0xcf, 0xc7, 0xcb, 0xc8, 0xcd, 0xd0, 0x00, 0x00, 0x00, 0x00, 0xd2, 0xd3, 0x00,
	0x0b, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x1e, 0x30, 0x2e, 0x20, 0x12, 0x21, 0x22, 0x23, 0x17, 0x24, 0x25, 0x26, 0x32, 0x31, 0x18,
	0x19, 0x10, 0x13, 0x1f, 0x14, 0x16, 0x2f, 0x11, 0x2d, 0x15, 0x2c, 0x00, 0x00, 0x00, 0x00, 0x00,
#ifdef USE_NUMPAD_ENTER
	0x52, 0x4f, 0x50, 0x51, 0x4b, 0x4c, 0x4d, 0x47, 0x48, 0x49, 0x37, 0x4e, 0x9c, 0x4a, 0x53, 0xb5,
#else
	0x52, 0x4f, 0x50, 0x51, 0x4b, 0x4c, 0x4d, 0x47, 0x48, 0x49, 0x37, 0x4e, 0x00, 0x4a, 0x53, 0xb5,
#endif
	0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 0x40, 0x41, 0x42, 0x43, 0x44, 0x57, 0x58, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x46, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	DIK_LSHIFT, DIK_RSHIFT, DIK_LCONTROL, DIK_RCONTROL, DIK_LMENU, DIK_RMENU,
	                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x92, 0x27, 0x33, 0x0c, 0x34, 0x35,
	0x91, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1a, 0x7d, 0x1b, 0x90, 0x00,
	0x00, 0x00, 0x2b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//	0x3a, 0x00, 0x70, 0x00, 0x94, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	// caps, kana, kanji
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

#define get_joy_range(min_value, max_value, lo_value, hi_value) \
{ \
	uint64_t center = ((uint64_t)min_value + (uint64_t)max_value) / 2; \
	lo_value = (DWORD)((center + (uint64_t)min_value) / 2); \
	hi_value = (DWORD)((center + (uint64_t)max_value) / 2); \
}

void OSD::initialize_input()
{
	// initialize status
	memset(key_status, 0, sizeof(key_status));
#ifdef USE_JOYSTICK
	memset(joy_status, 0, sizeof(joy_status));
	memset(joy_to_key_status, 0, sizeof(joy_to_key_status));
#endif
#ifdef USE_MOUSE
	memset(mouse_status, 0, sizeof(mouse_status));
#endif
	
	// initialize direct input
	dinput_key_available = false;
	lpdi = NULL;
	lpdikey = NULL;
	
	if(config.use_dinput) {
#if DIRECTINPUT_VERSION >= 0x0800
		if(SUCCEEDED(DirectInput8Create(instance_handle, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&lpdi, NULL))) {
#else
		if(SUCCEEDED(DirectInputCreate(instance_handle, DIRECTINPUT_VERSION, &lpdi, NULL))) {
#endif
			if(SUCCEEDED(lpdi->CreateDevice(GUID_SysKeyboard, &lpdikey, NULL))) {
				if(SUCCEEDED(lpdikey->SetDataFormat(&c_dfDIKeyboard))) {
					if(SUCCEEDED(lpdikey->SetCooperativeLevel(main_window_handle, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE))) {
						lpdikey->Acquire();
						dinput_key_available = true;
						memset(key_dik_prev, 0, sizeof(key_dik_prev));
					}
				}
			}
		}
	}
	
#ifdef USE_JOYSTICK
	// initialize joysticks
	joy_num = joyGetNumDevs();
	for(int i = 0; i < joy_num && i < 4; i++) {
		JOYCAPS joycaps;
		if(joyGetDevCaps(i, &joycaps, sizeof(joycaps)) == JOYERR_NOERROR) {
			joy_caps[i].wNumAxes = joycaps.wNumAxes;
			if(joycaps.wNumAxes >= 1) {
				get_joy_range(joycaps.wXmin, joycaps.wXmax, joy_caps[i].dwXposLo, joy_caps[i].dwXposHi);
			}
			if(joycaps.wNumAxes >= 1) {
				get_joy_range(joycaps.wYmin, joycaps.wYmax, joy_caps[i].dwYposLo, joy_caps[i].dwYposHi);
			}
			if(joycaps.wNumAxes >= 1) {
				get_joy_range(joycaps.wZmin, joycaps.wZmax, joy_caps[i].dwZposLo, joy_caps[i].dwZposHi);
			}
			if(joycaps.wNumAxes >= 1) {
				get_joy_range(joycaps.wRmin, joycaps.wRmax, joy_caps[i].dwRposLo, joy_caps[i].dwRposHi);
			}
			if(joycaps.wNumAxes >= 1) {
				get_joy_range(joycaps.wUmin, joycaps.wUmax, joy_caps[i].dwUposLo, joy_caps[i].dwUposHi);
			}
			if(joycaps.wNumAxes >= 1) {
				get_joy_range(joycaps.wVmin, joycaps.wVmax, joy_caps[i].dwVposLo, joy_caps[i].dwVposHi);
			}
			joy_caps[i].dwButtonsMask = (1 << min(16, joycaps.wNumButtons)) - 1;
		} else {
			joy_caps[i].wNumAxes = 2; // 2axes
			joy_caps[i].dwXposLo = joy_caps[i].dwYposLo = 0x3fff;
			joy_caps[i].dwXposHi = joy_caps[i].dwYposHi = 0xbfff;
			joy_caps[i].dwButtonsMask = 0x0f; // 4buttons
		}
	}
#endif
	
#ifdef USE_MOUSE
	// mouse emulation is disabled
	mouse_enabled = false;
#endif
	
	// initialize keycode convert table
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("keycode.cfg")), FILEIO_READ_BINARY)) {
		fio->Fread(keycode_conv, sizeof(keycode_conv), 1);
		fio->Fclose();
	} else {
		for(int i = 0; i < 256; i++) {
			keycode_conv[i] = i;
		}
	}
	delete fio;
	
	key_shift_pressed = key_shift_released = false;
	key_caps_locked = ((GetAsyncKeyState(VK_CAPITAL) & 0x0001) != 0);
	
	lost_focus = false;
}

void OSD::release_input()
{
#ifdef USE_MOUSE
	// release mouse
	if(mouse_enabled) {
		disable_mouse();
	}
#endif
	
	// release direct input
	if(lpdi) {
		if(lpdikey) {
			lpdikey->Unacquire();
			lpdikey->Release();
			lpdikey = NULL;
		}
		lpdi->Release();
		lpdi = NULL;
	}
}

void OSD::update_input()
{
#ifdef USE_AUTO_KEY
	if(!now_auto_key && !config.romaji_to_kana) {
#endif
		if(dinput_key_available) {
			// direct input
			memset(key_dik, 0, sizeof(key_dik));
//			lpdikey->Acquire();
			if(FAILED(lpdikey->GetDeviceState(256, key_dik))) {
				lpdikey->Acquire();
				lpdikey->GetDeviceState(256, key_dik);
			}
#if DIRECTINPUT_VERSION < 0x0800
			// DIK_RSHIFT is not detected on Vista or later
			if(vista_or_later) {
				key_dik[DIK_RSHIFT] = (GetAsyncKeyState(VK_RSHIFT) & 0x8000) ? 0x80 : 0;
			}
#endif
			// XXX: don't release shift key while numpad key is pressed
			uint8_t numpad_keys;
			numpad_keys  = key_dik[DIK_NUMPAD0];
			numpad_keys |= key_dik[DIK_NUMPAD1];
			numpad_keys |= key_dik[DIK_NUMPAD2];
			numpad_keys |= key_dik[DIK_NUMPAD3];
			numpad_keys |= key_dik[DIK_NUMPAD4];
			numpad_keys |= key_dik[DIK_NUMPAD5];
			numpad_keys |= key_dik[DIK_NUMPAD6];
			numpad_keys |= key_dik[DIK_NUMPAD7];
			numpad_keys |= key_dik[DIK_NUMPAD8];
			numpad_keys |= key_dik[DIK_NUMPAD9];
			if(numpad_keys & 0x80) {
				key_dik[DIK_LSHIFT] |= key_dik_prev[DIK_LSHIFT];
				key_dik[DIK_RSHIFT] |= key_dik_prev[DIK_RSHIFT];
			}
			key_dik[DIK_CIRCUMFLEX] |= key_dik[DIK_EQUALS     ];
			key_dik[DIK_COLON     ] |= key_dik[DIK_APOSTROPHE ];
			key_dik[DIK_YEN       ] |= key_dik[DIK_GRAVE      ];
#ifndef USE_NUMPAD_ENTER
			key_dik[DIK_RETURN    ] |= key_dik[DIK_NUMPADENTER];
#endif
			
			for(int vk = 0; vk < 256; vk++) {
				int dik = vk_dik[vk];
				if(dik) {
					if(key_dik[dik] & 0x80) {
						if(!(key_dik_prev[dik] & 0x80)) {
							key_down_native(vk, false);
						}
					} else {
//						if(key_dik_prev[dik] & 0x80) {
#ifdef USE_JOYSTICK
						if(!joy_to_key_status[vk])
#endif
							key_up_native(vk);
//						}
					}
				}
			}
			memcpy(key_dik_prev, key_dik, sizeof(key_dik_prev));
		} else {
			// update numpad key status
			if(key_shift_pressed && !key_shift_released) {
				if(key_status[VK_LSHIFT] == 0) {
					// shift key is newly pressed
					if(!key_status[VK_RSHIFT]) {
						vm->key_down(VK_SHIFT, false);
					}
					vm->key_down(VK_LSHIFT, false);
					key_status[VK_LSHIFT] = key_status[VK_SHIFT] = 0x80;
				}
			} else if(!key_shift_pressed && key_shift_released) {
				if(key_status[VK_LSHIFT] != 0) {
					// shift key is newly released
					vm->key_up(VK_LSHIFT);
					if(!key_status[VK_RSHIFT]) {
						vm->key_up(VK_SHIFT);
					}
					key_status[VK_LSHIFT] = key_status[VK_SHIFT] = 0;
				}
			}
			key_shift_pressed = key_shift_released = false;
		}
		
		// update shift + caps lock
		bool caps_locked = ((GetAsyncKeyState(VK_CAPITAL) & 0x0001) != 0);
		if(key_caps_locked != caps_locked) {
			key_down_native(0xf0, false);
			key_caps_locked = caps_locked;
		}
#ifdef USE_AUTO_KEY
	}
#endif
	
	// release keys
#ifdef USE_AUTO_KEY
	if(lost_focus && !now_auto_key) {
#else
	if(lost_focus) {
#endif
		// we lost key focus so release all pressed keys
		for(int i = 0; i < 256; i++) {
			if(key_status[i] & 0x80) {
				key_status[i] &= 0x7f;
				if(!key_status[i]) {
					vm->key_up(i);
				}
			}
		}
	} else {
		for(int i = 0; i < 256; i++) {
			if(key_status[i] & 0x7f) {
				key_status[i] = (key_status[i] & 0x80) | ((key_status[i] & 0x7f) - 1);
				if(!key_status[i]) {
					vm->key_up(i);
				}
			}
		}
	}
	lost_focus = false;
	
	// VK_$00 should be 0
	key_status[0] = 0;
	
#ifdef USE_JOYSTICK
	// update joystick status
	memset(joy_status, 0, sizeof(joy_status));
	
	for(int i = 0; i < joy_num && i < 4; i++) {
		JOYINFOEX joyinfo;
		joyinfo.dwSize = sizeof(JOYINFOEX);
		joyinfo.dwFlags = JOY_RETURNALL;
		joy_status[i] = 0;
		if(joyGetPosEx(i, &joyinfo) == JOYERR_NOERROR) {
			if(joy_caps[i].wNumAxes >= 2) {
				if(joyinfo.dwYpos < joy_caps[i].dwYposLo) joy_status[i] |= 0x00000001;	// up
				if(joyinfo.dwYpos > joy_caps[i].dwYposHi) joy_status[i] |= 0x00000002;	// down
			}
			if(joy_caps[i].wNumAxes >= 1) {
				if(joyinfo.dwXpos < joy_caps[i].dwXposLo) joy_status[i] |= 0x00000004;	// left
				if(joyinfo.dwXpos > joy_caps[i].dwXposHi) joy_status[i] |= 0x00000008;	// right
			}
			if(joy_caps[i].wNumAxes >= 3) {
				if(joyinfo.dwZpos < joy_caps[i].dwZposLo) joy_status[i] |= 0x00100000;
				if(joyinfo.dwZpos > joy_caps[i].dwZposHi) joy_status[i] |= 0x00200000;
			}
			if(joy_caps[i].wNumAxes >= 4) {
				if(joyinfo.dwRpos < joy_caps[i].dwRposLo) joy_status[i] |= 0x00400000;
				if(joyinfo.dwRpos > joy_caps[i].dwRposHi) joy_status[i] |= 0x00800000;
			}
			if(joy_caps[i].wNumAxes >= 5) {
				if(joyinfo.dwUpos < joy_caps[i].dwUposLo) joy_status[i] |= 0x01000000;
				if(joyinfo.dwUpos > joy_caps[i].dwUposHi) joy_status[i] |= 0x02000000;
			}
			if(joy_caps[i].wNumAxes >= 6) {
				if(joyinfo.dwVpos < joy_caps[i].dwVposLo) joy_status[i] |= 0x04000000;
				if(joyinfo.dwVpos > joy_caps[i].dwVposHi) joy_status[i] |= 0x08000000;
			}
			if(joyinfo.dwPOV != 0xffff) {
				static const uint32_t dir[8] = {
					0x10000000 + 0x00000000,
					0x10000000 + 0x20000000,
					0x00000000 + 0x20000000,
					0x40000000 + 0x20000000,
					0x40000000 + 0x00000000,
					0x40000000 + 0x80000000,
					0x00000000 + 0x80000000,
					0x10000000 + 0x80000000,
				};
				for(int j = 0; j < 9; j++) {
					if(joyinfo.dwPOV < (DWORD)(2250 + 4500 * j)) {
						joy_status[i] |= dir[j & 7];
						break;
					}
				}
			}
			joy_status[i] |= ((joyinfo.dwButtons & joy_caps[i].dwButtonsMask) << 4);
		}
	}
	if(config.use_joy_to_key) {
		int status[256] = {0};
		if(config.joy_to_key_type == 0) {
			// cursor key
			static const int vk[] = {VK_UP, VK_DOWN, VK_LEFT, VK_RIGHT};
			for(int i = 0; i < 4; i++) {
				if(joy_status[0] & (1 << i)) {
					status[vk[i]] = 1;
				}
			}
		} else if(config.joy_to_key_type == 1) {
			// numpad key (4-directions)
			static const int vk[] = {VK_NUMPAD8, VK_NUMPAD2, VK_NUMPAD4, VK_NUMPAD6};
			for(int i = 0; i < 4; i++) {
				if(joy_status[0] & (1 << i)) {
					status[vk[i]] = 1;
				}
			}
		} else if(config.joy_to_key_type == 2) {
			// numpad key (8-directions)
			switch(joy_status[0] & 0x0f) {
			case 0x02 + 0x04: status[VK_NUMPAD1] = 1; break; // down-left
			case 0x02       : status[VK_NUMPAD2] = 1; break; // down
			case 0x02 + 0x08: status[VK_NUMPAD3] = 1; break; // down-right
			case 0x00 + 0x04: status[VK_NUMPAD4] = 1; break; // left
//			case 0x00       : status[VK_NUMPAD5] = 1; break;
			case 0x00 + 0x08: status[VK_NUMPAD6] = 1; break; // right
			case 0x01 + 0x04: status[VK_NUMPAD7] = 1; break; // up-left
			case 0x01       : status[VK_NUMPAD8] = 1; break; // up
			case 0x01 + 0x08: status[VK_NUMPAD9] = 1; break; // up-right
			}
		}
		if(config.joy_to_key_type == 1 || config.joy_to_key_type == 2) {
			// numpad key
			if(config.joy_to_key_numpad5 && !(joy_status[0] & 0x0f)) {
				status[VK_NUMPAD5] = 1;
			}
		}
		for(int i = 0; i < 16; i++) {
			if(joy_status[0] & (1 << (i + 4))) {
				if(config.joy_to_key_buttons[i] < 0 && -config.joy_to_key_buttons[i] < 256) {
					status[-config.joy_to_key_buttons[i]] = 1;
				}
			}
		}
		for(int i = 0; i < 256; i++) {
			if(status[i]) {
				if(!joy_to_key_status[i]) {
					if(!(key_status[i] & 0x80)) {
						key_down_native(i, false);
						// do not keep key pressed
						if(config.joy_to_key_numpad5 && (i >= VK_NUMPAD1 && i <= VK_NUMPAD9)) {
							key_status[i] = KEY_KEEP_FRAMES;
						}
					}
					joy_to_key_status[i] = true;
				}
			} else {
				if(joy_to_key_status[i]) {
					if(key_status[i]) {
						key_up_native(i);
					}
					joy_to_key_status[i] = false;
				}
			}
		}
	}
#endif
	
#ifdef USE_MOUSE
	// update mouse status
	memset(mouse_status, 0, sizeof(mouse_status));
	
	if(mouse_enabled) {
		// get current status
		POINT pt;
		GetCursorPos(&pt);
		ScreenToClient(main_window_handle, &pt);
		mouse_status[0]  = pt.x - host_window_width / 2;
		mouse_status[1]  = pt.y - host_window_height / 2;
		mouse_status[2]  = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) ? 1 : 0;
		mouse_status[2] |= (GetAsyncKeyState(VK_RBUTTON) & 0x8000) ? 2 : 0;
		mouse_status[2] |= (GetAsyncKeyState(VK_MBUTTON) & 0x8000) ? 4 : 0;
		// move mouse cursor to the center of window
		if(!(mouse_status[0] == 0 && mouse_status[1] == 0)) {
			pt.x = host_window_width / 2;
			pt.y = host_window_height / 2;
			ClientToScreen(main_window_handle, &pt);
			SetCursorPos(pt.x, pt.y);
		}
	}
#endif
}

void OSD::key_down(int code, bool extended, bool repeat)
{
#ifdef USE_AUTO_KEY
	if(!now_auto_key && !config.romaji_to_kana) {
#endif
		if(!dinput_key_available) {
			if(code == VK_SHIFT) {
				if(!(key_status[VK_RSHIFT] & 0x80) && (GetAsyncKeyState(VK_RSHIFT) & 0x8000)) {
					key_down_native(VK_RSHIFT, repeat);
				}
				if(!(key_status[VK_LSHIFT] & 0x80) && (GetAsyncKeyState(VK_LSHIFT) & 0x8000)) {
					code = VK_LSHIFT;
				} else {
					return;
				}
			} else if(code == VK_CONTROL) {
				if(!(key_status[VK_RCONTROL] & 0x80) && (GetAsyncKeyState(VK_RCONTROL) & 0x8000)) {
					key_down_native(VK_RCONTROL, repeat);
				}
				if(!(key_status[VK_LCONTROL] & 0x80) && (GetAsyncKeyState(VK_LCONTROL) & 0x8000)) {
					code = VK_LCONTROL;
				} else {
					return;
				}
			} else if(code == VK_MENU) {
				if(!(key_status[VK_RMENU] & 0x80) && (GetAsyncKeyState(VK_RMENU) & 0x8000)) {
					key_down_native(VK_RMENU, repeat);
				}
				if(!(key_status[VK_LMENU] & 0x80) && (GetAsyncKeyState(VK_LMENU) & 0x8000)) {
					code = VK_LMENU;
				} else {
					return;
				}
			}
//			if(code == VK_LSHIFT || code == VK_RSHIFT) {
			if(code == VK_LSHIFT) {
				key_shift_pressed = true;
				return;
			}
			if(!extended) {
				switch(code) {
				case VK_INSERT:
					if(key_shift_pressed || key_shift_released || key_status[VK_NUMPAD0]) {
						code = VK_NUMPAD0;
						key_shift_pressed = true;
					}
					break;
				case VK_END:
					if(key_shift_pressed || key_shift_released || key_status[VK_NUMPAD1]) {
						code = VK_NUMPAD1;
						key_shift_pressed = true;
					}
					break;
				case VK_DOWN:
					if(key_shift_pressed || key_shift_released || key_status[VK_NUMPAD2]) {
						code = VK_NUMPAD2;
						key_shift_pressed = true;
					}
					break;
				case VK_NEXT:
					if(key_shift_pressed || key_shift_released || key_status[VK_NUMPAD3]) {
						code = VK_NUMPAD3;
						key_shift_pressed = true;
					}
					break;
				case VK_LEFT:
					if(key_shift_pressed || key_shift_released || key_status[VK_NUMPAD4]) {
						code = VK_NUMPAD4;
						key_shift_pressed = true;
					}
					break;
				case VK_CLEAR:
					if(key_shift_pressed || key_shift_released || key_status[VK_NUMPAD5]) {
						code = VK_NUMPAD5;
						key_shift_pressed = true;
					}
					break;
				case VK_RIGHT:
					if(key_shift_pressed || key_shift_released || key_status[VK_NUMPAD6]) {
						code = VK_NUMPAD6;
						key_shift_pressed = true;
					}
					break;
				case VK_HOME:
					if(key_shift_pressed || key_shift_released || key_status[VK_NUMPAD7]) {
						code = VK_NUMPAD7;
						key_shift_pressed = true;
					}
					break;
				case VK_UP:
					if(key_shift_pressed || key_shift_released || key_status[VK_NUMPAD8]) {
						code = VK_NUMPAD8;
						key_shift_pressed = true;
					}
					break;
				case VK_PRIOR:
					if(key_shift_pressed || key_shift_released || key_status[VK_NUMPAD9]) {
						code = VK_NUMPAD9;
						key_shift_pressed = true;
					}
					break;
				}
			}
			key_down_native(code, repeat);
		} else {
			if(repeat || code == 0xf0 || code == 0xf1 || code == 0xf2 || code == 0xf3 || code == 0xf4) {
				key_down_native(code, repeat);
			}
		}
#ifdef USE_AUTO_KEY
	}
#endif
}

void OSD::key_up(int code, bool extended)
{
#ifdef USE_AUTO_KEY
	if(!now_auto_key && !config.romaji_to_kana) {
#endif
		if(!dinput_key_available) {
			if(code == VK_SHIFT) {
				if((key_status[VK_RSHIFT] & 0x80) && !(GetAsyncKeyState(VK_RSHIFT) & 0x8000)) {
					key_up_native(VK_RSHIFT);
				}
				if((key_status[VK_LSHIFT] & 0x80) && !(GetAsyncKeyState(VK_LSHIFT) & 0x8000)) {
					code = VK_LSHIFT;
				} else {
					return;
				}
			} else if(code == VK_CONTROL) {
				if((key_status[VK_RCONTROL] & 0x80) && !(GetAsyncKeyState(VK_RCONTROL) & 0x8000)) {
					key_up_native(VK_RCONTROL);
				}
				if((key_status[VK_LCONTROL] & 0x80) && !(GetAsyncKeyState(VK_LCONTROL) & 0x8000)) {
					code = VK_LCONTROL;
				} else {
					return;
				}
			} else if(code == VK_MENU) {
				if((key_status[VK_RMENU] & 0x80) && !(GetAsyncKeyState(VK_RMENU) & 0x8000)) {
					key_up_native(VK_RMENU);
				}
				if((key_status[VK_LMENU] & 0x80) && !(GetAsyncKeyState(VK_LMENU) & 0x8000)) {
					code = VK_LMENU;
				} else {
					return;
				}
			}
//			if(code == VK_LSHIFT || code == VK_RSHIFT) {
			if(code == VK_LSHIFT) {
				key_shift_pressed = false;
				key_shift_released = true;
				return;
			}
			if(!extended) {
				switch(code) {
				case VK_NUMPAD0: case VK_INSERT:
					key_up_native(VK_NUMPAD0);
					key_up_native(VK_INSERT);
					return;
				case VK_NUMPAD1: case VK_END:
					key_up_native(VK_NUMPAD1);
					key_up_native(VK_END);
					return;
				case VK_NUMPAD2: case VK_DOWN:
					key_up_native(VK_NUMPAD2);
					key_up_native(VK_DOWN);
					return;
				case VK_NUMPAD3: case VK_NEXT:
					key_up_native(VK_NUMPAD3);
					key_up_native(VK_NEXT);
					return;
				case VK_NUMPAD4: case VK_LEFT:
					key_up_native(VK_NUMPAD4);
					key_up_native(VK_LEFT);
					return;
				case VK_NUMPAD5: case VK_CLEAR:
					key_up_native(VK_NUMPAD5);
					key_up_native(VK_CLEAR);
					return;
				case VK_NUMPAD6: case VK_RIGHT:
					key_up_native(VK_NUMPAD6);
					key_up_native(VK_RIGHT);
					return;
				case VK_NUMPAD7: case VK_HOME:
					key_up_native(VK_NUMPAD7);
					key_up_native(VK_HOME);
					return;
				case VK_NUMPAD8: case VK_UP:
					key_up_native(VK_NUMPAD8);
					key_up_native(VK_UP);
					return;
				case VK_NUMPAD9: case VK_PRIOR:
					key_up_native(VK_NUMPAD9);
					key_up_native(VK_PRIOR);
					return;
				}
			}
			key_up_native(code);
		}
#ifdef USE_AUTO_KEY
	}
#endif
}

void OSD::key_down_native(int code, bool repeat)
{
	bool keep_frames = false;
	
	if(code == 0xf0) {
		code = VK_CAPITAL;
		keep_frames = true;
	} else if(code == 0xf1 || code == 0xf2) {
		code = VK_KANA;
		keep_frames = true;
	} else if(code == 0xf3 || code == 0xf4) {
		code = VK_KANJI;
		keep_frames = true;
	}
	if(!(code == VK_LSHIFT || code == VK_RSHIFT || code == VK_LCONTROL || code == VK_RCONTROL || code == VK_LMENU || code == VK_RMENU)) {
		code = keycode_conv[code];
	}
	if(key_status[code] == 0 || keep_frames) {
		repeat = false;
	}
#ifdef DONT_KEEEP_KEY_PRESSED
	if(!(code == VK_LSHIFT || code == VK_RSHIFT || code == VK_LCONTROL || code == VK_RCONTROL || code == VK_LMENU || code == VK_RMENU)) {
		key_status[code] = KEY_KEEP_FRAMES;
	} else
#endif
	key_status[code] = keep_frames ? KEY_KEEP_FRAMES : 0x80;
	
	uint8_t prev_shift = key_status[VK_SHIFT];
	uint8_t prev_control = key_status[VK_CONTROL];
	uint8_t prev_menu = key_status[VK_MENU];
	
	key_status[VK_SHIFT] = key_status[VK_LSHIFT] | key_status[VK_RSHIFT];
	key_status[VK_CONTROL] = key_status[VK_LCONTROL] | key_status[VK_RCONTROL];
	key_status[VK_MENU] = key_status[VK_LMENU] | key_status[VK_RMENU];
	
	if(code == VK_LSHIFT || code == VK_RSHIFT) {
		if(prev_shift == 0 && key_status[VK_SHIFT] != 0) {
			vm->key_down(VK_SHIFT, repeat);
		}
	} else if(code == VK_LCONTROL|| code == VK_RCONTROL) {
		if(prev_control == 0 && key_status[VK_CONTROL] != 0) {
			vm->key_down(VK_CONTROL, repeat);
		}
	} else if(code == VK_LMENU|| code == VK_RMENU) {
		if(prev_menu == 0 && key_status[VK_MENU] != 0) {
			vm->key_down(VK_MENU, repeat);
		}
	}
	vm->key_down(code, repeat);
}

void OSD::key_up_native(int code)
{
	if(!(code == VK_LSHIFT || code == VK_RSHIFT || code == VK_LCONTROL || code == VK_RCONTROL || code == VK_LMENU || code == VK_RMENU)) {
		code = keycode_conv[code];
	}
	if(key_status[code] == 0) {
		return;
	}
	if((key_status[code] &= 0x7f) != 0) {
		return;
	}
	vm->key_up(code);
	
	uint8_t prev_shift = key_status[VK_SHIFT];
	uint8_t prev_control = key_status[VK_CONTROL];
	uint8_t prev_menu = key_status[VK_MENU];
	
	key_status[VK_SHIFT] = key_status[VK_LSHIFT] | key_status[VK_RSHIFT];
	key_status[VK_CONTROL] = key_status[VK_LCONTROL] | key_status[VK_RCONTROL];
	key_status[VK_MENU] = key_status[VK_LMENU] | key_status[VK_RMENU];
	
	if(code == VK_LSHIFT || code == VK_RSHIFT) {
		if(prev_shift != 0 && key_status[VK_SHIFT] == 0) {
			vm->key_up(VK_SHIFT);
		}
	} else if(code == VK_LCONTROL|| code == VK_RCONTROL) {
		if(prev_control != 0 && key_status[VK_CONTROL] == 0) {
			vm->key_up(VK_CONTROL);
		}
	} else if(code == VK_LMENU || code == VK_RMENU) {
		if(prev_menu != 0 && key_status[VK_MENU] == 0) {
			vm->key_up(VK_MENU);
		}
	}
}

#ifdef USE_MOUSE
void OSD::enable_mouse()
{
	// enable mouse emulation
	if(!mouse_enabled) {
		// hide mouse cursor
		ShowCursor(FALSE);
		// move mouse cursor to the center of window
		POINT pt;
		pt.x = host_window_width / 2;
		pt.y = host_window_height / 2;
		ClientToScreen(main_window_handle, &pt);
		SetCursorPos(pt.x, pt.y);
	}
	mouse_enabled = true;
}

void OSD::disable_mouse()
{
	// disable mouse emulation
	if(mouse_enabled) {
		ShowCursor(TRUE);
	}
	mouse_enabled = false;
}

void OSD::toggle_mouse()
{
	// toggle mouse enable / disable
	if(mouse_enabled) {
		disable_mouse();
	} else {
		enable_mouse();
	}
}
#endif

