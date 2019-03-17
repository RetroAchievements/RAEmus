/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ debugger console ]
*/

#include <stdlib.h>
#include <io.h>
#include <fcntl.h>
#include "vm/device.h"
#include "vm/debugger.h"
#include "vm/vm.h"
#include "fileio.h"

#if USE_RETROACHIEVEMENTS
#include "retroachievements.h"
#endif

#ifdef USE_DEBUGGER

static FILEIO* logfile = NULL;
static FILEIO* cmdfile = NULL;

const _TCHAR *my_absolute_path(const _TCHAR *file_name)
{
	static _TCHAR file_path[_MAX_PATH];
	
	if(is_absolute_path(file_name)) {
		my_tcscpy_s(file_path, _MAX_PATH, file_name);
	} else {
		my_stprintf_s(file_path, _MAX_PATH, _T("%s%s"), get_initial_current_path(), file_name);
	}
	return (const _TCHAR *)file_path;
}

void my_printf(OSD *osd, const _TCHAR *format, ...)
{
	_TCHAR buffer[8192];
	va_list ap;
	
	va_start(ap, format);
	my_vstprintf_s(buffer, array_length(buffer), format, ap);
	va_end(ap);
	
	if(logfile != NULL && logfile->IsOpened()) {
		logfile->Fwrite(buffer, _tcslen(buffer) * sizeof(_TCHAR), 1);
	}
	osd->write_console(buffer, _tcslen(buffer));
}

void my_putch(OSD *osd, _TCHAR c)
{
	if(logfile != NULL && logfile->IsOpened()) {
		logfile->Fwrite(&c, sizeof(_TCHAR), 1);
	}
	osd->write_console(&c, 1);
}

uint32_t my_hexatoi(DEVICE *device, const _TCHAR *str)
{
	_TCHAR tmp[1024], *s;
	symbol_t *first_symbol = NULL;
	
	if(str == NULL || _tcslen(str) == 0) {
		return 0;
	}
	my_tcscpy_s(tmp, array_length(tmp), str);
	
	if(device != NULL) {
		DEBUGGER *debugger = (DEBUGGER *)device->get_debugger();
		if(debugger != NULL) {
			first_symbol = debugger->first_symbol;
		}
	}
	for(symbol_t* symbol = first_symbol; symbol; symbol = symbol->next_symbol) {
		if(_tcsicmp(symbol->name, str) == 0) {
			return symbol->addr;
		}
	}
	if(_tcslen(tmp) == 3 && tmp[0] == _T('\'') && tmp[2] == _T('\'')) {
		// ank
		return tmp[1] & 0xff;
	} else if((s = _tcsstr(tmp, _T(":"))) != NULL) {
		// 0000:0000
		s[0] = _T('\0');
		return (my_hexatoi(device, tmp) << 4) + my_hexatoi(device, s + 1);
	} else if(tmp[0] == _T('%')) {
		// decimal
		return _tstoi(tmp + 1);
	}
	return _tcstoul(tmp, NULL, 16);
}

uint8_t my_hexatob(char *value)
{
	char tmp[3];
	tmp[0] = value[0];
	tmp[1] = value[1];
	tmp[2] = '\0';
	return (uint8_t)strtoul(tmp, NULL, 16);
}

uint16_t my_hexatow(char *value)
{
	char tmp[5];
	tmp[0] = value[0];
	tmp[1] = value[1];
	tmp[2] = value[2];
	tmp[3] = value[3];
	tmp[4] = '\0';
	return (uint16_t)strtoul(tmp, NULL, 16);
}

const _TCHAR *my_get_symbol(DEVICE *device, uint32_t addr)
{
	symbol_t *first_symbol = NULL;
	
	if(device != NULL) {
		DEBUGGER *debugger = (DEBUGGER *)device->get_debugger();
		if(debugger != NULL) {
			first_symbol = debugger->first_symbol;
		}
	}
	return get_symbol(first_symbol, addr);
}

const _TCHAR *my_get_value_or_symbol(DEVICE *device, const _TCHAR *format, uint32_t addr)
{
	symbol_t *first_symbol = NULL;
	
	if(device != NULL) {
		DEBUGGER *debugger = (DEBUGGER *)device->get_debugger();
		if(debugger != NULL) {
			first_symbol = debugger->first_symbol;
		}
	}
	return get_value_or_symbol(first_symbol, format, addr);
}

const _TCHAR *my_get_value_and_symbol(DEVICE *device, const _TCHAR *format, uint32_t addr)
{
	symbol_t *first_symbol = NULL;
	
	if(device != NULL) {
		DEBUGGER *debugger = (DEBUGGER *)device->get_debugger();
		if(debugger != NULL) {
			first_symbol = debugger->first_symbol;
		}
	}
	return get_value_and_symbol(first_symbol, format, addr);
}

break_point_t *get_break_point(DEBUGGER *debugger, const _TCHAR *command)
{
	if(command[0] == _T('B') || command[0] == _T('b') || command[0] == _T('C') || command[0] == _T('c')) {
		return &debugger->bp;
	} else if(command[0] == _T('R') || command[0] == _T('r')) {
		return &debugger->rbp;
	} else if(command[0] == _T('W') || command[0] == _T('w')) {
		return &debugger->wbp;
	} else if(command[0] == _T('I') || command[0] == _T('i')) {
		return &debugger->ibp;
	} else if(command[0] == _T('O') || command[0] == _T('o')) {
		return &debugger->obp;
	}
	return NULL;
}

void show_break_reason(OSD *osd, DEVICE *cpu, DEVICE *target, bool hide_bp)
{
	DEBUGGER *cpu_debugger = (DEBUGGER *)cpu->get_debugger();
	DEBUGGER *target_debugger = (DEBUGGER *)target->get_debugger();
	
	osd->set_console_text_attribute(OSD_CONSOLE_RED | OSD_CONSOLE_INTENSITY);
	
	if(cpu_debugger != NULL) {
		if(cpu_debugger->bp.hit && !hide_bp) {
			my_printf(osd, _T("breaked at %s: breakpoint %s was hit\n"),
				my_get_value_and_symbol(cpu, _T("%08X"), cpu->get_next_pc()),
				my_get_value_and_symbol(cpu, _T("%08X"), cpu_debugger->bp.hit_addr));
		}
		if(cpu_debugger->rbp.hit) {
			my_printf(osd, _T("breaked at %s: memory %s was read at %s\n"),
				my_get_value_and_symbol(cpu, _T("%08X"), cpu->get_next_pc()),
				my_get_value_and_symbol(cpu, _T("%08X"), cpu_debugger->rbp.hit_addr),
				my_get_value_and_symbol(cpu, _T("%08X"), cpu->get_pc()));
		}
		if(cpu_debugger->wbp.hit) {
			my_printf(osd, _T("breaked at %s: memory %s was written at %s\n"),
				my_get_value_and_symbol(cpu, _T("%08X"), cpu->get_next_pc()),
				my_get_value_and_symbol(cpu, _T("%08X"), cpu_debugger->wbp.hit_addr),
				my_get_value_and_symbol(cpu, _T("%08X"), cpu->get_pc()));
		}
		if(cpu_debugger->ibp.hit) {
			my_printf(osd, _T("breaked at %s: port %s was read at %s\n"),
				my_get_value_and_symbol(cpu, _T("%08X"), cpu->get_next_pc()),
				my_get_value_and_symbol(cpu, _T("%08X"), cpu_debugger->ibp.hit_addr),
				my_get_value_and_symbol(cpu, _T("%08X"), cpu->get_pc()));
		}
		if(cpu_debugger->obp.hit) {
			my_printf(osd, _T("breaked at %s: port %s was written at %s\n"),
				my_get_value_and_symbol(cpu, _T("%08X"), cpu->get_next_pc()),
				my_get_value_and_symbol(cpu, _T("%08X"), cpu_debugger->obp.hit_addr),
				my_get_value_and_symbol(cpu, _T("%08X"), cpu->get_pc()));
		}
	}
	if(target != cpu && target_debugger != NULL) {
		if(target_debugger->bp.hit) {
			my_printf(osd, _T("breaked at %s: %s breakpoint %s was hit\n"),
				my_get_value_and_symbol(cpu, _T("%08X"), cpu->get_next_pc()),
				target->get_device_name(),
				my_get_value_and_symbol(target, _T("%08X"), target_debugger->bp.hit_addr));
		}
		if(target_debugger->rbp.hit) {
			my_printf(osd, _T("breaked at %s: %s memory %s was read at %s\n"),
				my_get_value_and_symbol(cpu, _T("%08X"), cpu->get_next_pc()),
				target->get_device_name(),
				my_get_value_and_symbol(target, _T("%08X"), target_debugger->rbp.hit_addr),
				my_get_value_and_symbol(cpu, _T("%08X"), cpu->get_pc()));
		}
		if(target_debugger->wbp.hit) {
			my_printf(osd, _T("breaked at %s: %s memory %s was written at %s\n"),
				my_get_value_and_symbol(cpu, _T("%08X"), cpu->get_next_pc()),
				target->get_device_name(),
				my_get_value_and_symbol(target, _T("%08X"), target_debugger->wbp.hit_addr),
				my_get_value_and_symbol(cpu, _T("%08X"), cpu->get_pc()));
		}
		if(target_debugger->ibp.hit) {
			my_printf(osd, _T("breaked at %s: %s port %s was read at %s\n"),
				my_get_value_and_symbol(cpu, _T("%08X"), cpu->get_next_pc()),
				target->get_device_name(),
				my_get_value_and_symbol(target, _T("%08X"), target_debugger->ibp.hit_addr),
				my_get_value_and_symbol(cpu, _T("%08X"), cpu->get_pc()));
		}
		if(target_debugger->obp.hit) {
			my_printf(osd, _T("breaked at %s: %s port %s was written at %s\n"),
				my_get_value_and_symbol(cpu, _T("%08X"), cpu->get_next_pc()),
				target->get_device_name(),
				my_get_value_and_symbol(target, _T("%08X"), target_debugger->obp.hit_addr),
				my_get_value_and_symbol(cpu, _T("%08X"), cpu->get_pc()));
		}
	}
}

#ifdef _MSC_VER
unsigned __stdcall debugger_thread(void *lpx)
#else
void* debugger_thread(void *lpx)
#endif
{
	volatile debugger_thread_t *p = (debugger_thread_t *)lpx;
	p->running = true;
	
	// initialize console
	_TCHAR buffer[8192];
	bool cp932 = (p->osd->get_console_code_page() == 932);
	
	p->osd->open_console(create_string(_T("Debugger - %s"), _T(DEVICE_NAME)));
	
	// break cpu
	DEVICE *cpu = p->vm->get_cpu(p->cpu_index);
	DEBUGGER *cpu_debugger = (DEBUGGER *)cpu->get_debugger();
	DEVICE *target = cpu;
	DEBUGGER *target_debugger = cpu_debugger;
	
	cpu_debugger->set_context_child(NULL);
	cpu_debugger->now_going = false;
	cpu_debugger->now_debugging = true;
	int wait_count = 0;
	while(!p->request_terminate && !(cpu_debugger->now_suspended && cpu_debugger->now_waiting)) {
		if((wait_count++) == 100) {
			p->osd->set_console_text_attribute(OSD_CONSOLE_RED | OSD_CONSOLE_INTENSITY);
			my_printf(p->osd, _T("waiting until cpu is suspended...\n"));
		}
		p->osd->sleep(10);
	}
	
	uint32_t dump_addr = 0;
	uint32_t dasm_addr = cpu->get_next_pc();
	
	p->osd->set_console_text_attribute(OSD_CONSOLE_RED | OSD_CONSOLE_GREEN | OSD_CONSOLE_BLUE | OSD_CONSOLE_INTENSITY);
	if(cpu->get_debug_regs_info(buffer, array_length(buffer))) {
		my_printf(p->osd, _T("%s\n"), buffer);
	}
	
	p->osd->set_console_text_attribute(OSD_CONSOLE_RED | OSD_CONSOLE_INTENSITY);
	my_printf(p->osd, _T("breaked at %s\n"), my_get_value_and_symbol(cpu, _T("%08X"), cpu->get_next_pc()));
	
	p->osd->set_console_text_attribute(OSD_CONSOLE_GREEN | OSD_CONSOLE_BLUE | OSD_CONSOLE_INTENSITY);
	cpu->debug_dasm(cpu->get_next_pc(), buffer, array_length(buffer));
	my_printf(p->osd, _T("next\t%s  %s\n"), my_get_value_and_symbol(cpu, _T("%08X"), cpu->get_next_pc()), buffer);
	p->osd->set_console_text_attribute(OSD_CONSOLE_RED | OSD_CONSOLE_GREEN | OSD_CONSOLE_BLUE | OSD_CONSOLE_INTENSITY);
	
	// initialize files
	logfile = NULL;
	cmdfile = NULL;
	
	_TCHAR command[MAX_COMMAND_LENGTH + 1];
	_TCHAR prev_command[MAX_COMMAND_LENGTH + 1];
	
	memset(prev_command, 0, sizeof(prev_command));
	
	while(!p->request_terminate) {
		p->emu->draw_screen();
		
		my_printf(p->osd, _T("- "));
		
		// get command
		_TCHAR ir[16 + 2];
		int enter_ptr = 0;
		int history_ptr = 0;
		bool enter_done = false;
		
		while(!p->request_terminate && !enter_done) {
			if(cmdfile != NULL && cmdfile->IsOpened()) {
				if(cmdfile->Fgetts(command, array_length(command)) != NULL) {
					while(_tcslen(command) > 0 && (command[_tcslen(command) - 1] == 0x0d || command[_tcslen(command) - 1] == 0x0a)) {
						command[_tcslen(command) - 1] = _T('\0');
					}
					if(_tcslen(command) > 0) {
						my_printf(p->osd, _T("%s\n"), command);
						enter_done = true;
					}
				} else {
					cmdfile->Fclose();
					delete cmdfile;
					cmdfile = NULL;
				}
			} else {
				memset(ir, 0, sizeof(ir));
				int count = p->osd->read_console_input(ir, 16);
				
				for(int i = 0; i < count; i++) {
					if(ir[i] == 0x08) {
						if(enter_ptr > 0) {
							enter_ptr--;
							my_putch(p->osd, 0x08);
							my_putch(p->osd, _T(' '));
							my_putch(p->osd, 0x08);
						}
					} else if(ir[i] == 0x0d || ir[i] == 0x0a) {
						if(enter_ptr == 0 && prev_command[0] != _T('\0')) {
							memcpy(command, prev_command, sizeof(command));
							my_printf(p->osd, _T("%s\n"), command);
							enter_done = true;
							break;
						} else if(enter_ptr > 0) {
							command[enter_ptr] = _T('\0');
							memcpy(prev_command, command, sizeof(command));
							if(_tcsicmp(cpu_debugger->history[cpu_debugger->history_ptr], command) != 0) {
								memcpy(cpu_debugger->history[cpu_debugger->history_ptr], command, sizeof(command));
								if(++cpu_debugger->history_ptr >= MAX_COMMAND_HISTORY) {
									cpu_debugger->history_ptr = 0;
								}
							}
							my_printf(p->osd, _T("\n"));
							enter_done = true;
							break;
						}
					} else if(ir[i] == 0x1b && ir[i + 1] == 0x5b) {
						if(ir[i + 2] == _T('A') || ir[i + 2] == _T('B')) {
							int history_ptr_stored = history_ptr;
							if(ir[i + 2] == _T('A')) {
								if(++history_ptr >= MAX_COMMAND_HISTORY) {
									history_ptr = MAX_COMMAND_HISTORY;
								}
							} else {
								if(--history_ptr < 0) {
									history_ptr = 0;
								}
							}
							int index = cpu_debugger->history_ptr - history_ptr;
							while(index < 0) {
								index += MAX_COMMAND_HISTORY;
							}
							while(index >= MAX_COMMAND_HISTORY) {
								index -= MAX_COMMAND_HISTORY;
							}
							if(cpu_debugger->history[index][0] != _T('\0')) {
								for(int i = 0; i < enter_ptr; i++) {
									my_putch(p->osd, 0x08);
									my_putch(p->osd, _T(' '));
									my_putch(p->osd, 0x08);
								}
								memcpy(command, cpu_debugger->history[index], sizeof(command));
								my_printf(p->osd, _T("%s"), command);
								enter_ptr = _tcslen(command);
							} else {
								history_ptr = history_ptr_stored;
							}
						}
						i += 2; // skip 2 characters
					} else if(ir[i] >= 0x20 && ir[i] <= 0x7e && enter_ptr < MAX_COMMAND_LENGTH && !(ir[i] == 0x20 && enter_ptr == 0)) {
						command[enter_ptr++] = ir[i];
						my_putch(p->osd, ir[i]);
					}
				}
				p->osd->sleep(10);
			}
		}
		
		// process command
		if(!p->request_terminate && enter_done) {
			_TCHAR *params[32], *token = NULL, *context = NULL;
			int num = 0;
			
			if((token = my_tcstok_s(command, _T(" "), &context)) != NULL) {
				params[num++] = token;
				while(num < 32 && (token = my_tcstok_s(NULL, _T(" "), &context)) != NULL) {
					params[num++] = token;
				}
			}
			if(_tcsicmp(params[0], _T("D")) == 0) {
				if(num <= 3) {
					uint32_t start_addr = dump_addr;
					if(num >= 2) {
						start_addr = my_hexatoi(target, params[1]);
					}
					start_addr %= target->get_debug_data_addr_space();
					
					uint32_t end_addr = start_addr + 8 * 16 - 1;
					if(num == 3) {
						end_addr = my_hexatoi(target, params[2]);
					}
					end_addr %= target->get_debug_data_addr_space();
					
					if(start_addr > end_addr) {
						end_addr = (uint32_t)(target->get_debug_data_addr_space() - 1);
					}
					for(uint64_t addr = start_addr & ~0x0f; addr <= end_addr; addr++) {
						if(addr > target->get_debug_data_addr_space() - 1) {
							end_addr = (uint32_t)(target->get_debug_data_addr_space() - 1);
							break;
						}
						if((addr & 0x0f) == 0) {
							my_printf(p->osd, _T("%08X "), addr % target->get_debug_data_addr_space());
							memset(buffer, 0, sizeof(buffer));
						}
						if(addr < start_addr) {
							my_printf(p->osd, _T("   "));
							buffer[addr & 0x0f] = _T(' ');
						} else {
							uint32_t data = target->read_debug_data8((uint32_t)(addr % target->get_debug_data_addr_space()));
							my_printf(p->osd, _T(" %02X"), data);
							buffer[addr & 0x0f] = ((data >= 0x20 && data <= 0x7e) || (cp932 && data >= 0xa1 && data <= 0xdf)) ? data : _T('.');
						}
						if((addr & 0x0f) == 0x0f) {
							my_printf(p->osd, _T("  %s\n"), buffer);
						}
					}
					if((end_addr & 0x0f) != 0x0f) {
						for(uint32_t addr = (end_addr & 0x0f) + 1; addr <= 0x0f; addr++) {
							my_printf(p->osd, _T("   "));
						}
						my_printf(p->osd, _T("  %s\n"), buffer);
					}
					dump_addr = (end_addr + 1) % target->get_debug_data_addr_space();
					prev_command[1] = _T('\0'); // remove parameters to dump continuously
				} else {
					my_printf(p->osd, _T("invalid parameter number\n"));
				}
			} else if(_tcsicmp(params[0], _T("E")) == 0 || _tcsicmp(params[0], _T("EB")) == 0) {
				if(num >= 3) {
					uint32_t addr = my_hexatoi(target, params[1]) % target->get_debug_data_addr_space();
					for(int i = 2; i < num; i++) {
						target->write_debug_data8(addr, my_hexatoi(target, params[i]) & 0xff);
						addr = (addr + 1) % target->get_debug_data_addr_space();
					}
				} else {
					my_printf(p->osd, _T("invalid parameter number\n"));
				}
			} else if(_tcsicmp(params[0], _T("EW")) == 0) {
				if(num >= 3) {
					uint32_t addr = my_hexatoi(target, params[1]) % target->get_debug_data_addr_space();
					for(int i = 2; i < num; i++) {
						target->write_debug_data16(addr, my_hexatoi(target, params[i]) & 0xffff);
						addr = (addr + 2) % target->get_debug_data_addr_space();
					}
				} else {
					my_printf(p->osd, _T("invalid parameter number\n"));
				}
			} else if(_tcsicmp(params[0], _T("ED")) == 0) {
				if(num >= 3) {
					uint32_t addr = my_hexatoi(target, params[1]) % target->get_debug_data_addr_space();
					for(int i = 2; i < num; i++) {
						target->write_debug_data32(addr, my_hexatoi(target, params[i]));
						addr = (addr + 4) % target->get_debug_data_addr_space();
					}
				} else {
					my_printf(p->osd, _T("invalid parameter number\n"));
				}
			} else if(_tcsicmp(params[0], _T("EA")) == 0) {
				if(num >= 3) {
					uint32_t addr = my_hexatoi(target, params[1]) % target->get_debug_data_addr_space();
					my_tcscpy_s(buffer, array_length(buffer), prev_command);
					if((token = my_tcstok_s(buffer, _T("\""), &context)) != NULL && (token = my_tcstok_s(NULL, _T("\""), &context)) != NULL) {
						int len = _tcslen(token);
						for(int i = 0; i < len; i++) {
							target->write_debug_data8(addr, token[i] & 0xff);
							addr = (addr + 1) % target->get_debug_data_addr_space();
						}
					} else {
						my_printf(p->osd, _T("invalid parameter\n"));
					}
				} else {
					my_printf(p->osd, _T("invalid parameter number\n"));
				}
			} else if(_tcsicmp(params[0], _T("I")) == 0 || _tcsicmp(params[0], _T("IB")) == 0) {
				if(num == 2) {
					my_printf(p->osd, _T("%02X\n"), target->read_debug_io8(my_hexatoi(target, params[1])) & 0xff);
				} else {
					my_printf(p->osd, _T("invalid parameter number\n"));
				}
			} else if(_tcsicmp(params[0], _T("IW")) == 0) {
				if(num == 2) {
					my_printf(p->osd, _T("%02X\n"), target->read_debug_io16(my_hexatoi(target, params[1])) & 0xffff);
				} else {
					my_printf(p->osd, _T("invalid parameter number\n"));
				}
			} else if(_tcsicmp(params[0], _T("ID")) == 0) {
				if(num == 2) {
					my_printf(p->osd, _T("%02X\n"), target->read_debug_io32(my_hexatoi(target, params[1])));
				} else {
					my_printf(p->osd, _T("invalid parameter number\n"));
				}
			} else if(_tcsicmp(params[0], _T("O")) == 0 || _tcsicmp(params[0], _T("OB")) == 0) {
				if(num == 3) {
					target->write_debug_io8(my_hexatoi(target, params[1]), my_hexatoi(target, params[2]) & 0xff);
				} else {
					my_printf(p->osd, _T("invalid parameter number\n"));
				}
			} else if(_tcsicmp(params[0], _T("OW")) == 0) {
				if(num == 3) {
					target->write_debug_io16(my_hexatoi(target, params[1]), my_hexatoi(target, params[2]) & 0xffff);
				} else {
					my_printf(p->osd, _T("invalid parameter number\n"));
				}
			} else if(_tcsicmp(params[0], _T("OD")) == 0) {
				if(num == 3) {
					target->write_debug_io32(my_hexatoi(target, params[1]), my_hexatoi(target, params[2]));
				} else {
					my_printf(p->osd, _T("invalid parameter number\n"));
				}
			} else if(_tcsicmp(params[0], _T("R")) == 0) {
				if(num == 1) {
					if(target->get_debug_regs_info(buffer, array_length(buffer))) {
						my_printf(p->osd, _T("%s\n"), buffer);
					}
				} else if(num == 3) {
					if(!target->write_debug_reg(params[1], my_hexatoi(target, params[2]))) {
						my_printf(p->osd, _T("unknown register %s\n"), params[1]);
					}
				} else {
					my_printf(p->osd, _T("invalid parameter number\n"));
				}
			} else if(_tcsicmp(params[0], _T("S")) == 0) {
				if(num >= 4) {
					uint32_t start_addr = my_hexatoi(target, params[1]) % target->get_debug_data_addr_space();
					uint32_t end_addr = my_hexatoi(target, params[2]) % target->get_debug_data_addr_space();
					uint8_t list[32];
					for(int i = 3, j = 0; i < num; i++, j++) {
						list[j] = my_hexatoi(target, params[i]);
					}
					for(uint64_t addr = start_addr; addr <= end_addr; addr++) {
						bool found = true;
						for(int i = 3, j = 0; i < num; i++, j++) {
							if(target->read_debug_data8((uint32_t)((addr + j) % target->get_debug_data_addr_space())) != list[j]) {
								found = false;
								break;
							}
						}
						if(found) {
							my_printf(p->osd, _T("%s\n"), my_get_value_and_symbol(target, _T("%08X"), (uint32_t)addr));
						}
					}
				} else {
					my_printf(p->osd, _T("invalid parameter number\n"));
				}
			} else if(_tcsicmp(params[0], _T("U")) == 0) {
				if(num <= 3) {
					if(num >= 2) {
						dasm_addr = my_hexatoi(target, params[1]) & target->get_debug_prog_addr_mask();
					}
					if(num == 3) {
						uint32_t end_addr = my_hexatoi(target, params[2]) & target->get_debug_prog_addr_mask();
						while(dasm_addr <= end_addr) {
							const _TCHAR *name = my_get_symbol(target, dasm_addr & target->get_debug_prog_addr_mask());
							int len = target->debug_dasm(dasm_addr & target->get_debug_prog_addr_mask(), buffer, array_length(buffer));
							if(name != NULL) {
								my_printf(p->osd, _T("%08X                  "), dasm_addr & target->get_debug_prog_addr_mask());
								p->osd->set_console_text_attribute(OSD_CONSOLE_GREEN | OSD_CONSOLE_INTENSITY);
								my_printf(p->osd, _T("%s:\n"), name);
								p->osd->set_console_text_attribute(OSD_CONSOLE_RED | OSD_CONSOLE_GREEN | OSD_CONSOLE_BLUE | OSD_CONSOLE_INTENSITY);
							}
							my_printf(p->osd, _T("%08X  "), dasm_addr & target->get_debug_prog_addr_mask());
							for(int i = 0; i < len; i++) {
								my_printf(p->osd, _T("%02X"), target->read_debug_data8((dasm_addr + i) & target->get_debug_prog_addr_mask()));
							}
							for(int i = len; i < 8; i++) {
								my_printf(p->osd, _T("  "));
							}
							my_printf(p->osd, _T("  %s\n"), buffer);
							dasm_addr += len;
						}
					} else {
						for(int i = 0; i < 16; i++) {
							const _TCHAR *name = my_get_symbol(target, dasm_addr & target->get_debug_prog_addr_mask());
							int len = target->debug_dasm(dasm_addr & target->get_debug_prog_addr_mask(), buffer, array_length(buffer));
							if(name != NULL) {
								my_printf(p->osd, _T("%08X                  "), dasm_addr & target->get_debug_prog_addr_mask());
								p->osd->set_console_text_attribute(OSD_CONSOLE_GREEN | OSD_CONSOLE_INTENSITY);
								my_printf(p->osd, _T("%s:\n"), name);
								p->osd->set_console_text_attribute(OSD_CONSOLE_RED | OSD_CONSOLE_GREEN | OSD_CONSOLE_BLUE | OSD_CONSOLE_INTENSITY);
							}
							my_printf(p->osd, _T("%08X  "), dasm_addr & target->get_debug_prog_addr_mask());
							for(int i = 0; i < len; i++) {
								my_printf(p->osd, _T("%02X"), target->read_debug_data8((dasm_addr + i) & target->get_debug_prog_addr_mask()));
							}
							for(int i = len; i < 8; i++) {
								my_printf(p->osd, _T("  "));
							}
							my_printf(p->osd, _T("  %s\n"), buffer);
							dasm_addr += len;
						}
					}
					prev_command[1] = _T('\0'); // remove parameters to disassemble continuously
				} else {
					my_printf(p->osd, _T("invalid parameter number\n"));
				}
			} else if(_tcsicmp(params[0], _T("UT")) == 0) {
				if(target_debugger == NULL) {
					my_printf(p->osd, _T("debugger is not attached to target device %s\n"), target->this_device_name);
				} else if(num <= 3) {
					int steps = 128;
					if(num >= 2) {
						steps = min((int)my_hexatoi(target, params[1]), MAX_CPU_TRACE);
					}
					for(int i = MAX_CPU_TRACE - steps; i < MAX_CPU_TRACE; i++) {
						int index = (target_debugger->cpu_trace_ptr + i) & (MAX_CPU_TRACE - 1);
						if(!(target_debugger->cpu_trace[index] & ~target->get_debug_prog_addr_mask())) {
							const _TCHAR *name = my_get_symbol(target, target_debugger->cpu_trace[index] & target->get_debug_prog_addr_mask());
							int len = target->debug_dasm(target_debugger->cpu_trace[index] & target->get_debug_prog_addr_mask(), buffer, array_length(buffer));
							if(name != NULL) {
								my_printf(p->osd, _T("%08X                  "), target_debugger->cpu_trace[index] & target->get_debug_prog_addr_mask());
								p->osd->set_console_text_attribute(OSD_CONSOLE_GREEN | OSD_CONSOLE_INTENSITY);
								my_printf(p->osd, _T("%s:\n"), name);
								p->osd->set_console_text_attribute(OSD_CONSOLE_RED | OSD_CONSOLE_GREEN | OSD_CONSOLE_BLUE | OSD_CONSOLE_INTENSITY);
							}
							my_printf(p->osd, _T("%08X  "), target_debugger->cpu_trace[index] & target->get_debug_prog_addr_mask());
							for(int i = 0; i < len; i++) {
								my_printf(p->osd, _T("%02X"), target->read_debug_data8((target_debugger->cpu_trace[index] + i) & target->get_debug_prog_addr_mask()));
							}
							for(int i = len; i < 8; i++) {
								my_printf(p->osd, _T("  "));
							}
							my_printf(p->osd, _T("  %s\n"), buffer);
						}
					}
				} else {
					my_printf(p->osd, _T("invalid parameter number\n"));
				}
			} else if(_tcsicmp(params[0], _T("H")) == 0) {
				if(num == 3) {
					uint32_t l = my_hexatoi(target, params[1]);
					uint32_t r = my_hexatoi(target, params[2]);
					my_printf(p->osd, _T("%08X  %08X\n"), l + r, l - r);
				} else {
					my_printf(p->osd, _T("invalid parameter number\n"));
				}
			} else if(_tcsicmp(params[0], _T("N")) == 0) {
				if(num >= 2 && params[1][0] == _T('\"')) {
					my_tcscpy_s(buffer, array_length(buffer), prev_command);
					if((token = my_tcstok_s(buffer, _T("\""), &context)) != NULL && (token = my_tcstok_s(NULL, _T("\""), &context)) != NULL) {
						my_tcscpy_s(cpu_debugger->file_path, _MAX_PATH, my_absolute_path(token));
					} else {
						my_printf(p->osd, _T("invalid parameter\n"));
					}
				} else if(num == 2) {
					my_tcscpy_s(cpu_debugger->file_path, _MAX_PATH, my_absolute_path(params[1]));
				} else {
					my_printf(p->osd, _T("invalid parameter number\n"));
				}
			} else if(_tcsicmp(params[0], _T("L")) == 0) {
				if(target_debugger == NULL) {
					my_printf(p->osd, _T("debugger is not attached to target device %s\n"), target->this_device_name);
				} else {
					FILEIO* fio = new FILEIO();
					if(check_file_extension(cpu_debugger->file_path, _T(".sym"))) {
						if(fio->Fopen(cpu_debugger->file_path, FILEIO_READ_ASCII)) {
							target_debugger->release_symbols();
							_TCHAR line[1024];
							while(fio->Fgetts(line, array_length(line)) != NULL) {
								_TCHAR *next = NULL;
								_TCHAR *addr = my_tcstok_s(line, _T("\t #$*,;"), &next);
								while(addr != NULL) {
									if(_tcslen(addr) > 0) {
										_TCHAR *name = my_tcstok_s(NULL, _T("\t #$*,;"), &next);
										while(name != NULL) {
											while(_tcslen(name) > 0 && (name[_tcslen(name) - 1] == 0x0d || name[_tcslen(name) - 1] == 0x0a)) {
												name[_tcslen(name) - 1] = _T('\0');
											}
											if(_tcslen(name) > 0) {
												target_debugger->add_symbol(my_hexatoi(NULL, addr), name);
												break;
											}
											name = my_tcstok_s(NULL, _T("\t #$*,;"), &next);
										}
									}
									addr = my_tcstok_s(NULL, _T("\t #$*,;"), &next);
								}
							}
							fio->Fclose();
						} else {
							my_printf(p->osd, _T("can't open %s\n"), cpu_debugger->file_path);
						}
					} else if(check_file_extension(cpu_debugger->file_path, _T(".hex"))) {
						if(fio->Fopen(cpu_debugger->file_path, FILEIO_READ_ASCII)) {
							uint32_t start_addr = 0, linear = 0, segment = 0;
							if(num >= 2) {
								start_addr = my_hexatoi(target, params[1]);
							}
							char line[1024];
							while(fio->Fgets(line, sizeof(line)) != NULL) {
								if(line[0] != ':') continue;
								int type = my_hexatob(line + 7);
								if(type == 0x00) {
									uint32_t bytes = my_hexatob(line + 1);
									uint32_t addr = my_hexatow(line + 3) + start_addr + linear + segment;
									for(uint32_t i = 0; i < bytes; i++) {
										target->write_debug_data8((addr + i) % target->get_debug_data_addr_space(), my_hexatob(line + 9 + 2 * i));
									}
								} else if(type == 0x01) {
									break;
								} else if(type == 0x02) {
									segment = my_hexatow(line + 9) << 4;
									start_addr = 0;
								} else if(type == 0x04) {
									linear = my_hexatow(line + 9) << 16;
									start_addr = 0;
								}
							}
							fio->Fclose();
						} else {
							my_printf(p->osd, _T("can't open %s\n"), cpu_debugger->file_path);
						}
					} else {
						if(fio->Fopen(cpu_debugger->file_path, FILEIO_READ_BINARY)) {
							uint32_t start_addr = 0x100, end_addr = (uint32_t)(target->get_debug_data_addr_space() - 1);
							if(num >= 2) {
								start_addr = my_hexatoi(target, params[1]) % target->get_debug_data_addr_space();
							}
							if(num >= 3) {
								end_addr = my_hexatoi(target, params[2]) % target->get_debug_data_addr_space();
							}
							for(uint32_t addr = start_addr; addr <= end_addr; addr++) {
								int data = fio->Fgetc();
								if(data == EOF) {
									break;
								}
								target->write_debug_data8(addr % target->get_debug_data_addr_space(), data);
							}
							fio->Fclose();
						} else {
							my_printf(p->osd, _T("can't open %s\n"), cpu_debugger->file_path);
						}
					}
					delete fio;
				}
			} else if(_tcsicmp(params[0], _T("W")) == 0) {
				if(target_debugger == NULL) {
					my_printf(p->osd, _T("debugger is not attached to target device %s\n"), target->this_device_name);
				} else if(num == 3) {
					uint32_t start_addr = my_hexatoi(target, params[1]) % target->get_debug_data_addr_space(), end_addr = my_hexatoi(target, params[2]) % target->get_debug_data_addr_space();
					FILEIO* fio = new FILEIO();
					if(check_file_extension(cpu_debugger->file_path, _T(".hex"))) {
						// write intel hex format file
						if(fio->Fopen(cpu_debugger->file_path, FILEIO_WRITE_ASCII)) {
							uint32_t addr = start_addr;
							while(addr <= end_addr) {
								uint32_t len = min(end_addr - addr + 1, (uint32_t)16);
								uint32_t sum = len + ((addr >> 8) & 0xff) + (addr & 0xff) + 0x00;
								fio->Fprintf(":%02X%04X%02X", len, addr & 0xffff, 0x00);
								for(uint32_t i = 0; i < len; i++) {
									uint8_t data = target->read_debug_data8((addr++) % target->get_debug_data_addr_space());
									sum += data;
									fio->Fprintf("%02X", data);
								}
								fio->Fprintf("%02X\n", (0x100 - (sum & 0xff)) & 0xff);
							}
							fio->Fprintf(":00000001FF\n");
							fio->Fclose();
						} else {
							my_printf(p->osd, _T("can't open %s\n"), cpu_debugger->file_path);
						}
					} else {
						if(fio->Fopen(cpu_debugger->file_path, FILEIO_WRITE_BINARY)) {
							for(uint32_t addr = start_addr; addr <= end_addr; addr++) {
								fio->Fputc(target->read_debug_data8(addr % target->get_debug_data_addr_space()));
							}
							fio->Fclose();
						} else {
							my_printf(p->osd, _T("can't open %s\n"), cpu_debugger->file_path);
						}
					}
					delete fio;
				} else {
					my_printf(p->osd, _T("invalid parameter number\n"));
				}
			} else if(_tcsicmp(params[0], _T("SC")) == 0) {
				if(target_debugger == NULL) {
					my_printf(p->osd, _T("debugger is not attached to target device %s\n"), target->this_device_name);
				} else if(num == 1) {
					target_debugger->release_symbols();
				} else {
					my_printf(p->osd, _T("invalid parameter number\n"));
				}
			} else if(_tcsicmp(params[0], _T("SL")) == 0) {
				if(target_debugger == NULL) {
					my_printf(p->osd, _T("debugger is not attached to target device %s\n"), target->this_device_name);
				} else if(num == 1) {
					for(symbol_t* symbol = target_debugger->first_symbol; symbol; symbol = symbol->next_symbol) {
						my_printf(p->osd, _T("%08X %s\n"), symbol->addr, symbol->name);
					}
				} else {
					my_printf(p->osd, _T("invalid parameter number\n"));
				}
			} else if(_tcsicmp(params[0], _T( "BP")) == 0 ||
			          _tcsicmp(params[0], _T( "CP")) == 0) {
				if(target_debugger == NULL) {
					my_printf(p->osd, _T("debugger is not attached to target device %s\n"), target->this_device_name);
				} else {
					break_point_t *bp = get_break_point(target_debugger, params[0]);
					if(num == 2) {
						uint32_t addr = my_hexatoi(target, params[1]);
						bool found = false;
						for(int i = 0; i < MAX_BREAK_POINTS && !found; i++) {
							if(bp->table[i].status == 0 || (bp->table[i].addr == addr && bp->table[i].mask == target->get_debug_prog_addr_mask())) {
								bp->table[i].addr = addr;
								bp->table[i].mask = target->get_debug_prog_addr_mask();
								bp->table[i].status = 1;
								bp->table[i].check_point = (params[0][0] == 'C' || params[0][0] == 'c' || params[0][1] == 'C' || params[0][1] == 'c');
								found = true;
							}
						}
						if(!found) {
							my_printf(p->osd, _T("too many break points\n"));
						}
					} else {
						my_printf(p->osd, _T("invalid parameter number\n"));
					}
				}
			} else if(_tcsicmp(params[0], _T("RBP")) == 0 || _tcsicmp(params[0], _T("WBP")) == 0 ||
			          _tcsicmp(params[0], _T("RCP")) == 0 || _tcsicmp(params[0], _T("WCP")) == 0) {
				if(target_debugger == NULL) {
					my_printf(p->osd, _T("debugger is not attached to target device %s\n"), target->this_device_name);
				} else {
					break_point_t *bp = get_break_point(target_debugger, params[0]);
					if(num == 2) {
						uint32_t addr = my_hexatoi(target, params[1]);
						bool found = false;
						for(int i = 0; i < MAX_BREAK_POINTS && !found; i++) {
							if(bp->table[i].status == 0 || (bp->table[i].addr == addr && bp->table[i].mask == target->get_debug_data_addr_mask())) {
								bp->table[i].addr = addr;
								bp->table[i].mask = target->get_debug_data_addr_mask();
								bp->table[i].status = 1;
								bp->table[i].check_point = (params[0][0] == 'C' || params[0][0] == 'c' || params[0][1] == 'C' || params[0][1] == 'c');
								found = true;
							}
						}
						if(!found) {
							my_printf(p->osd, _T("too many break points\n"));
						}
					} else {
						my_printf(p->osd, _T("invalid parameter number\n"));
					}
				}
			} else if(_tcsicmp(params[0], _T("IBP")) == 0 || _tcsicmp(params[0], _T("OBP")) == 0 ||
			          _tcsicmp(params[0], _T("ICP")) == 0 || _tcsicmp(params[0], _T("OCP")) == 0) {
				if(target_debugger == NULL) {
					my_printf(p->osd, _T("debugger is not attached to target device %s\n"), target->this_device_name);
				} else {
					break_point_t *bp = get_break_point(target_debugger, params[0]);
					if(num == 2 || num == 3) {
						uint32_t addr = my_hexatoi(target, params[1]), mask = 0xff;
						if(num == 3) {
							mask = my_hexatoi(target, params[2]);
						}
						bool found = false;
						for(int i = 0; i < MAX_BREAK_POINTS && !found; i++) {
							if(bp->table[i].status == 0 || (bp->table[i].addr == addr && bp->table[i].mask == mask)) {
								bp->table[i].addr = addr;
								bp->table[i].mask = mask;
								bp->table[i].status = 1;
								bp->table[i].check_point = (params[0][1] == 'C' || params[0][1] == 'c');
								found = true;
							}
						}
						if(!found) {
							my_printf(p->osd, _T("too many break points\n"));
						}
					} else {
						my_printf(p->osd, _T("invalid parameter number\n"));
					}
				}
			} else if(_tcsicmp(params[0], _T("BC")) == 0 || _tcsicmp(params[0], _T("RBC")) == 0 || _tcsicmp(params[0], _T("WBC")) == 0 || _tcsicmp(params[0], _T("IBC")) == 0 || _tcsicmp(params[0], _T("OBC")) == 0) {
				if(target_debugger == NULL) {
					my_printf(p->osd, _T("debugger is not attached to target device %s\n"), target->this_device_name);
				} else {
					break_point_t *bp = get_break_point(target_debugger, params[0]);
					if(num == 2 && (_tcsicmp(params[1], _T("*")) == 0 || _tcsicmp(params[1], _T("ALL")) == 0)) {
						memset(bp->table, 0, sizeof(bp->table));
					} else if(num >= 2) {
						for(int i = 1; i < num; i++) {
							int index = my_hexatoi(target, params[i]);
							if(!(index >= 0 && index < MAX_BREAK_POINTS)) {
								my_printf(p->osd, _T("invalid index %x\n"), index);
							} else {
								bp->table[index].addr = bp->table[index].mask = 0;
								bp->table[index].status = 0;
							}
						}
					} else {
						my_printf(p->osd, _T("invalid parameter number\n"));
					}
				}
			} else if(_tcsicmp(params[0], _T("BD")) == 0 || _tcsicmp(params[0], _T("RBD")) == 0 || _tcsicmp(params[0], _T("WBD")) == 0 || _tcsicmp(params[0], _T("IBD")) == 0 || _tcsicmp(params[0], _T("OBD")) == 0 ||
			          _tcsicmp(params[0], _T("BE")) == 0 || _tcsicmp(params[0], _T("RBE")) == 0 || _tcsicmp(params[0], _T("WBE")) == 0 || _tcsicmp(params[0], _T("IBE")) == 0 || _tcsicmp(params[0], _T("OBE")) == 0) {
				if(target_debugger == NULL) {
					my_printf(p->osd, _T("debugger is not attached to target device %s\n"), target->this_device_name);
				} else {
					break_point_t *bp = get_break_point(target_debugger, params[0]);
					bool enabled = (params[0][1] == _T('E') || params[0][1] == _T('e') || params[0][2] == _T('E') || params[0][2] == _T('e'));
					if(num == 2 && (_tcsicmp(params[1], _T("*")) == 0 || _tcsicmp(params[1], _T("ALL")) == 0)) {
						for(int i = 0; i < MAX_BREAK_POINTS; i++) {
							if(bp->table[i].status != 0) {
								bp->table[i].status = enabled ? 1 : -1;
							}
						}
					} else if(num >= 2) {
						for(int i = 1; i < num; i++) {
							int index = my_hexatoi(target, params[i]);
							if(!(index >= 0 && index < MAX_BREAK_POINTS)) {
								my_printf(p->osd, _T("invalid index %x\n"), index);
							} else if(bp->table[index].status == 0) {
								my_printf(p->osd, _T("break point %x is null\n"), index);
							} else {
								bp->table[index].status = enabled ? 1 : -1;
							}
						}
					} else {
						my_printf(p->osd, _T("invalid parameter number\n"));
					}
				}
			} else if(_tcsicmp(params[0], _T("BL")) == 0 || _tcsicmp(params[0], _T("RBL")) == 0 || _tcsicmp(params[0], _T("WBL")) == 0) {
				if(target_debugger == NULL) {
					my_printf(p->osd, _T("debugger is not attached to target device %s\n"), target->this_device_name);
				} else {
					if(num == 1) {
						break_point_t *bp = get_break_point(target_debugger, params[0]);
						for(int i = 0; i < MAX_BREAK_POINTS; i++) {
							if(bp->table[i].status) {
								my_printf(p->osd, _T("%x %c %s %s\n"), i,
									bp->table[i].status == 1 ? _T('e') : _T('d'),
									my_get_value_and_symbol(target, _T("%08X"), bp->table[i].addr),
									bp->table[i].check_point ? "checkpoint" : "");
							}
						}
					} else {
						my_printf(p->osd, _T("invalid parameter number\n"));
					}
				}
			} else if(_tcsicmp(params[0], _T("IBL")) == 0 || _tcsicmp(params[0], _T("OBL")) == 0) {
				if(target_debugger == NULL) {
					my_printf(p->osd, _T("debugger is not attached to target device %s\n"), target->this_device_name);
				} else {
					if(num == 1) {
						break_point_t *bp = get_break_point(target_debugger, params[0]);
						for(int i = 0; i < MAX_BREAK_POINTS; i++) {
							if(bp->table[i].status) {
								my_printf(p->osd, _T("%x %c %s %08X %s\n"), i,
									bp->table[i].status == 1 ? _T('e') : _T('d'),
									my_get_value_and_symbol(target, _T("%08X"), bp->table[i].addr),
									bp->table[i].mask,
									bp->table[i].check_point ? "checkpoint" : "");
							}
						}
					} else {
						my_printf(p->osd, _T("invalid parameter number\n"));
					}
				}
			} else if(_tcsicmp(params[0], _T("G")) == 0 || _tcsicmp(params[0], _T("P")) == 0) {
				if(num == 1 || num == 2) {
					bool break_points_stored = false;
					if(_tcsicmp(params[0], _T("P")) == 0) {
						cpu_debugger->store_break_points();
						cpu_debugger->bp.table[0].addr = (cpu->get_next_pc() + cpu->debug_dasm(cpu->get_next_pc(), buffer, array_length(buffer))) & cpu->get_debug_prog_addr_mask();
						cpu_debugger->bp.table[0].mask = cpu->get_debug_prog_addr_mask();
						cpu_debugger->bp.table[0].status = 1;
						cpu_debugger->bp.table[0].check_point = false;
						break_points_stored = true;
					} else if(num >= 2) {
						cpu_debugger->store_break_points();
						cpu_debugger->bp.table[0].addr = my_hexatoi(cpu, params[1]) & cpu->get_debug_prog_addr_mask();
						cpu_debugger->bp.table[0].mask = cpu->get_debug_prog_addr_mask();
						cpu_debugger->bp.table[0].status = 1;
						cpu_debugger->bp.table[0].check_point = false;
						break_points_stored = true;
					}
RESTART_GO:
					cpu_debugger->now_going = true;
					cpu_debugger->now_suspended = false;
#if defined(_MSC_VER)
					while(!p->request_terminate && !cpu_debugger->now_suspended) {
						if(p->osd->is_console_key_pressed(VK_ESCAPE) && p->osd->is_console_active()) {
							break;
						}
						p->osd->sleep(10);
					}
#elif defined(OSD_QT)
					while(!p->request_terminate && !cpu_debugger->now_suspended) {
						if(p->osd->console_input_string() != NULL && p->osd->is_console_active()) {
							p->osd->clear_console_input_string();
							break;
						}
						p->osd->sleep(10);
					}
#endif
					// break cpu
					cpu_debugger->now_going = false;
					wait_count = 0;
					while(!p->request_terminate && !(cpu_debugger->now_suspended && cpu_debugger->now_waiting)) {
						if((wait_count++) == 100) {
							p->osd->set_console_text_attribute(OSD_CONSOLE_RED | OSD_CONSOLE_INTENSITY);
							my_printf(p->osd, _T("waiting until cpu is suspended...\n"));
						}
						p->osd->sleep(10);
					}
					if(target == cpu) {
						dasm_addr = cpu->get_next_pc();
					}
					
					p->osd->set_console_text_attribute(OSD_CONSOLE_GREEN | OSD_CONSOLE_BLUE | OSD_CONSOLE_INTENSITY);
					cpu->debug_dasm(cpu->get_pc(), buffer, array_length(buffer));
					my_printf(p->osd, _T("done\t%s  %s\n"), my_get_value_and_symbol(cpu, _T("%08X"), cpu->get_pc()), buffer);
					
					p->osd->set_console_text_attribute(OSD_CONSOLE_RED | OSD_CONSOLE_GREEN | OSD_CONSOLE_BLUE | OSD_CONSOLE_INTENSITY);
					if(cpu->get_debug_regs_info(buffer, array_length(buffer))) {
						my_printf(p->osd, _T("%s\n"), buffer);
					}
					
					if(target != cpu) {
						p->osd->set_console_text_attribute(OSD_CONSOLE_GREEN | OSD_CONSOLE_INTENSITY);
						if(target->debug_dasm(target->get_next_pc(), buffer, array_length(buffer)) != 0) {
							my_printf(p->osd, _T("next\t%s  %s\n"), my_get_value_and_symbol(target, _T("%08X"), target->get_next_pc()), buffer);
						}
						if(target->get_debug_regs_info(buffer, array_length(buffer))) {
							my_printf(p->osd, _T("%s\n"), buffer);
						}
					}
					
					if(cpu_debugger->hit()) {
						show_break_reason(p->osd, cpu, target, (_tcsicmp(params[0], _T("P")) == 0));
						bool restart = cpu_debugger->restartable();
						cpu_debugger->clear_hit();
						if(restart) goto RESTART_GO;
					} else {
						p->osd->set_console_text_attribute(OSD_CONSOLE_RED | OSD_CONSOLE_INTENSITY);
						my_printf(p->osd, _T("breaked at %s: esc key was pressed\n"), my_get_value_and_symbol(cpu, _T("%08X"), cpu->get_next_pc()));
					}
					if(break_points_stored) {
						cpu_debugger->restore_break_points();
					}
					p->osd->set_console_text_attribute(OSD_CONSOLE_GREEN | OSD_CONSOLE_BLUE | OSD_CONSOLE_INTENSITY);
					cpu->debug_dasm(cpu->get_next_pc(), buffer, array_length(buffer));
					my_printf(p->osd, _T("next\t%s  %s\n"), my_get_value_and_symbol(cpu, _T("%08X"), cpu->get_next_pc()), buffer);
					p->osd->set_console_text_attribute(OSD_CONSOLE_RED | OSD_CONSOLE_GREEN | OSD_CONSOLE_BLUE | OSD_CONSOLE_INTENSITY);
				} else {
					my_printf(p->osd, _T("invalid parameter number\n"));
				}
			} else if(_tcsicmp(params[0], _T("T")) == 0) {
				if(num == 1 || num == 2) {
					int steps = 1;
					if(num >= 2) {
						steps = my_hexatoi(cpu, params[1]);
					}
					for(int i = 0; i < steps; i++) {
						cpu_debugger->now_going = false;
						cpu_debugger->now_suspended = false;
						wait_count = 0;
						while(!p->request_terminate && !(cpu_debugger->now_suspended && cpu_debugger->now_waiting)) {
							if((wait_count++) == 100) {
								p->osd->set_console_text_attribute(OSD_CONSOLE_RED | OSD_CONSOLE_INTENSITY);
								my_printf(p->osd, _T("waiting until cpu is suspended...\n"));
							}
							p->osd->sleep(10);
						}
						if(target == cpu) {
							dasm_addr = cpu->get_next_pc();
						}
						
						p->osd->set_console_text_attribute(OSD_CONSOLE_GREEN | OSD_CONSOLE_BLUE | OSD_CONSOLE_INTENSITY);
						cpu->debug_dasm(cpu->get_pc(), buffer, array_length(buffer));
						my_printf(p->osd, _T("done\t%s  %s\n"), my_get_value_and_symbol(cpu, _T("%08X"), cpu->get_pc()), buffer);
						
						p->osd->set_console_text_attribute(OSD_CONSOLE_RED | OSD_CONSOLE_GREEN | OSD_CONSOLE_BLUE | OSD_CONSOLE_INTENSITY);
						if(cpu->get_debug_regs_info(buffer, array_length(buffer))) {
							my_printf(p->osd, _T("%s\n"), buffer);
						}
						
						if(target != cpu) {
							p->osd->set_console_text_attribute(OSD_CONSOLE_GREEN | OSD_CONSOLE_INTENSITY);
							if(target->debug_dasm(target->get_next_pc(), buffer, array_length(buffer)) != 0) {
								my_printf(p->osd, _T("next\t%s  %s\n"), my_get_value_and_symbol(target, _T("%08X"), target->get_next_pc()), buffer);
							}
							if(target->get_debug_regs_info(buffer, array_length(buffer))) {
								my_printf(p->osd, _T("%s\n"), buffer);
							}
						}
						
						if(cpu_debugger->hit()) {
							show_break_reason(p->osd, cpu, target, false);
							bool restart = cpu_debugger->restartable();
							cpu_debugger->clear_hit();
							if(!restart) break;
						} else if(p->osd->is_console_key_pressed(VK_ESCAPE) && p->osd->is_console_active()) {
							break;
						}
					}
					p->osd->set_console_text_attribute(OSD_CONSOLE_GREEN | OSD_CONSOLE_BLUE | OSD_CONSOLE_INTENSITY);
					cpu->debug_dasm(cpu->get_next_pc(), buffer, array_length(buffer));
					my_printf(p->osd, _T("next\t%s  %s\n"), my_get_value_and_symbol(cpu, _T("%08X"), cpu->get_next_pc()), buffer);
					p->osd->set_console_text_attribute(OSD_CONSOLE_RED | OSD_CONSOLE_GREEN | OSD_CONSOLE_BLUE | OSD_CONSOLE_INTENSITY);
				} else {
					my_printf(p->osd, _T("invalid parameter number\n"));
				}
			} else if(_tcsicmp(params[0], _T("Q")) == 0) {
				p->osd->close_debugger_console();
				break;
			} else if(_tcsicmp(params[0], _T(">")) == 0) {
				if(num == 2) {
					if(logfile != NULL) {
						if(logfile->IsOpened()) {
							logfile->Fclose();
						}
						delete logfile;
						logfile = NULL;
					}
					logfile = new FILEIO();
					logfile->Fopen(my_absolute_path(params[1]), FILEIO_WRITE_ASCII);
				} else {
					my_printf(p->osd, _T("invalid parameter number\n"));
				}
			} else if(_tcsicmp(params[0], _T("<")) == 0) {
				if(num == 2) {
					if(cmdfile != NULL) {
						if(cmdfile->IsOpened()) {
							cmdfile->Fclose();
						}
					} else {
						cmdfile = new FILEIO();
					}
					if(!cmdfile->Fopen(my_absolute_path(params[1]), FILEIO_READ_ASCII)) {
						delete cmdfile;
						cmdfile = NULL;
						my_printf(p->osd, _T("can't open %s\n"), params[1]);
					}
				} else {
					my_printf(p->osd, _T("invalid parameter number\n"));
				}
			} else if(_tcsicmp(params[0], _T("!")) == 0) {
				if(num == 1) {
					my_printf(p->osd, _T("invalid parameter number\n"));
				} else if(_tcsicmp(params[1], _T("RESET")) == 0) {
					if(num == 2) {
						p->vm->reset();
					} else if(num == 3) {
						if(_tcsicmp(params[2], _T("ALL")) == 0) {
							p->vm->reset();
						} if(_tcsicmp(params[2], _T("CPU")) == 0) {
							cpu->reset();
						} if(_tcsicmp(params[2], _T("TARGET")) == 0) {
							target->reset();
						} else {
							my_printf(p->osd, _T("unknown device %s\n"), params[2]);
						}
					} else {
						my_printf(p->osd, _T("invalid parameter number\n"));
					}
				} else if(_tcsicmp(params[1], _T("KEY")) == 0) {
					if(num == 3 || num == 4) {
						int code =  my_hexatoi(target, params[2]) & 0xff, msec = 100;
						if(num == 4) {
							msec = my_hexatoi(target, params[3]);
						}
						int frames = (int)(p->vm->get_frame_rate() * (double)msec / 1000.0 + 0.5);
						p->osd->get_key_buffer()[code] &= 0x7f;
						p->osd->get_key_buffer()[code] |= max(1, min(127, frames));
						p->vm->key_down(code, false);
					} else {
						my_printf(p->osd, _T("invalid parameter number\n"));
					}
				} else if(_tcsicmp(params[1], _T("DEVICE")) == 0) {
					if(num == 2) {
						for(DEVICE* device = p->vm->first_device; device; device = device->next_device) {
							if(device->is_debugger_available()) {
								my_printf(p->osd, _T("ID=%02X  %s"), device->this_device_id, device->this_device_name);
								if(device == target) {
									p->osd->set_console_text_attribute(OSD_CONSOLE_RED | OSD_CONSOLE_INTENSITY);
									my_printf(p->osd, _T("  <=== target"));
									p->osd->set_console_text_attribute(OSD_CONSOLE_RED | OSD_CONSOLE_GREEN | OSD_CONSOLE_BLUE | OSD_CONSOLE_INTENSITY);
								}
								my_printf(p->osd, _T("\n"));
							}
						}
					} else if(num == 3) {
						DEVICE *device = NULL;
						if(_tcsicmp(params[2], _T("CPU")) == 0) {
							device = cpu;
						} else {
							device = p->vm->get_device(my_hexatoi(NULL, params[2]));
						}
						if(device != NULL && device->is_debugger_available()) {
							if(device != target) {
								if(target_debugger != NULL) {
									target_debugger->now_device_debugging = false;
								}
								cpu_debugger->set_context_child(NULL);
								target = device;
								target_debugger = (DEBUGGER *)target->get_debugger();
								if(target_debugger != NULL) {
									if(target != cpu) {
										target_debugger->now_device_debugging = true;
										cpu_debugger->set_context_child(target_debugger);
									} else {
										target_debugger->now_device_debugging = false;
									}
								}
								dump_addr = 0;
								dasm_addr = target->get_next_pc();
							}
						} else {
							my_printf(p->osd, _T("device not found\n"));
						}
					} else {
						my_printf(p->osd, _T("invalid parameter number\n"));
					}
				} else if(_tcsicmp(params[1], _T("CPU")) == 0) {
					if(num == 2) {
						for(DEVICE* device = p->vm->first_device; device; device = device->next_device) {
							if(device->is_cpu() && device->get_debugger() != NULL) {
								my_printf(p->osd, _T("ID=%02X  %s"), device->this_device_id, device->this_device_name);
								if(device == cpu) {
									p->osd->set_console_text_attribute(OSD_CONSOLE_RED | OSD_CONSOLE_INTENSITY);
									my_printf(p->osd, _T("  <=== target"));
									p->osd->set_console_text_attribute(OSD_CONSOLE_RED | OSD_CONSOLE_GREEN | OSD_CONSOLE_BLUE | OSD_CONSOLE_INTENSITY);
								}
								my_printf(p->osd, _T("\n"));
							}
						}
					} else if(num == 3) {
						DEVICE *device = p->vm->get_device(my_hexatoi(NULL, params[2]));
						if(device != NULL && device->is_cpu() && device->get_debugger() != NULL) {
							if(device != cpu) {
								DEBUGGER *prev_debugger = cpu_debugger;
								cpu = device;
								cpu_debugger = (DEBUGGER *)cpu->get_debugger();
								cpu_debugger->set_context_child(NULL);
								cpu_debugger->now_going = false;
								cpu_debugger->now_debugging = true;
								prev_debugger->now_debugging = prev_debugger->now_going = prev_debugger->now_suspended = prev_debugger->now_waiting = false;
								if(target_debugger != NULL) {
									target_debugger->now_device_debugging = false;
								}
								target = cpu;
								target_debugger = cpu_debugger;
								wait_count = 0;
								while(!p->request_terminate && !(cpu_debugger->now_suspended && cpu_debugger->now_waiting)) {
									if((wait_count++) == 100) {
										p->osd->set_console_text_attribute(OSD_CONSOLE_RED | OSD_CONSOLE_INTENSITY);
										my_printf(p->osd, _T("waiting until cpu is suspended...\n"));
									}
									p->osd->sleep(10);
								}
								dump_addr = 0;
								dasm_addr = cpu->get_next_pc();
								
								p->osd->set_console_text_attribute(OSD_CONSOLE_RED | OSD_CONSOLE_GREEN | OSD_CONSOLE_BLUE | OSD_CONSOLE_INTENSITY);
								if(cpu->get_debug_regs_info(buffer, array_length(buffer))) {
									my_printf(p->osd, _T("%s\n"), buffer);
								}
								
								p->osd->set_console_text_attribute(OSD_CONSOLE_RED | OSD_CONSOLE_INTENSITY);
								my_printf(p->osd, _T("breaked at %s\n"), my_get_value_and_symbol(cpu, _T("%08X"), cpu->get_next_pc()));
								
								p->osd->set_console_text_attribute(OSD_CONSOLE_GREEN | OSD_CONSOLE_BLUE | OSD_CONSOLE_INTENSITY);
								cpu->debug_dasm(cpu->get_next_pc(), buffer, array_length(buffer));
								my_printf(p->osd, _T("next\t%s  %s\n"), my_get_value_and_symbol(cpu, _T("%08X"), cpu->get_next_pc()), buffer);
								p->osd->set_console_text_attribute(OSD_CONSOLE_RED | OSD_CONSOLE_GREEN | OSD_CONSOLE_BLUE | OSD_CONSOLE_INTENSITY);
							}
						} else {
							my_printf(p->osd, _T("device not found\n"));
						}
					} else {
						my_printf(p->osd, _T("invalid parameter number\n"));
					}
				} else {
					my_printf(p->osd, _T("unknown command ! %s\n"), params[1]);
				}
			} else if(_tcsicmp(params[0], _T("!!")) == 0) {
				// do nothing
			} else if(_tcsicmp(params[0], _T("?")) == 0) {
				my_printf(p->osd, _T("D [<range>] - dump memory\n"));
				my_printf(p->osd, _T("E[{B,W,D}] <address> <list> - edit memory (byte,word,dword)\n"));
				my_printf(p->osd, _T("EA <address> \"<value>\" - edit memory (ascii)\n"));
				my_printf(p->osd, _T("I[{B,W,D}] <port> - input port (byte,word,dword)\n"));
				my_printf(p->osd, _T("O[{B,W,D}] <port> <value> - output port (byte,word,dword)\n"));
				my_printf(p->osd, _T("R - show register(s)\n"));
				my_printf(p->osd, _T("R <reg> <value> - edit register\n"));
				my_printf(p->osd, _T("S <range> <list> - search\n"));
				my_printf(p->osd, _T("U [<range>] - unassemble\n"));
				my_printf(p->osd, _T("UT [<steps>] - unassemble trace\n"));
				
				my_printf(p->osd, _T("H <value> <value> - hexadd\n"));
				my_printf(p->osd, _T("N <filename> - name\n"));
				my_printf(p->osd, _T("L [<range>] - load binary/hex/symbol file\n"));
				my_printf(p->osd, _T("W <range> - write binary/hex file\n"));
				
				my_printf(p->osd, _T("SC - clear symbol(s)\n"));
				my_printf(p->osd, _T("SL - list symbol(s)\n"));
				
				my_printf(p->osd, _T("BP <address> - set breakpoint\n"));
				my_printf(p->osd, _T("{R,W}BP <address> - set breakpoint (break at memory access)\n"));
				my_printf(p->osd, _T("{I,O}BP <port> [<mask>] - set breakpoint (break at i/o access)\n"));
				my_printf(p->osd, _T("[{R,W,I,O}]B{C,D,E} {*,<list>} - clear/disable/enable breakpoint(s)\n"));
				my_printf(p->osd, _T("[{R,W,I,O}]BL - list breakpoints\n"));
				my_printf(p->osd, _T("[{R,W,I,O}]CP <address/port> [<mask>] - set checkpoint (don't break)\n"));
				
				my_printf(p->osd, _T("G - go (press esc key to break)\n"));
				my_printf(p->osd, _T("G <address> - go and break at address (ignore breakpoints)\n"));
				my_printf(p->osd, _T("P - trace one opcode (step over, ignore breakpoints)\n"));
				my_printf(p->osd, _T("T [<count>] - trace (step in)\n"));
				my_printf(p->osd, _T("Q - quit\n"));
				
				my_printf(p->osd, _T("> <filename> - output logfile\n"));
				my_printf(p->osd, _T("< <filename> - input commands from file\n"));
				
				my_printf(p->osd, _T("! reset [all/cpu/target] - reset\n"));
				my_printf(p->osd, _T("! key <code> [<msec>] - press key\n"));
				my_printf(p->osd, _T("! device - enumerate debugger available device\n"));
				my_printf(p->osd, _T("! device <id/cpu> - select target device\n"));
				my_printf(p->osd, _T("! cpu - enumerate debugger available cpu\n"));
				my_printf(p->osd, _T("! cpu <id> - select target cpu\n"));
				my_printf(p->osd, _T("!! <remark> - do nothing\n"));
				
				my_printf(p->osd, _T("<value> - hexa, decimal(%%d), ascii('a')\n"));
			} else {
				my_printf(p->osd, _T("unknown command %s\n"), params[0]);
			}
		}
	}
	
	// stop debugger
	try {
		if(target_debugger != NULL) {
			target_debugger->now_device_debugging = false;
		}
		cpu_debugger->now_debugging = cpu_debugger->now_going = cpu_debugger->now_suspended = cpu_debugger->now_waiting = false;
	} catch(...) {
	}
	
	// release files
	if(logfile != NULL) {
		if(logfile->IsOpened()) {
			logfile->Fclose();
		}
		delete logfile;
		logfile = NULL;
	}
	if(cmdfile != NULL) {
		if(cmdfile->IsOpened()) {
			cmdfile->Fclose();
		}
		delete cmdfile;
		cmdfile = NULL;
	}
	
	// release console
	p->osd->close_console();
	
	p->running = false;
#ifdef _MSC_VER
	_endthreadex(0);
	return 0;
#else
	pthread_exit(NULL);
	return NULL;
#endif
}

void EMU::initialize_debugger()
{
	now_debugging = false;
}

void EMU::release_debugger()
{
	close_debugger();
}

void EMU::open_debugger(int cpu_index)
{
#if USE_RETROACHIEVEMENTS
    if (!RA_WarnDisableHardcore("opening the debugger"))
        return;
#endif

	if(!(now_debugging && debugger_thread_param.cpu_index == cpu_index)) {
		close_debugger();
		if(vm->get_cpu(cpu_index) != NULL && vm->get_cpu(cpu_index)->get_debugger() != NULL) {
			debugger_thread_param.emu = this;
			debugger_thread_param.osd = osd;
			debugger_thread_param.vm = vm;
			debugger_thread_param.cpu_index = cpu_index;
			debugger_thread_param.request_terminate = false;
#ifdef _MSC_VER
			if((hDebuggerThread = (HANDLE)_beginthreadex(NULL, 0, debugger_thread, &debugger_thread_param, 0, NULL)) != (HANDLE)0) {
#else
			if(pthread_create(&debugger_thread_id, NULL, debugger_thread, &debugger_thread_param) == 0) {
#endif
				stop_record_sound();
				stop_record_video();
				now_debugging = true;
			}
		}
	}
}

void EMU::close_debugger()
{
	if(now_debugging) {
		if(debugger_thread_param.running) {
			debugger_thread_param.request_terminate = true;
		}
#ifdef _MSC_VER
		WaitForSingleObject(hDebuggerThread, INFINITE);
		CloseHandle(hDebuggerThread);
#else
		pthread_join(debugger_thread_id, NULL);
#endif
		now_debugging = false;
	}
}

bool EMU::is_debugger_enabled(int cpu_index)
{
	return (vm->get_cpu(cpu_index) != NULL && vm->get_cpu(cpu_index)->get_debugger() != NULL);
}

void EMU::start_waiting_in_debugger()
{
	now_waiting_in_debugger = true;
	osd->mute_sound();
	osd->start_waiting_in_debugger();
}

void EMU::finish_waiting_in_debugger()
{
	osd->finish_waiting_in_debugger();
	now_waiting_in_debugger = false;
}

void EMU::process_waiting_in_debugger()
{
	osd->process_waiting_in_debugger();
	osd->sleep(10);
}

#endif

