/***********************************************************
 *                                                         *
 * CCnet.c : ConsoleClassix network support                *
 *                                                         *
 * This source is a part of Gens project                   *
 * Written by Stéphane Dallongeville                       *
 * Copyright (c) 2002 by Stéphane Dallongeville            *
 *                                                         *
 ***********************************************************/

#include <stdio.h>
#include <windows.h>
#include <wininet.h>
#include <winsock.h>

#include "CCnet.h"


_CC_Rom CCRom;

static char strError[256];
static char rcvbuf[256];
static char UserName[256];
static char Password[256];
static char ServerName[256];
static char GameName[256];

static SOCKET sock = (int) NULL;


int CC_TranslateCommand(char *command, char user[256], char password[256], char server[256], char game[256])
{
	int i;
	char *com;

	com = command;
	memset(user, 0, 256);
	memset(password, 0, 256);
	memset(server, 0, 256);
	memset(game, 0, 256);

	if( _strnicmp( com, "CCGEN://", 8 ) )
		return 1;
	com += 8;

	i = 0;
	while ((com[i] != ':') && (com[i] != '\0')) i++;
	strncpy_s( user, i, com, 256 );
	if (!com[i]) return 1;
	com += i + 1;

	i = 0;
	while ((com[i] != '@') && (com[i] != '\0')) i++;
	strncpy_s( password, i, com, 256 );
	if (!com[i]) return 1;
	com += i + 1;

	i = 0;
	while ((com[i] != '/') && (com[i] != '\0')) i++;
	strncpy_s( server, i, com, 256 );
	if (!com[i]) return 1;
	com += i + 1;

	i = 0;
	while ((com[i] != ' ') && (com[i] != '\0')) i++;
	strncpy_s( game, i, com, 256 );

	if ((*user) && (*password) && (*server) && (*game)) 
		return 0;

	return 1;
}

	
int CC_Connect(char *Command, char *Rom, void (*Callback_End)(char mess[256]))
{
	int i, fileSize;
	char *buffer;
	WSADATA wsData;
	int statusCounter;
	unsigned long ip_addr;
	struct hostent *hostAddr;
	struct sockaddr_in ServerAddress;
	struct sockaddr_in clientIP;

	memset(&CCRom, 0, sizeof(CCRom));

	if (CC_TranslateCommand(Command, UserName, Password, ServerName, GameName))
	{
//		wsprintf(strError, "Can't resolve parameters: %s:%s (%s) -> %s", UserName, Password, ServerName, GameName);
//		MessageBox(NULL, strError, NULL, MB_OK | MB_ICONEXCLAMATION);
		return 2;
	}
	
/* 
	This Demo uses a dialog box, the real client will get this from the command line.
	CCGEN://Username:Password@emu.consoleclassix.com/gamename
	CCGEN:// will tell the OS to open the client and pass the command line arguments.
	Username and Password are, of course, the username and password.
	Emu.consoleclassix.com is the server name.
	Gamename is the name of the game.

	// Set ServerName
	ServerName = "emu.consoleclassix.com";

	// Set GameName. I use smdh.nes in the example 
	// because we have 22 copies so one is always free.
	// If you want a Gen game use sonicthehedgehog2.gen
	strcpy(GameName,"smdh.nes");
*/

	// Clear the receive buffer
	memset(rcvbuf, 0, sizeof(rcvbuf));

	// Begin: Init Winsock2
	if (WSAStartup(MAKEWORD(2, 2), &wsData))
	{
		MessageBox(NULL, "Can't find a suitable DLL to create socket", "Error", MB_OK | MB_ICONEXCLAMATION);
		return 1;
	}

	// Create the socket.
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET)
	{
		MessageBox(NULL, "Error Creating Socket", NULL, MB_OK | MB_ICONEXCLAMATION);
		return 1;
	}

	// Ignore this. It is used to specify the host by Ip.
	// ip_addr = inet_addr("10.96.10.75"); 
	hostAddr = gethostbyname(ServerName);
	if (hostAddr == NULL)
	{
		wsprintf(strError, "Error getting host: %d", WSAGetLastError());
		MessageBox(NULL, strError, NULL, MB_OK | MB_ICONEXCLAMATION);
		closesocket(sock);
		return 1;
	}

	ip_addr = *((long *) hostAddr->h_addr_list[0]);

	// Create the struct for the server
	ServerAddress.sin_family = AF_INET;
	ServerAddress.sin_port = htons(11001);	// Set the port to 11001
	ServerAddress.sin_addr.s_addr = ip_addr;	// Set the Ip Address

	for (i = 0; i < 8; i++) ServerAddress.sin_zero[i] = 0;

	// Connect to the server
	if (connect(sock, (const struct sockaddr *) &ServerAddress, sizeof(ServerAddress)) == SOCKET_ERROR)
	{
		wsprintf(strError, "Connecting to Server Failed: %d", WSAGetLastError());
		MessageBox(NULL, strError, "Error", MB_OK | MB_ICONEXCLAMATION);
		closesocket(sock);
		return 1;
	}

	// Send the username
	if (send(sock, UserName, strlen(UserName), 0) == SOCKET_ERROR)
	{
		wsprintf(strError, "Error sending data: %d", WSAGetLastError());
		MessageBox(NULL, strError, "Error-Client", MB_OK | MB_ICONEXCLAMATION);
		closesocket(sock);
		return 1;
	}

	// Receive response
	if (recv(sock, rcvbuf, sizeof(rcvbuf), 0) == SOCKET_ERROR)
	{
		wsprintf(strError, "Error receiving data: %d", WSAGetLastError());
		MessageBox(NULL, strError, "Error-Client", MB_OK | MB_ICONEXCLAMATION);
		closesocket(sock);
		return 1;
	}

	// Clear the receive buffer 
	memset(rcvbuf, 0, sizeof(rcvbuf));

	// Send the Password
	if (send(sock, Password, strlen(Password), 0) == SOCKET_ERROR)
	{
		wsprintf(strError, "Error sending data: %d", WSAGetLastError());
		MessageBox(NULL, strError, "Error-Client", MB_OK | MB_ICONEXCLAMATION);
		closesocket(sock);
		return 1;
	}

	// Receive response
	if (recv(sock, rcvbuf, sizeof(rcvbuf), 0) == SOCKET_ERROR)
	{
		wsprintf(strError, "Error receiving data: %d", WSAGetLastError());
		MessageBox(NULL, strError, "Error-Client", MB_OK | MB_ICONEXCLAMATION);
		closesocket(sock);
		return 1;
	}

	// If the reponse is "ok" 
	if (!strcmp("ok", rcvbuf))
	{
		// Get the client's ip address
		char ipStr[64];
		int IPLen = sizeof(clientIP);

		getsockname(sock, (struct sockaddr *) &clientIP, &IPLen);
		wsprintf(ipStr, "%s", inet_ntoa(clientIP.sin_addr));
		
		// Send client ip
		send(sock, ipStr, 64, 0);

		// Receive response
		recv(sock, rcvbuf, 3, 0);

		MessageBox(NULL, "Welcome to Console Classix !", "Login", MB_OK);
	}
	else
	{
		MessageBox(NULL, "You Have Been Disconnected", NULL, MB_OK);
		closesocket(sock);
		return 1;
	}

	// The client will now be connected to the CC server. 
	// Now we'll open a game

	// Clear the receive buffer 
	memset(rcvbuf, 0, sizeof(rcvbuf));

	// Send Open Notification to server
	send(sock, "op", sizeof("op"), 0);

	// Receive response
	recv(sock, rcvbuf, sizeof(rcvbuf), 0);

	// If reponse is "ok"
	if (!strcmp(rcvbuf, "ok"))
	{
		// Clear the receive buffer 
		memset(rcvbuf, 0, sizeof(rcvbuf));

		// Send GameName to the server.
		if (send(sock, GameName, strlen(GameName), 0) == SOCKET_ERROR)
		{
			wsprintf(strError, TEXT("Sending Data Failed.  Error: %d"), WSAGetLastError());
			MessageBox(NULL, strError, NULL, MB_OK | MB_ICONEXCLAMATION);
			closesocket(sock);
			return 1;
		}

		// Receive response
		recv(sock, rcvbuf, sizeof(rcvbuf), 0);

		// If response is not "ok"
		if(strcmp(rcvbuf,"ok"))
		{
			// Display responce from server and exit
			MessageBox(NULL, rcvbuf, NULL, MB_OK);
			closesocket(sock);
			return 1;
		}
	}
	else
	{
		MessageBox(NULL, "Error loading game, please try again.", NULL, MB_OK);
		closesocket(sock);
		return 1;
	}
	
	// Clear the receive buffer 
	memset(rcvbuf, 0, sizeof(rcvbuf));

	// Send "ok" to server
	send(sock, "ok", sizeof("ok"), 0);

	// Receive file size.
	if (recv(sock, rcvbuf, sizeof(rcvbuf), 0) == SOCKET_ERROR)
	{
		wsprintf(strError, "Recv Failed, Error: %d", WSAGetLastError());
		MessageBox(NULL, strError, "Client", MB_OK | MB_ICONEXCLAMATION);
		closesocket(sock);
		return 1;
	}
	else
	{
		fileSize = strtoul(rcvbuf, NULL, 10);
	}

	// Clear the receive buffer 
	memset(rcvbuf, 0, sizeof(rcvbuf));

	// Send "ok" to server
	send(sock, "ok", sizeof("ok"), 0);

	// Initialise file pointer to Rom address
	buffer = Rom;

	// Set the statusCounter to 0
	statusCounter=0;

	// Loop used to receive file in buffer size hunks
	for (i = 0; i < fileSize; i += statusCounter)
	{
		// Set statusCounter equal to the number of bytes received up to buffer max
		statusCounter = recv(sock, rcvbuf, sizeof(rcvbuf), 0);

		if (statusCounter == SOCKET_ERROR)
		{
			wsprintf(strError, "Recv Failed, Error: %d", WSAGetLastError());
			MessageBox(NULL, strError, "Client", MB_OK | MB_ICONEXCLAMATION);
			closesocket(sock);
			return 1;
		}
		else
		{
			// Copy the receive buffer into memory
			memcpy(buffer, rcvbuf, statusCounter);
			
			// Move memTrack up through the bytes read
			buffer += statusCounter;
		}
	}

	// Once this is completed, we fill the CCRom structure
	strcpy_s(CCRom.RName, 256, GameName);
	CCRom.RSize = fileSize;

	// This is just to let you know everything went as it should. 
	// No need to have this in the client.
	MessageBox(NULL, "File Received !", "File Status", MB_OK);

	/* This is the Query Roms feature. You can ignore it as you won't be using it.
	send(sock, "qr", 3, 0);

	memset(wszrcvbuf,0,sizeof(wszrcvbuf));
	recv(sock, wszrcvbuf, sizeof(wszrcvbuf), 0);

	send(sock, "NES",(sizeof("NES")+1),0);
	
	memset(wszrcvbuf,0,sizeof(wszrcvbuf));
	recv(sock, wszrcvbuf, sizeof(wszrcvbuf), 0);

	send(sock, "^[a-b]",(sizeof("^[a-b]")+1),0);
	
	memset(wszrcvbuf,0,sizeof(wszrcvbuf));
	recv(sock, wszrcvbuf, sizeof(wszrcvbuf), 0);

	send(sock, "ok", 3, 0);

	memset(wszrcvbuf,0,sizeof(wszrcvbuf));
	recv(sock, wszrcvbuf, sizeof(wszrcvbuf), 0);
	unsigned long count = strtoul(wszrcvbuf, NULL, 10);

	FILE* test;
	test = fopen("test.doc","wb");

	for(unsigned long counter = 0;counter<count;counter++)
	{
		do
		{
			memset(wszrcvbuf,0,sizeof(wszrcvbuf));
			recv(sock, wszrcvbuf, 1, 0);
			fwrite(wszrcvbuf,1,1,test);
		}
		while(strcmp("\0",&wszrcvbuf[0]));
		fwrite("\n",1,1,test);
	}

	fclose(test);
	*/

	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) CC_Idle, (LPVOID) Callback_End, 0, NULL);

	return 0;
}

int CC_Close(void)
{
	if (sock)
	{
		closesocket(sock);
		sock = (int) NULL;
		return 0;
	}

	return 1;
}

void CC_Idle(void (*Callback)(char mess[256]))
{
	memset(rcvbuf, 0, sizeof(rcvbuf));
	strcpy_s(rcvbuf, 256, "ok");

	while ((sock) && (!strcmp(rcvbuf, "ok")))
	{
		Sleep(5000);
		
		send(sock, "hi", sizeof("hi"), 0);
		memset(rcvbuf, 0, sizeof(rcvbuf));
		recv(sock, rcvbuf, sizeof(rcvbuf), 0);
	}

	if (sock) Callback(rcvbuf);
	else Callback("Rom closed");

	CC_Close();
	ExitThread(0);
}
