/******************************************************************************
	[CDInterface.h]

	CD-ROM デバイスを操作するためのインタフェイスを定義します。
	Define interface for controlling CD-ROM device.

	Copyright (C) 2004 Ki

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
******************************************************************************/
#ifndef CDROM_INTERFACE_H_INCLUDED
#define CDROM_INTERFACE_H_INCLUDED


#include "TypeDefs.h"


//#defines of flags for the status callback function
#define CDIF_SEEK			0x00000001
#define CDIF_READ			0x00000002
#define CDIF_SUBQ			0x00000004
#define CDIF_READCDDA		0x00000008//Kitao追加
#define CDIF_PLAYCDDA		0x00000010//Kitao追加
#define CDIF_READCDDA2		0x00000020//Kitao追加
#define CDIF_SEEKDATA		0x00000040//Kitao追加
#define CDIF_INSTALL		0x00000080//Kitao追加
#define CDIF_INSTALLWAV		0x00000100//Kitao追加
#define CDIF_SEEKHDD		0x00000200//Kitao追加
#define CDIF_SEEKDATAHDD	0x00000400//Kitao追加
#define CDIF_READHDD		0x00000800//Kitao追加
#define CDIF_READCDDAHDD	0x00001000//Kitao追加
#define CDIF_READCDDA2HDD	0x00002000//Kitao追加

#define CDIF_ERROR			0x80000000


/*-----------------------------------------------------------------------------
	[Init]
		初期化します。 
-----------------------------------------------------------------------------*/
Sint32 //v2.33更新。-1=エラー。0～=見つかったCD-ROM(DVD,BD)ドライブ数。
CDIF_Init(void (*callback)(Uint32 flags));


/*-----------------------------------------------------------------------------
	[Deinit]
		終了します。 
-----------------------------------------------------------------------------*/
void
CDIF_Deinit();

Sint32
CDIF_GetNumDevices();

BOOL
CDIF_SelectDevice(
	Sint32	deviceNum);

Sint32
CDIF_GetFirstTrack();

Sint32
CDIF_GetLastTrack();

//Kitao追加
Uint32
CDIF_GetTrackNumber(
	Uint32		m,
	Uint32		s,
	Uint32		f);

Uint32
CDIF_GetTrackStartPositionMSF(
	Sint32		track);

Uint32
CDIF_GetTrackStartPositionLBA(
	Sint32		track);


BOOL
CDIF_SetSpeed(
	Uint32		speed);		// 倍速 

BOOL
CDIF_IsDeviceBusy();


/*-----------------------------------------------------------------------------
	[ReadSector]
		セクタを読み出します。
-----------------------------------------------------------------------------*/
BOOL
CDIF_ReadSector(
	Uint8*			pBuf,				// 読み込んだセクタデータの保存先 
	Uint32			sector,				// セクタ番号 
	Uint32			nSectors,			// 読み出すセクタ数 
	BOOL			bCallback);

//Kitao追加。データをReadする直前用。セクターの先読みも行う。
BOOL
CDIF_SeekData(
	Uint8*			pBuf,				// 読み込んだセクタデータの保存先 
	Uint32			sector,				// セクタ番号 
	Uint32			nSectors,			// 読み出すセクタ数 
	BOOL			bCallback);

/*-----------------------------------------------------------------------------
	[ReadSubChannelQ]
		Ｑサブチャネルを読み出します。
-----------------------------------------------------------------------------*/
BOOL
CDIF_ReadSubChannelQ(
	Uint8*		pBuf,		// 10-byte buffer
	BOOL		bCallback);


//Kitao追加
BOOL
CDIF_SeekCdda(
	Uint8	minStart,
	Uint8	secStart,
	Uint8	frmStart);

//Kitao追加
BOOL
CDIF_ReadCddaSector(
	Uint8*		pBuf,				// 読み込んだセクタデータの保存先 
	Uint32		sector,				// セクタ番号 
	Sint32		nSectors,			// 読み出すセクタ数 
	BOOL		bCallback);

//Kitao追加
BOOL
CDIF_ReadCddaSectorHDD(
	Uint8*		pBuf,		// 読み込んだセクタデータの保存先 
	Sint32		track,		// 読み込むトラックナンバー
	Uint32		addr,		// 読み込むアドレス。トラック(＝ファイル)の先頭を0x0000とする。
	Sint32		nSectors,	// 読み出すセクタ数 
	BOOL		bCallback);

//Kitao更新
BOOL
CDIF_Seek(
	Uint8*		pBuf,		// 読み込んだセクタデータの保存先 
	Sint32		track,		// 読み込むトラックナンバー
	Uint32		sector,		// セクタ番号 
	Sint32		nSectors,	// 読み出すセクタ数 
	BOOL		bCallback);

//Kitao追加
BOOL
CDIF_SeekHDD(
	Uint8*		pBuf,		// 読み込んだセクタデータの保存先 
	Sint32		track,		// 読み込むトラックナンバー
	Uint32		addr,		// 読み込むアドレス。トラック(＝ファイル(WAVEヘッダがあるので45バイト目から))の先頭を0x0000とする。
	Sint32		nSectors,	// 読み出すセクタ数 
	BOOL		bCallback);

//Kitao追加
BOOL
CDIF_PlayCdda(
	BOOL	bCallback);

//Kitao追加
BOOL
CDIF_ReadCddaSector2(
	Uint8*		pBuf,				// 読み込んだセクタデータの保存先 
	Uint32		sector,				// セクタ番号 
	Sint32		nSectors,			// 読み出すセクタ数 
	BOOL		bCallback);

//Kitao追加
BOOL
CDIF_ReadCddaSector2HDD(
	Uint8*		pBuf,		// 読み込んだセクタデータの保存先 
	Sint32		track,		// 読み込むトラックナンバー
	Uint32		addr,		// 読み込むアドレス。トラック(＝ファイル)の先頭を0x0000とする。
	Sint32		nSectors,	// 読み出すセクタ数 
	BOOL		bCallback);

//Kitao追加
Sint32
CDIF_GetDriveLetters(
	int	n);

//Kitao追加
Sint32
CDIF_GetDeviceInUse();

//Kitao追加
BOOL
CDIF_CDInstall(
	Uint8*		pBuf,		// 読み込んだセクタデータの保存先 
	Uint32		sector,		// セクタ番号 
	Uint32		nSectors,	// 読み出すセクタ数 
	BOOL		bCallback);

//Kitao追加
BOOL
CDIF_CDInstallWav(
	Uint8*		pBuf,		// 読み込んだセクタデータの保存先 
	Sint32		track,		// 読み込むトラックナンバー
	Uint32		sector,		// セクタ番号 
	Uint32		nSectors,	// 読み出すセクタ数 
	BOOL		bCallback);

//Kitao追加
BOOL
CDIF_SeekDataHDD(
	Uint8*		pBuf,		// 読み込んだセクタデータの保存先
	Sint32		track,		// 読み込むトラックナンバー
	Uint32		addr,		// 読み込むアドレス。トラック(＝ファイル)の先頭を0x0000とする。
	Uint32		nSectors,	// 読み出すセクタ数
	BOOL		bCallback);

//Kitao追加
BOOL
CDIF_ReadSectorHDD(
	Uint8*		pBuf,		// 読み込んだセクタデータの保存先 
	Sint32		track,		// 読み込むトラックナンバー
	Uint32		addr,		// 読み込むアドレス。トラック(＝ファイル)の先頭を0x0000とする。
	Sint32		nSectors,	// 読み出すセクタ数 
	BOOL		bCallback);

//Kitao追加
BOOL
CDIF_CddaStartWait(
	BOOL		bCallback);

//Kitao追加。
void
CDIF_WaitDeviceBusy();

//Kitao追加
BOOL
CDIF_SetBadInstalled(
	FILE*	fp);

//Kitao追加
BOOL
CDIF_GetBadInstalled();


#endif /* CDROM_INTERFACE_H_INCLUDED */
