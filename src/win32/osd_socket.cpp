/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2015.11.26-

	[ win32 socket ]
*/

#include "osd.h"

void OSD::initialize_socket()
{
	// init winsock
	WSADATA wsaData;
	WSAStartup(0x0101, &wsaData);
	
	// init sockets
	for(int i = 0; i < SOCKET_MAX; i++) {
		soc[i] = INVALID_SOCKET;
		socket_delay[i] = 0;
		recv_r_ptr[i] = recv_w_ptr[i] = 0;
	}
}

void OSD::release_socket()
{
	// release sockets
	for(int i = 0; i < SOCKET_MAX; i++) {
		if(soc[i] != INVALID_SOCKET) {
			shutdown(soc[i], 2);
			closesocket(soc[i]);
		}
	}
	
	// release winsock
	WSACleanup();
}

void OSD::notify_socket_connected(int ch)
{
	// winmain notify that network is connected
	vm->notify_socket_connected(ch);
}

void OSD::notify_socket_disconnected(int ch)
{
	// winmain notify that network is disconnected
	if(!socket_delay[ch]) {
		socket_delay[ch] = 1;//56;
	}
}

void OSD::update_socket()
{
	for(int i = 0; i < SOCKET_MAX; i++) {
		if(recv_r_ptr[i] < recv_w_ptr[i]) {
			// get buffer
			int size0, size1;
			uint8_t* buf0 = vm->get_socket_recv_buffer0(i, &size0, &size1);
			uint8_t* buf1 = vm->get_socket_recv_buffer1(i);
			
			int size = recv_w_ptr[i] - recv_r_ptr[i];
			if(size > size0 + size1) {
				size = size0 + size1;
			}
			char* src = &recv_buffer[i][recv_r_ptr[i]];
			recv_r_ptr[i] += size;
			
			if(size <= size0) {
				memcpy(buf0, src, size);
			} else {
				memcpy(buf0, src, size0);
				memcpy(buf1, src + size0, size - size0);
			}
			vm->inc_socket_recv_buffer_ptr(i, size);
		} else if(socket_delay[i] != 0) {
			if(--socket_delay[i] == 0) {
				vm->notify_socket_disconnected(i);
			}
		}
	}
}

bool OSD::initialize_socket_tcp(int ch)
{
	is_tcp[ch] = true;
	
	if(soc[ch] != INVALID_SOCKET) {
		disconnect_socket(ch);
	}
	if((soc[ch] = socket(PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
		return false;
	}
	if(WSAAsyncSelect(soc[ch], main_window_handle, WM_SOCKET0 + ch, FD_CONNECT | FD_WRITE | FD_READ | FD_CLOSE) == SOCKET_ERROR) {
		closesocket(soc[ch]);
		soc[ch] = INVALID_SOCKET;
		return false;
	}
	recv_r_ptr[ch] = recv_w_ptr[ch] = 0;
	return true;
}

bool OSD::initialize_socket_udp(int ch)
{
	is_tcp[ch] = false;
	
	disconnect_socket(ch);
	if((soc[ch] = socket(PF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
		return false;
	}
	if(WSAAsyncSelect(soc[ch], main_window_handle, WM_SOCKET0 + ch, FD_CONNECT | FD_WRITE | FD_READ | FD_CLOSE) == SOCKET_ERROR) {
		closesocket(soc[ch]);
		soc[ch] = INVALID_SOCKET;
		return false;
	}
	recv_r_ptr[ch] = recv_w_ptr[ch] = 0;
	return true;
}

bool OSD::connect_socket(int ch, uint32_t ipaddr, int port)
{
	struct sockaddr_in tcpaddr;
	tcpaddr.sin_family = AF_INET;
	tcpaddr.sin_addr.s_addr = ipaddr;
	tcpaddr.sin_port = htons((unsigned short)port);
	memset(tcpaddr.sin_zero, (int)0, sizeof(tcpaddr.sin_zero));
	
	if(connect(soc[ch], (struct sockaddr *)&tcpaddr, sizeof(tcpaddr)) == SOCKET_ERROR) {
		if(WSAGetLastError() != WSAEWOULDBLOCK) {
			return false;
		}
	}
	return true;
}

void OSD::disconnect_socket(int ch)
{
	if(soc[ch] != INVALID_SOCKET) {
		shutdown(soc[ch], 2);
		closesocket(soc[ch]);
		soc[ch] = INVALID_SOCKET;
	}
	vm->notify_socket_disconnected(ch);
}

bool OSD::listen_socket(int ch)
{
	return false;
}

void OSD::send_socket_data_tcp(int ch)
{
	if(is_tcp[ch]) {
		send_socket_data(ch);
	}
}

void OSD::send_socket_data_udp(int ch, uint32_t ipaddr, int port)
{
	if(!is_tcp[ch]) {
		udpaddr[ch].sin_family = AF_INET;
		udpaddr[ch].sin_addr.s_addr = ipaddr;
		udpaddr[ch].sin_port = htons((unsigned short)port);
		memset(udpaddr[ch].sin_zero, (int)0, sizeof(udpaddr[ch].sin_zero));
		
		send_socket_data(ch);
	}
}

void OSD::send_socket_data(int ch)
{
	// loop while send buffer is not emupty or not WSAEWOULDBLOCK
	while(1) {
		// get send buffer and data size
		int size;
		uint8_t* buf = vm->get_socket_send_buffer(ch, &size);
		
		if(!size) {
			return;
		}
		if(is_tcp[ch]) {
			if((size = send(soc[ch], (char *)buf, size, 0)) == SOCKET_ERROR) {
				// if WSAEWOULDBLOCK, WM_SOCKET* and FD_WRITE will come later
				if(WSAGetLastError() != WSAEWOULDBLOCK) {
					disconnect_socket(ch);
					notify_socket_disconnected(ch);
				}
				return;
			}
		} else {
			if((size = sendto(soc[ch], (char *)buf, size, 0, (struct sockaddr *)&udpaddr[ch], sizeof(udpaddr[ch]))) == SOCKET_ERROR) {
				// if WSAEWOULDBLOCK, WM_SOCKET* and FD_WRITE will come later
				if(WSAGetLastError() != WSAEWOULDBLOCK) {
					disconnect_socket(ch);
					notify_socket_disconnected(ch);
				}
				return;
			}
		}
		vm->inc_socket_send_buffer_ptr(ch, size);
	}
}

void OSD::recv_socket_data(int ch)
{
	if(is_tcp[ch]) {
		int size = SOCKET_BUFFER_MAX - recv_w_ptr[ch];
		char* buf = &recv_buffer[ch][recv_w_ptr[ch]];
		if((size = recv(soc[ch], buf, size, 0)) == SOCKET_ERROR) {
			disconnect_socket(ch);
			notify_socket_disconnected(ch);
			return;
		}
		recv_w_ptr[ch] += size;
	} else {
		SOCKADDR_IN addr;
		int len = sizeof(addr);
		int size = SOCKET_BUFFER_MAX - recv_w_ptr[ch];
		char* buf = &recv_buffer[ch][recv_w_ptr[ch]];
		
		if(size < 8) {
			return;
		}
		if((size = recvfrom(soc[ch], buf + 8, size - 8, 0, (struct sockaddr *)&addr, &len)) == SOCKET_ERROR) {
			disconnect_socket(ch);
			notify_socket_disconnected(ch);
			return;
		}
		size += 8;
		buf[0] = size >> 8;
		buf[1] = size;
		buf[2] = (char)addr.sin_addr.s_addr;
		buf[3] = (char)(addr.sin_addr.s_addr >> 8);
		buf[4] = (char)(addr.sin_addr.s_addr >> 16);
		buf[5] = (char)(addr.sin_addr.s_addr >> 24);
		buf[6] = (char)addr.sin_port;
		buf[7] = (char)(addr.sin_port >> 8);
		recv_w_ptr[ch] += size;
	}
}

