/******************************************************************************
Ootake
・CD-DAの再生をWAVで行うようにしたので、CD-DAのセクターを読み込むルーチンを追
  加した。
・ファイルハンドルの作成を初回にだけ行うようにして、CDアクセスを高速化＆安定性
  を上げた。以前より多くの環境でシャーロックホームズやイース４などのアクセスに
  シビアなゲームが快適に動くようになった。v1.00
・CD-DA専用のシークルーチン（PCE側に結果を返さない）を追加した。
・データリード専用のシークルーチンを追加した。v0.61
・NT系のOSで利用した場合、安定性を考え、必ずSPTIを使うようにした。v0.57
・ASPI使用の場合もドライブ名を設定するようにした。v0.57
・ファイル名をASPICdrom.c からCDInerface.c 変更した。v0.64
・リードエラー(データトラックを音楽再生しようとした場合など)時の処理を実機の動
  作に近づけた。v2.11

Copyright(C)2006-2011 Kitao Nakamura.
	改造版・後継版を公開なさるときは必ずソースコードを添付してください。
	その際に事後でかまいませんので、ひとことお知らせいただけると幸いです。
	商的な利用は禁じます。
	あとは「GNU General Public License(一般公衆利用許諾契約書)」に準じます。

*******************************************************************************
	[CDInterface.c]

	CD-ROM デバイスの制御を SPTIまたは ASPI マネージャを利用して実装します。

	Implement CD-ROM device control using ASPI manager.

	[Reference]
		C Magazine 11, 2001 自在に操るSCSI/ATAPI

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
#define _CRT_SECURE_NO_DEPRECATE
#include <stdio.h>
#include <stddef.h>				// offsetof(structName, memberName)
#include "CDInterface.h"
#include "SCSIDEFS.h"
#include "WNASPI32.h"
#include "Printf.h"
#include "CDROM.h"
#include "ADPCM.h"
#include "WinMain.h"
#include "App.h"
#include "APU.h"

#define CDIF_EVENT		TEXT("CDIF_EVENT")
#define CDIF_EXIT		0x80000000

/* defines for SPTI */
#define IOCTL_SCSI_PASS_THROUGH_DIRECT	0x4D014
#define SCSI_IOCTL_DATA_IN				1

typedef struct
{
	Uint16			Length;
	Uint8			ScsiStatus;
	Uint8			PathId;
	Uint8			TargetId;
	Uint8			Lun;
	Uint8			CdbLength;
	Uint8			SenseInfoLength;
	Uint8			DataIn;
	unsigned int	DataTransferLength;
	Uint32			TimeOutValue;
	void*			DataBuffer;
	unsigned int	SenseInfoOffset;
	Uint8			Cdb[16];
} SCSI_PASS_THROUGH_DIRECT;

typedef struct
{
	SCSI_PASS_THROUGH_DIRECT		sptd;
    Uint32							Filler;			// realign buffer to double word boundary
    Uint8							ucSenseBuf[32];
} SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER;	

typedef DWORD (*LPGETASPI32SUPPORTINFO)(void);
typedef DWORD (*LPSENDASPI32COMMAND)(LPSRB);

typedef struct
{
	Uint32		adapter;
	Uint32		target;
	Uint32		lun;
	BOOL		bATAPI;
} CdromInfo;

typedef struct
{
	Uint32		lba;
	BOOL		bAudio;
} TrackInfo;

//v2.32更新。playSecとelapsedSecをカット。track(startLBAのトラック)を追加。
typedef struct
{
	Uint32		command;
	Uint8*		pBuf;
	Uint32		startLBA;
	Uint32		endLBA;
	Sint32		track;
	BOOL		bPlaying;
	BOOL		bPaused;
	BOOL		bCallback;
} CdArg;

static HANDLE					_hWNASPI32 = INVALID_HANDLE_VALUE;
static LPGETASPI32SUPPORTINFO	_pfnGetASPI32SupportInfo;
static LPSENDASPI32COMMAND		_pfnSendASPI32Command;

static Sint32					_FirstTrack = 1;
static Sint32					_LastTrack;

static Sint32					_nAdapters;

static BOOL						_bInit = FALSE; //Kitao追加。v1.04
static Sint32					_nCdromDevice;
static Sint32					_DeviceInUse;
static CdromInfo				_CdromInfo[4];

static TrackInfo				_TrackInfo[256];	// [0] は使わない 

static volatile BOOL			_bDeviceBusy = FALSE;

static HANDLE					_hEvent;
static HANDLE					_hThread = INVALID_HANDLE_VALUE;
static DWORD					_dwThreadID;
static volatile CdArg			_CdArg; //v2.05更新。volatileに。

static Sint32					_DriveLetters[26];	// 'A'～'Z'。Kitao更新。ASPIのときも設定するようにした。
static BOOL						_bUseSpti;
static HANDLE					_SPTIfileHandle = INVALID_HANDLE_VALUE; //Kitao追加。v1.00

static void						(*_Callback)(Uint32);

static BYTE						_ReadBuf[2048 + 0x10]; //Kitao更新。初回に一度だけ容量確保することで高速化。v1.00
static BYTE*					_pReadBuf; //Kitao更新
static BYTE						_CDDAReadBuf[2352 + 0x10]; //Kitao追加。CDDA用。v1.00
static BYTE*					_pCDDAReadBuf; //Kitao追加

static BOOL						_bBadInstalled; //cue起動した際に、古いOotakeでリッピングしたための不具合がある場合TRUEに。v2.31


//Kitao追加。_CdArgをクリアする
static void
clearCdArg()
{
	ZeroMemory((CdArg*)&_CdArg, sizeof(_CdArg));
}


//Kitao追加。CDデバイスを使用中のときに待つ
//タイミングによってはAPUスレッドの処理と同時にアクセスしてしまう可能性があるためそれも考慮。v2.03
static void
waitDeviceBusy()
{
	int		i;
	DWORD	t1, t2;

	//APUスレッドが処理中の場合終わるまで待つ
	t1 = timeGetTime();
	t2 = t1 + 10000;//10秒以上待った場合は、メインスレッドでハード的なトラブルが出た可能性が高いと想定し、Sleep(1)へ切り替えてOSの安定に備える。
	while (APU_GetApuBusy())
		WINMAIN_SafetySleepZero(t1, t2); //メインスレッドでエミュレート処理中の場合、1/1000秒待ちは大きいので(0)で待つ。v2.04。安全にSleep(0)を行う。v2.42更新

	//CDデバイスを使用中の場合終わるまで待つ。40秒経過しても反応がない場合はエラーと判断して無限ループを抜ける。
	i = 0;
	while (_bDeviceBusy && i<40000)
	{
		Sleep(1); //※40秒経過判定のためSleep(0)は駄目。v2.05
		i++;
	}
}


//Kitao追加。
void
CDIF_WaitDeviceBusy()
{
	waitDeviceBusy();
}


static void
lba2msf(
	Uint32		lba,
	Uint32*		m,
	Uint32*		s,
	Uint32*		f)
{
	*m = lba / 75 / 60;
	*s = (lba - *m * 75 * 60) / 75;
	*f = lba - (*m * 75 * 60) - (*s * 75);
}


static Uint32
msf2lba(
	Uint32		m,
	Uint32		s,
	Uint32		f)
{
	return (m*60 + (s-2)) * 75 + f;
}


/*-----------------------------------------------------------------------------
	[execute_scsi_command]
		SCSI アダプタへコマンドを発行します。 
-----------------------------------------------------------------------------*/
static BOOL
execute_scsi_command(
	Uint8			*pOutBuf,
	int				outBufLen,
	BYTE			HaId,
	BYTE			targetId,
	BYTE			Lun,
	Uint8			*pCDB,
	int				cdbLen)
{
	//SPTI用
	SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER	swb;
	ULONG									ulReturned;
	BOOL									res;
	char									path[10];
	//ASPI用
	HANDLE				hEvent;	// 終了通知を行なうオブジェクトへのハンドル
	SRB_ExecSCSICmd		cmd;	// コマンド構造体 

	if (_nCdromDevice == 0) //ドライブ未接続の場合。v2.33追加
		return FALSE;

	if (_bUseSpti)
	{	//SPTIでコマンドを実行するための処理 
		ZeroMemory(&swb, sizeof(swb));

		swb.sptd.Length = sizeof(SCSI_PASS_THROUGH_DIRECT);
		swb.sptd.CdbLength          = cdbLen;				// コマンドの長さ
		swb.sptd.SenseInfoLength    = 24;					//
		swb.sptd.DataIn             = SCSI_IOCTL_DATA_IN;	// 読みとりモード
		swb.sptd.DataBuffer         = pOutBuf;				// バッファポインタ
		swb.sptd.DataTransferLength = outBufLen;			// バッファサイズ
		swb.sptd.TimeOutValue       = 30;					// タイムアウトまでの時間。v2.31更新
		swb.sptd.SenseInfoOffset    = offsetof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER, ucSenseBuf); // ucSenseBufのオフセット
		//swb.sptd.PathId = 0; //Kitao更新
		//swb.sptd.TargetId = 0; //Kitao更新
		//swb.sptd.Lun = 0; //Kitao更新

		memcpy(swb.sptd.Cdb, pCDB, cdbLen);

		res = DeviceIoControl(	_SPTIfileHandle,
								IOCTL_SCSI_PASS_THROUGH_DIRECT,
								&swb,
								sizeof(swb),
								&swb,
								sizeof(swb),
								&ulReturned,
								NULL);
		if(res && swb.sptd.ScsiStatus == 0)
			return TRUE;
		else
		{	//Kitao更新。失敗した場合、ハンドルを再取得してもう一度試みる。v1.00
			if (_SPTIfileHandle != INVALID_HANDLE_VALUE)
				CloseHandle(_SPTIfileHandle);
			sprintf(path, "\\\\.\\%c:", (int)(_DriveLetters[_DeviceInUse]));
			_SPTIfileHandle = CreateFile(path,
										GENERIC_READ | GENERIC_WRITE,
										FILE_SHARE_READ,
										NULL,
										OPEN_EXISTING,
										0,
										NULL); 
			res = DeviceIoControl(	_SPTIfileHandle,
									IOCTL_SCSI_PASS_THROUGH_DIRECT,
									&swb,
									sizeof(swb),
									&swb,
									sizeof(swb),
									&ulReturned,
									NULL);
			if(res && swb.sptd.ScsiStatus == 0)
				return TRUE;
		}
	}
	else
	{	//ASPIでコマンドを実行するための処理 
		// notification event を作成する 
		if ((hEvent = CreateEvent(NULL, TRUE, FALSE, NULL)) == NULL)
			return FALSE;

		ZeroMemory(&cmd, sizeof(SRB_ExecSCSICmd));
		cmd.SRB_Cmd 		= SC_EXEC_SCSI_CMD;
		cmd.SRB_HaId		= HaId;
		cmd.SRB_Flags		= SRB_DIR_IN | SRB_EVENT_NOTIFY; // 読み出し＆通知を指定 
		cmd.SRB_Target		= targetId;
		cmd.SRB_Lun			= Lun;
		cmd.SRB_BufLen		= outBufLen;
		cmd.SRB_SenseLen	= SENSE_LEN;
		cmd.SRB_PostProc	= (LPVOID)hEvent;

		cmd.SRB_BufPointer	= pOutBuf;
		cmd.SRB_BufLen		= outBufLen;
		cmd.SRB_CDBLen		= cdbLen;

		CopyMemory(cmd.CDBByte, pCDB, cdbLen);

		// コマンドを実行し、終了通知を待つ 
		if (_pfnSendASPI32Command((void*)&cmd) == SS_PENDING)
			WaitForSingleObject(hEvent, INFINITE);

		CloseHandle(hEvent);

		if (cmd.SRB_Status == SS_COMP)
			return TRUE;
	}

	return FALSE;
}


static void
service_abort(
	int			adapter,
	void*		pSRB)
{
	SRB_Abort	SRBforAbort;

	// SRBのアボート処理
	ZeroMemory(&SRBforAbort, sizeof(SRBforAbort));
	SRBforAbort.SRB_Cmd     = SC_ABORT_SRB;
	SRBforAbort.SRB_HaId    = (BYTE)adapter;
	SRBforAbort.SRB_ToAbort = pSRB;
	if (_pfnSendASPI32Command(&SRBforAbort) != SS_COMP)
	{
		PRINTF("service_abort() failed.");
	}
}


static int
get_device_type(
	int		adapter,
	int		target,
	int		lun,
	BOOL*	pbATAPI)
{
	const DWORD			TIMEOUT = 10 * 1000;
	HANDLE				hEvent;
	DWORD				waitStatus;
	SRB_ExecSCSICmd		srb;
	BYTE				localBuf[256];
	BYTE*				pBuf;
	int					devicetypenum;

	hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (hEvent == NULL)	return -1;

	*pbATAPI = FALSE;

	ZeroMemory(localBuf, sizeof(localBuf));

	// 先頭アドレスが１６バイト境界のバッファを用意する 
	pBuf = (BYTE*)(((Uint32)&localBuf[0] + 0xF) & ~0xF);

	ZeroMemory(&srb, sizeof(srb));
	srb.SRB_Cmd			= SC_EXEC_SCSI_CMD;
	srb.SRB_HaId		= (BYTE)adapter;
	srb.SRB_Lun			= (BYTE)lun;
	srb.SRB_Target		= (BYTE)target;
	srb.SRB_Flags		= SRB_DIR_IN | SRB_EVENT_NOTIFY;
	srb.SRB_BufLen		= 128;
	srb.SRB_SenseLen	= SENSE_LEN;
	srb.SRB_BufPointer	= pBuf;
	srb.SRB_CDBLen		= 6;
	srb.SRB_PostProc	= (LPVOID)hEvent;
	srb.CDBByte[0]		= SCSI_INQUIRY;
	srb.CDBByte[4]		= 128;

	if (_pfnSendASPI32Command(&srb) == SS_PENDING)
	{
		waitStatus = WaitForSingleObject(hEvent, TIMEOUT);
		if (waitStatus == WAIT_TIMEOUT)
		{
			service_abort(adapter, &srb);
			return -1;
		}
	}
	CloseHandle(hEvent);

	if (srb.SRB_Status == SS_COMP)
	{
		devicetypenum = (int)pBuf[0] & 0x1F;
		if (((pBuf[2] & 7) == 0) || (srb.SRB_TargStat != STATUS_GOOD))
			*pbATAPI = TRUE;
	}
	else
		return -1;

	return devicetypenum;
}


static Sint32
scan_cdrom_devices()
{
	SRB_HAInquiry	srb;
	int				adapter;
	int				target;
	int				lun;
	BOOL			bATAPI;
	int				nDevice;

	nDevice = 0;

	for (adapter = 0; adapter < _nAdapters; adapter++)
	{
		ZeroMemory(&srb, sizeof(srb));
		srb.SRB_Cmd   = SC_HA_INQUIRY;
		srb.SRB_Flags = 0;
		srb.SRB_HaId  = (BYTE)adapter;

		if (_pfnSendASPI32Command(&srb) != SS_COMP)
			return FALSE;

		for (target = 0; target < 8; target++)
		{
			if (target == srb.HA_SCSI_ID)
				continue;

			for (lun = 0; lun < 8; lun++)
			{
				if (get_device_type(adapter, target, lun, &bATAPI) != DTYPE_CDROM)
					continue;

				_CdromInfo[nDevice].adapter = adapter;
				_CdromInfo[nDevice].target  = target;
				_CdromInfo[nDevice].lun     = lun;
				_CdromInfo[nDevice].bATAPI  = bATAPI;
				++nDevice;
			}
		}
	}

	return nDevice;
}


/*-----------------------------------------------------------------------------
	[read_toc]
		全トラックの TOC を読み出します。
-----------------------------------------------------------------------------*/
static BOOL
read_toc()
{
	BYTE	cdb[10];
	BYTE	toc[0x400+0x10];
	BYTE*	pTOC;
	BYTE*	pLBA;
	BYTE	ha  = (BYTE)_CdromInfo[_DeviceInUse].adapter;
	BYTE	tg  = (BYTE)_CdromInfo[_DeviceInUse].target;
	BYTE	lun = (BYTE)_CdromInfo[_DeviceInUse].lun;
	Sint32	i;
	Sint32	j;

	const Sint32	nTrial = 3;

	if (_nCdromDevice == 0)
		return FALSE;

	// 先頭アドレスが１６バイト境界のバッファを用意する 
	pTOC = (BYTE*)(((Uint32)&toc[0] + 0xF) & ~0xF);

	for (i = 0; i < nTrial; i++)
	{
		ZeroMemory(cdb, sizeof(cdb));
		cdb[0] = 0x43;		// read TOC
		cdb[1] = 0x00;		// 0x02 for MSF
		cdb[7] = 0x04;
		cdb[8] = 0x00;

		if (!execute_scsi_command(pTOC, 0x400, ha, tg, lun, cdb, sizeof(cdb)))
			continue;

		_LastTrack = (Uint32)pTOC[3];

		for (j = 1; j <= _LastTrack+1; j++)
		{
			pLBA = &pTOC[4+(j-1)*8+4];
			_TrackInfo[j].lba = (pLBA[0]<<24) + (pLBA[1]<<16) + (pLBA[2]<<8) + pLBA[3];
			_TrackInfo[j].bAudio = ((pTOC[4+(j-1)*8+1] & 0x4) == 0) ? TRUE : FALSE;
//			printf("track %d\n", j+1);
//			printf("LBA = %d\n", _TrackInfo[j+1].lba);
//			printf("bAudio = %d\n", _TrackInfo[j+1].bAudio);
		}
		return TRUE;
	}

	return FALSE;
}


//Kitao追加。インストール時のOotakeバージョンを読み込んで正常にリッピング出来ているファイルかどうかを判断する。CDROM.cppからも利用。v2.31
BOOL //バージョンの記載がなかった場合FALSEを返す。バージョン記載の有無にかかわらず、_bBadInstalledが設定される。
CDIF_SetBadInstalled(
	FILE*	fp)
{
	char	buf[256];
	char	ver[4];
	int		v;

	_bBadInstalled = FALSE;
	strcpy(buf, "");
	if (fgets(buf, 255, fp))
	{
		if (strstr(buf,"REM ver"))
		{
			ver[0] = buf[7];
			ver[1] = buf[8];
			ver[2] = buf[9];
			ver[3] = 0;
			v = atoi(ver);
			if (v < 232) //v2.32未満の場合、音楽トラック１等のリッピング不具合あり。
				_bBadInstalled = TRUE;
			if (v == 250) //v2.50の場合、音楽トラック１とCUEファイルのリッピング不具合あり。
				_bBadInstalled = TRUE;
			return TRUE;
		}
		else //バージョン記載がない場合
			_bBadInstalled = TRUE; //音楽トラック１等のリッピング不具合可能性あり。
	}
	return FALSE;
}

//Kitao追加。CueファイルからTOCを作成する。v2.24
static BOOL
read_toc_cue()
{
	FILE*	fp;
	FILE*	fp2;
	char	buf[256];
	char	buf2[256];
	char*	r;
	Uint32	tn;	
	Uint32	lba;
	char*	pi;
	char*	pi2;
	char	installPath[MAX_PATH+1];
	char	fn[MAX_PATH+1];
	DWORD	size;

	fp = fopen(APP_GetCueFilePathName(), "r");
	if (fp == NULL)	return FALSE;

	SetCursor(LoadCursor(NULL, IDC_WAIT)); //トラックが多いと時間が掛かるので、カーソルを砂時計に。

	strcpy(installPath, APP_GetCueFilePathName());
	pi = strrchr(installPath, '\\');
	if (pi != NULL)	*(pi+1)=0; //installPathのファイル名部分をカット

	tn = 1; //トラックナンバー
	lba = 0;

	//v2.31追加。インストール時のOotakeバージョンを読み込んで正常にリッピング出来ているファイルかどうかを判断する。
	if (!CDIF_SetBadInstalled(fp))
	{	//バージョン記載がない場合(v2.30以前のcue)、ファイルを先頭から開きなおす。
		fclose(fp);
		fp = fopen(APP_GetCueFilePathName(), "r");
	}

	while (TRUE)
	{
		strcpy(buf, "");
		while (strstr(buf,"FILE ") == NULL)
		{
			r = fgets(buf, 255, fp);
			if (r == NULL) break;
		}
		if (r == NULL)
			 break; //"FILE"が見つからなくなったら終了
		pi = strstr(buf,"FILE ") + 5;
		if (*pi == '"') pi++;
		strcpy(buf2, pi);
		pi2 = strstr(buf2,".");
		if (pi2 != NULL)
		{
			pi2 += 4;
			strcpy(buf, pi2);
			*pi2 = 0; //buf2の内容がファイル名のみになる
		}
		strcpy(fn, installPath);
		strcat(fn, buf2);
		fp2 = fopen(fn, "rb");
		if (fp2 == NULL) break;
		fseek(fp2, 0, SEEK_END);
		size = ftell(fp2);
		fclose(fp2);
		r = fgets(buf2, 255, fp); //"TRACK"の行を読み飛ばす
		r = fgets(buf2, 255, fp);
		if (r != NULL)
			if (strstr(buf2,"PREGAP") != NULL)
				lba += 150; //プリギャップぶんを足す
		_TrackInfo[tn].lba = lba;
		if (strstr(buf,"BINARY") != NULL) //isoの場合
		{
			_TrackInfo[tn].bAudio = FALSE;
			lba += size / 2048;
		}
		else //wavの場合
		{
			_TrackInfo[tn].bAudio = TRUE;
			lba += (size - 44) / 2352; //44=WAVEヘッダぶん
		}
		tn++;
	}
	fclose(fp);

	//リードアウトを設定
	_TrackInfo[tn].bAudio = FALSE;
	_TrackInfo[tn].lba = lba;
	_LastTrack = tn - 1;

	SetCursor(LoadCursor(NULL, IDC_ARROW)); //カーソルを元に戻す

	//PRINTF("TrackInfo Test = %X", _TrackInfo[10].lba);

	if (_LastTrack == 0)
		return FALSE;
	else
		return TRUE;
}


static void
cdb_play(
	BYTE*		cdb,
	CdArg*		pArg)
{
	ZeroMemory(cdb, sizeof(BYTE)*10);

	cdb[0] = SCSI_PLAYAUD_12;

	cdb[2] = (BYTE)(pArg->startLBA >> 24);
	cdb[3] = (BYTE)(pArg->startLBA >> 16);
	cdb[4] = (BYTE)(pArg->startLBA >> 8);
	cdb[5] = (BYTE)pArg->startLBA;

	cdb[6] = (BYTE)(pArg->endLBA >> 24);
	cdb[7] = (BYTE)(pArg->endLBA >> 16);
	cdb[8] = (BYTE)(pArg->endLBA >> 8);
	cdb[9] = (BYTE)pArg->endLBA;
}


static void
cdb_pause(
	BYTE*		cdb,
	CdArg*		pArg)
{
	BYTE		lun = (BYTE)_CdromInfo[_DeviceInUse].lun;

	cdb[0] = 0x4B;
	cdb[1] = (lun << 5) | 1;
	cdb[2] = 0;
	cdb[3] = 0;
	cdb[4] = 0;
	cdb[5] = 0;
	cdb[6] = 0;
	cdb[7] = 0;
	cdb[8] = 0;
	cdb[9] = 0;
}


static void
cdb_resume(
	BYTE*		cdb,
	CdArg*		pArg)
{
	BYTE		lun = (BYTE)_CdromInfo[_DeviceInUse].lun;

	cdb[0] = 0x4B;
	cdb[1] = (lun << 5) | 1;
	cdb[2] = 0;
	cdb[3] = 0;
	cdb[4] = 0;
	cdb[5] = 0;
	cdb[6] = 0;
	cdb[7] = 0;
	cdb[8] = 1;
	cdb[9] = 0;
}


static void
cdb_stop(
	BYTE*		cdb,
	CdArg*		pArg)
{
	BYTE		lun = (BYTE)_CdromInfo[_DeviceInUse].lun;

	cdb[0] = 0x4B;
	cdb[1] = (lun << 5) | 1;
	cdb[2] = 0;
	cdb[3] = 0;
	cdb[4] = 0;
	cdb[5] = 0;
	cdb[6] = 0;
	cdb[7] = 0;
	cdb[8] = 0;
	cdb[9] = 0;
}


static void
cdb_seek(
	BYTE*		cdb,
	CdArg*		pArg)
{
	BYTE		lun = (BYTE)_CdromInfo[_DeviceInUse].lun;

	ZeroMemory(cdb, sizeof(BYTE)*10);

	cdb[0] = 0x2B;
	cdb[1] = lun << 5;
	cdb[2] = (BYTE)(pArg->startLBA >> 24);
	cdb[3] = (BYTE)(pArg->startLBA >> 16);
	cdb[4] = (BYTE)(pArg->startLBA >>  8);
	cdb[5] = (BYTE)(pArg->startLBA      );
}


static BOOL
execute_command(
	BYTE*		cdb,
	CdArg*		pArg)
{
	BYTE		ha  = (BYTE)_CdromInfo[_DeviceInUse].adapter;
	BYTE		tg  = (BYTE)_CdromInfo[_DeviceInUse].target;
	BYTE		lun = (BYTE)_CdromInfo[_DeviceInUse].lun;

	if (execute_scsi_command(NULL, 0, ha, tg, lun, cdb, sizeof(BYTE)*10))
		return TRUE;

	return FALSE;
}


//Kitao更新。一度のアクセスで読み込むセクター数を多くし、高速化した。v1.00
//シャーロックホームズが動く環境が増えた＆負荷も下がったはず。
static BOOL
execute_read(
	BYTE*		cdb,
	CdArg*		pArg)
{
	BYTE	ha  = (BYTE)_CdromInfo[_DeviceInUse].adapter;
	BYTE	tg  = (BYTE)_CdromInfo[_DeviceInUse].target;
	BYTE	lun = (BYTE)_CdromInfo[_DeviceInUse].lun;

	int			i;
	int			nSectors;
	Uint32		lba;

	nSectors = pArg->endLBA - pArg->startLBA;
	lba      = pArg->startLBA;

	for (i=0; i<nSectors; i++)
	{
		ZeroMemory(cdb, sizeof(BYTE)*10);
		cdb[0]     = 0x28;	//   READ10。ドライブの互換性の問題を避けるため、PC2Eと同様にREAD10を使うようにした。v0.69。
		cdb[1]     = (BYTE)(lun << 5);
		cdb[3]     = (BYTE)(lba >> 16);
		cdb[4]     = (BYTE)(lba >> 8);
		cdb[5]     = (BYTE)(lba);
		cdb[8]     = 1; //Kitao参考。ここで複数セクタ読み込もうとすると失敗することがあった。
		
		if (execute_scsi_command(_pReadBuf, 2048, ha, tg, lun, cdb, sizeof(BYTE)*10))
		{
			CopyMemory(pArg->pBuf + i*2048, _pReadBuf, 2048);
			lba++;
		}
		else
		{
			ZeroMemory(pArg->pBuf + i*2048, 2048*(nSectors-i)); //読み込めなかった領域を0で埋めてからリターン。v2.11更新
			return FALSE;
		}
	}

	return TRUE;
}


//Kitao追加。CDデータをインストールしてあった場合用のread処理(CDの代わりにHDDのファイルへアクセス)。
//			 v1.58からこのスレッドで行うようにした(メインスレッドだと大きな読み込み時に、遅かった場合音声ノイズが出たため)。
static BOOL
execute_readHdd(
	BYTE*		cdb,
	CdArg*		pArg)	//pArg->trackにトラックナンバー，pArg->endLBAに読み込むセクター数(nSectors)，pArg->startLBAに読み込み開始アドレスを設定して呼ぶ。
{
	int		nSectors;
	int		track;
	FILE*	fp;
	char	fileName[MAX_PATH+1];

	nSectors = pArg->endLBA;
	track = pArg->track;

	CDROM_SetInstallFileName(track, fileName);
	if ((fp = fopen(fileName, "rb")) != NULL)
	{
		fseek(fp, pArg->startLBA, SEEK_SET);
		fread(pArg->pBuf, nSectors*2048, 1, fp);
		fclose(fp);
		return TRUE;
	}
	else
		return FALSE;
}


//Kitao追加。CD-DA(音楽データ)のセクターを読み込む。v2.32データトラックかどうかの判断はもっと厳密さが必要だったので速度の問題も含めてscsiドライバに任せるようにした。
static BOOL
execute_readCdda(
	BYTE*		cdb,
	CdArg*		pArg)
{
	BYTE	ha  = (BYTE)_CdromInfo[_DeviceInUse].adapter;
	BYTE	tg  = (BYTE)_CdromInfo[_DeviceInUse].target;
	BYTE	lun = (BYTE)_CdromInfo[_DeviceInUse].lun;

	int		i;
	int		nSectors;
	Uint32	lba;

	nSectors = pArg->endLBA - pArg->startLBA;
	lba      = pArg->startLBA;

	for (i=0; i<nSectors; i++)
	{
		ZeroMemory(cdb, sizeof(BYTE)*12);
		cdb[0]     = 0xBE;	//   READ CD LBA
		cdb[1]     = 0x04;	//   CDDA Sector これで読んだ場合、ドライブによっては低速(静音)で読み込んでくれる？
		cdb[2]     = (BYTE)(lba >> 24);
		cdb[3]     = (BYTE)(lba >> 16);
		cdb[4]     = (BYTE)(lba >> 8);
		cdb[5]     = (BYTE)(lba);
		cdb[8]     = 1; //Kitao参考。ここで複数セクタ読み込もうとするとうまくいかなかった。
		cdb[9]     = 0x10;	//   UserData
		
		//PRINTF("READ CD LBA = %X", lba);//test
		if (execute_scsi_command(_pCDDAReadBuf, 2352, ha, tg, lun, cdb, sizeof(BYTE)*12))
		{
			CopyMemory(pArg->pBuf + i*2352, _pCDDAReadBuf, 2352);
			lba++;
		}
		else
		{
			ZeroMemory(pArg->pBuf + i*2352, (nSectors-i)*2352); //読み込めなかった領域を0で埋めてからリターン。ヴァリス４のACT4デモシーンで必要。v2.11更新
			return FALSE;
		}
	}

	return TRUE;
}


//Kitao追加。CDデータをインストールしてあった場合用のCD-DAread処理(CDの代わりにHDDのファイルへアクセス)。
static BOOL
execute_readCddaHdd(
	BYTE*		cdb,
	CdArg*		pArg)	//pArg->trackにトラックナンバー，pArg->endLBAに読み込むセクター数(nSectors)，pArg->startLBAに読み込み開始アドレスを設定して呼ぶ。
{
	int		i;
	int		nSectors;
	FILE*	fp;
	char	fileName[MAX_PATH+1];
	size_t	size;

	nSectors = pArg->endLBA;
	ZeroMemory(pArg->pBuf, nSectors*2352); //読み込めなかった領域は0で埋める。ヴァリス４のACT4デモシーンで必要。v2.26

	CDROM_SetInstallWavFileName(pArg->track, fileName);
	if ((fp = fopen(fileName, "rb")) != NULL)
	{
		fseek(fp, pArg->startLBA, SEEK_SET);
		size = fread(pArg->pBuf, nSectors*2352, 1, fp);
		if (size == 0)
		{	//全領域を読み込めなかった場合、次のトラックが音楽トラックなら、そこから読み出す。
			fseek(fp, pArg->startLBA, SEEK_SET);
			for (i=0; i<nSectors; i++) //現トラックからどこまで読めたかを確かめる
				if (fread(pArg->pBuf + i*2352, 2352, 1, fp) == 0)
					break;
			fclose(fp);
			CDROM_SetInstallWavFileName(pArg->track + 1, fileName);
			if ((fp = fopen(fileName, "rb")) != NULL)
			{
				fseek(fp, 44, SEEK_SET); //44=波形データの先頭
				size = fread(pArg->pBuf + i*2352, (nSectors - i)*2352, 1, fp);
				fclose(fp);
				if (size != 0)
					return TRUE; //全領域を読み込めた
			}
			return FALSE;
			//※３つの短かすぎるトラックがまたがっている場合は恐らく無いケースなので、現状は想定しない。
		}
		else
		{
			fclose(fp);
			return TRUE; //全領域を読み込めた
		}
	}
	else
		return FALSE;
}


static BOOL
execute_subq(
	BYTE*		cdb,
	CdArg*		pArg)
{
	BYTE	subq[20];
	BYTE	ha  = (BYTE)_CdromInfo[_DeviceInUse].adapter;
	BYTE	tg  = (BYTE)_CdromInfo[_DeviceInUse].target;
	BYTE	lun = (BYTE)_CdromInfo[_DeviceInUse].lun;
	BYTE*	pBuf = (BYTE*)pArg->pBuf;

	ZeroMemory(cdb, sizeof(BYTE)*10);
	cdb[0]		= 0x42;
	cdb[1]		= (lun << 5) | 2;
	cdb[2]		= 0x40;				// sub q channel
	cdb[3]		= 0x01;				// current pos info
	cdb[6]		= 0;
	cdb[7]		= 0;
	cdb[8]		= sizeof(subq);		// buffer length

	if (execute_scsi_command(subq, sizeof(subq), ha, tg, lun, cdb, sizeof(BYTE)*10))
	{
//		pBuf[1] = subq[0] >> 4;		// control
		pBuf[1] = subq[7];
		pBuf[2] = subq[6];			// track #
//		pBuf[3] = subq[7];			// index ??
		pBuf[3] = subq[0] >> 4;		// control
		pBuf[4] = subq[13];			// min (track)
		pBuf[5] = subq[14];			// sec (track)
		pBuf[6] = subq[15];			// frame (track)
		pBuf[7] = subq[9];			// min (total)
		pBuf[8] = subq[10];			// sec (total)
		pBuf[9] = subq[11];			// frame (total)

		return TRUE;
	}

	return FALSE;
}


static DWORD WINAPI
cdif_main_thread(
	LPVOID	param)
{
	BYTE		cdb[12]; //Kitao更新
	HANDLE		hEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, CDIF_EVENT);
	CdArg*		pArg = (CdArg*)param;
	BOOL		bSuccess;
	int			retry;

	while (hEvent != NULL)
	{
		WaitForSingleObject(hEvent, INFINITE);

		//PRINTF("Test %X", pArg->command);
		if (pArg->command == CDIF_EXIT)
			break;

		bSuccess = FALSE;
		retry = 1; //エラー時のリトライ回数。実機上でも範囲外のセクターなどにアクセスを試みようとするソフトがあり、その場合は、エラーを返す必要がある。
				   //			             なのでリトライ回数を多くしすぎるとソフトの動作がもたつく恐れがある。レミングスのOPデモなど。
				   //						 現状はリトライしないようにして、動作テスト用に使う。v2.17追加

		while ((!bSuccess)&&(retry > 0)) //Kitao更新。成功するまで最大retry回繰り返せるようにした。v2.17
		{
			switch (pArg->command)
			{
				case CDIF_READ://Kitao更新
					bSuccess = execute_read(cdb, pArg);
					break;

				case CDIF_READHDD://Kitao追加
					bSuccess = execute_readHdd(cdb, pArg);
					break;

				case CDIF_SEEK:  //Audio Seek。Kitao更新。シーク時に初回バッファぶんを読み込むようにした。v2.29
					bSuccess = execute_readCdda(cdb, pArg); //READCDDAと同じ処理。コールバック後の処理が異なる。
					break;

				case CDIF_SEEKHDD: //Kitao追加。音楽トラックデータをインストールしてあった場合用のAudio Seek。シーク時に初回バッファぶんを読み込むようにした。v2.29更新
					bSuccess = execute_readCddaHdd(cdb, pArg); //READCDDAHDDと同じ処理。コールバック後の処理が異なる。
					break;

				case CDIF_PLAYCDDA://Kitao追加。v2.29
					bSuccess = TRUE; //何もしないで帰る。CDIF_PLAYCDDAは、ウェイトを入れるために利用。
					break;

				case CDIF_READCDDA://Kitao追加
				case CDIF_READCDDA2://Kitao追加。ここでの処理はCDIF_READCDDAと共通。コールバック後の処理が異なる。
					bSuccess = execute_readCdda(cdb, pArg);
					break;

				case CDIF_READCDDAHDD://Kitao追加
				case CDIF_READCDDA2HDD://Kitao追加。ここでの処理はCDIF_READCDDAHDDと共通。コールバック後の処理が異なる。
					bSuccess = execute_readCddaHdd(cdb, pArg);
					break;

				case CDIF_SEEKDATA: //Kitao追加。セカンダリバッファに先読みをおこなう。
					bSuccess = execute_read(cdb, pArg);
					//※ここでの処理はCDIF_READと同じだが、コールバック後の処理が異なる。
					break;

				case CDIF_SEEKDATAHDD: //Kitao追加。データをインストールしてあった場合用のシーク(CDの代わりにHDDのファイルへアクセス)。セカンダリバッファに先読みをおこなう。
					bSuccess = execute_readHdd(cdb, pArg);
					break;

				case CDIF_SUBQ:
					bSuccess = execute_subq(cdb, pArg);
					break;

				case CDIF_INSTALL://Kitao追加
					bSuccess = execute_read(cdb, pArg);
					break;

				case CDIF_INSTALLWAV://Kitao追加
					bSuccess = execute_readCdda(cdb, pArg);
					break;
			}

			/* 処理の完了を通知する */
			if (_Callback && pArg->bCallback)
			{
				if (bSuccess)
					_Callback(pArg->command);
				else
				{	//失敗した場合、リトライする。v2.17更新
					//PRINTF("CD-ROM Read Error."); //エラーを起こすことが前提の(エラーを返さなきゃ動かない)ゲームもあるため、エラーメッセージは表示せず。レミングスのOPなど。
					retry--;
					if (retry == 0) //すでに計retreu回トライ(retry-1回リトライ)していたら、あきらめてエラーで戻る。
						_Callback(pArg->command | CDIF_ERROR);
					else
						Sleep(1000); //少しでも悪いPC状況を変化させるため、１秒ウェイトを入れる。これが長いと、エラー前提のソフトでモッサリする可能性がある。
				}
			}
			else
				bSuccess = TRUE; //コールバックしない場合、常に成功としてループを抜ける。v2.17
		}

		_bDeviceBusy = FALSE; //Kitao更新。コールバックも終えてからデバイスビジーを解除。
	}

	CloseHandle(hEvent);
	ExitThread(TRUE);

	return 0;
}


Sint32
CDIF_GetNumDevices()
{
	return _nCdromDevice;
}


BOOL
CDIF_SelectDevice(
	Sint32	deviceNum)
{
	char	path[10];

	if (APP_GetCueFile()) //Cueファイルから起動するモードの場合。v2.24
		return read_toc_cue();

	if ((deviceNum >= 0)&&(deviceNum < _nCdromDevice))
	{
		_DeviceInUse = deviceNum;
		//Kitao更新。ここでSPTIのハンドルを取得しておくようにした(高速化＆安定)
		if (_bUseSpti)
		{
			if (_SPTIfileHandle != INVALID_HANDLE_VALUE)
				CloseHandle(_SPTIfileHandle);
			sprintf(path, "\\\\.\\%c:", (int)(_DriveLetters[_DeviceInUse]));
			_SPTIfileHandle = CreateFile(path,
										GENERIC_READ | GENERIC_WRITE,
										FILE_SHARE_READ,
										NULL,
										OPEN_EXISTING,
										0,
										NULL); 
		}
		return read_toc();
	}

	return FALSE;
}


/*-----------------------------------------------------------------------------
	[Deinit]
		SPTIまたはASPIを終了します。
-----------------------------------------------------------------------------*/
void
CDIF_Deinit()
{
	if (_bInit) //Kitao更新。初期化が完了していたときだけおこなう。v1.04
		waitDeviceBusy();

	if (_hEvent != NULL) //Kitao追加。v1.04
	{
		_CdArg.command = CDIF_EXIT;
		SetEvent(_hEvent);
		
		// スレッドの終了を待つ 
		if (_hThread != INVALID_HANDLE_VALUE)
		{
			WaitForSingleObject(_hThread, INFINITE);
			CloseHandle(_hThread);
			_hThread = INVALID_HANDLE_VALUE;
		}
		
		CloseHandle(_hEvent);
	}

	//Kitao追加
	if (_SPTIfileHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(_SPTIfileHandle);
		_SPTIfileHandle = INVALID_HANDLE_VALUE;
	}

	if (_hWNASPI32 != INVALID_HANDLE_VALUE)
	{
		FreeLibrary((HMODULE)_hWNASPI32);
		_hWNASPI32 = INVALID_HANDLE_VALUE;
	}

	_nAdapters = 0;
	_nCdromDevice = 0;
	_bUseSpti = TRUE;
	ZeroMemory(_DriveLetters, sizeof(_DriveLetters));

	_bDeviceBusy = FALSE;

	_bInit = FALSE; //Kitao追加。v1.04
}


/*-----------------------------------------------------------------------------
	[Init]
		SPTIまたはASPIを初期化します。
-----------------------------------------------------------------------------*/
Sint32 //v2.33更新。-1=エラー。0～=見つかったCD-ROM(DVD,BD)ドライブ数。
CDIF_Init(
	void	(*callback)(Uint32))
{
	int			i;
	char		buf[4*26+2];
	DWORD		dwSupportInfo;

	if (callback == NULL)
		return -1;
	_Callback = callback;
	_bInit = TRUE; //Kitao追加。v1.04。v2.33

	//先頭アドレスが16バイト境界のバッファを用意する。（Kitao更新。ここでバッファを初期設定しておくようにした。高速化）
	_pReadBuf = (BYTE*)(((Uint32)&_ReadBuf[0] + 0xF) & ~0xF);
	_pCDDAReadBuf = (BYTE*)(((Uint32)&_CDDAReadBuf[0] + 0xF) & ~0xF);

	//有効なドライブレターを取得する 
	ZeroMemory(&buf, sizeof(buf));
	GetLogicalDriveStrings(sizeof(buf), buf);

	//Kitao更新。CD-ROMドライブを検索
	for (i=0; buf[i]!=0; i+=4)
		if (GetDriveType(&buf[i]) == DRIVE_CDROM)
		{
			_DriveLetters[_nCdromDevice++] = buf[i];
			if (_nCdromDevice >= 26)
				break;
		}

	//Kitao追加。OSがNT系ならSPTIを使用する。※安定性＆ドライブ名が入れ替わってしまう可能性を避けるため。
	if (APP_GetWindows9x())	//Win95,98,Meなら
	{
		_bUseSpti = FALSE;
		
		// load WNASPI32.DLL
		if ((_hWNASPI32 = LoadLibrary("WNASPI32.DLL")) == 0)
		{
			PRINTF("CDIF_Init: \"WNASPI32.DLL\" not found.");
			return -1;
		}

		// load the address of GetASPI32SupportInfo
		_pfnGetASPI32SupportInfo = (LPGETASPI32SUPPORTINFO)GetProcAddress((HMODULE)_hWNASPI32, "GetASPI32SupportInfo");
		if (_pfnGetASPI32SupportInfo == NULL)
		{
			PRINTF("CDIF_Init: DLL function \"GetASPI32SupportInfo\" not found.");
			return -1;
		}
		// load the address of SendASPI32Command
		_pfnSendASPI32Command = (LPSENDASPI32COMMAND)GetProcAddress((HMODULE)_hWNASPI32, "SendASPI32Command");
		if (_pfnSendASPI32Command == NULL)
		{
			PRINTF("CDIF_Init: DLL function \"SendASPI32Command\" not found.");
			return -1;
		}
		
		//Kitao更新。ASPIでドライブ情報を設定
		dwSupportInfo = _pfnGetASPI32SupportInfo();
		if (HIBYTE(LOWORD(dwSupportInfo)) == SS_COMP)
		{
			_nAdapters = LOBYTE(LOWORD(dwSupportInfo));
			_nCdromDevice = scan_cdrom_devices();
		}
	}
	else //NT系(2000,XP,それ以降も)なら
		_bUseSpti = TRUE;

	// スレッド制御用のイベントを作成する 
	_hEvent = CreateEvent(NULL , FALSE, FALSE , CDIF_EVENT);
	if (_hEvent == NULL)
	{
		CDIF_Deinit();
		return -1;
	}
	// メインスレッドを作成して実行する
	clearCdArg(); //先に初期化するようにした。v1.04
	_hThread = CreateThread(NULL, 0, cdif_main_thread, (LPVOID)&_CdArg, 0, &_dwThreadID);
	if (_hThread == NULL)
	{
		CDIF_Deinit();
		return -1;
	}

	return _nCdromDevice; //v2.33からCD-ROMドライブの総数を返すようにした。１台も利用できるCD-ROMドライブが無い場合でも初期化が無事完了したら0を返す。
}


Sint32
CDIF_GetFirstTrack()
{
	return _FirstTrack;
}


Sint32
CDIF_GetLastTrack()
{
	return _LastTrack;
}


//MSFの値からトラックナンバーを返す。v2.11追加
Uint32
CDIF_GetTrackNumber(
	Uint32	m,
	Uint32	s,
	Uint32	f)
{
	Sint32	track = _FirstTrack;
	Uint32	msf = (m << 16) + (s << 8) + f;
	Uint32	msf2;
	Uint32	lba;
	Uint32	lba2;

	while (track <= _LastTrack+1) //Kitao更新。最終トラックのCD-DAも鳴らせるようにlastTrack+1とした。Linda3で発見
	{
		msf2 = CDIF_GetTrackStartPositionMSF(track) >> 8;
		if (msf < msf2)
		{
			if (_TrackInfo[track].bAudio == FALSE) //データトラックだった場合、プリギャップを考慮する。
			{
				lba = msf2lba(m, s, f) + 150;
				lba2 = msf2lba((msf2 >> 16), (msf2 >> 8) & 0xFF, msf2 & 0xFF) - 150;
				if (lba >= lba2) //ギャップ領域に入っていた場合、次のトラック番号を返す。
					return track;
			}
			return track - 1;
		}
		track++;
	}

	return 0;
}


/*-----------------------------------------------------------------------------
	[GetTrackStartPositionLBA]
		トラックの開始位置を LBA で返します。
-----------------------------------------------------------------------------*/
Uint32
CDIF_GetTrackStartPositionLBA(
	Sint32	track)
{
	// LastTrack+1 にはリードアウトデータが入っているので許可する 
	if (track > _LastTrack+1)
		return 0;

	return _TrackInfo[track].lba;
}


/*-----------------------------------------------------------------------------
	[GetTrackStartPositionMSF]
		トラックの開始位置を MSF で返します。
-----------------------------------------------------------------------------*/
Uint32
CDIF_GetTrackStartPositionMSF(
	Sint32	track)
{
	Uint32	lba;
	Uint8	min;
	Uint8	sec;
	Uint8	frame;
	Uint8	datatrack;

	if (track > _LastTrack+1)
		return 0;
	
	lba   = _TrackInfo[track].lba + 150; //Kitao追記。150…プリギャップ(２秒)
	min   = (Uint8)(lba / 75 / 60);
	sec   = (Uint8)((lba - (Uint32)min * 75 * 60) / 75);
	frame = (Uint8)(lba - ((Uint32)min * 75 * 60) - ((Uint32)sec * 75));

	datatrack = _TrackInfo[track].bAudio ? 0 : 4;

	return (min << 24) + (sec << 16) + (frame << 8) + datatrack;
}


//CD-ROMのRead速度を設定する。Kitao更新v2.50
BOOL
CDIF_SetSpeed(
	Uint32	speed)		// 倍速 
{
	BYTE	cdb[12];
	BYTE	ha  = (BYTE)_CdromInfo[_DeviceInUse].adapter;
	BYTE	tg  = (BYTE)_CdromInfo[_DeviceInUse].target;
	BYTE	lun = (BYTE)_CdromInfo[_DeviceInUse].lun;
	BOOL	bSuccess;

	waitDeviceBusy();
	_bDeviceBusy = TRUE;

	if (speed == 0)
		speed = 0xFFFF; //Kitao更新。0の場合、最高速に設定するようにした。
	else
		speed *= 176;

	ZeroMemory(cdb, sizeof(cdb)); //Kitao追加
	cdb[0] = 0xBB; //Set CD Speed
	cdb[1] = lun << 5;
	cdb[2] = (BYTE)(speed >> 8); //Read Speed
	cdb[3] = (BYTE)speed;		 //
	cdb[4] = 0xFF; //Write Speed
	cdb[5] = 0xFF; //

	bSuccess = execute_scsi_command(NULL, 0, ha, tg, lun, cdb, sizeof(cdb));
	_bDeviceBusy = FALSE;

	return bSuccess;
}


/*-----------------------------------------------------------------------------
	[ReadSector]
		指定のドライブから指定のセクタを読み出します。
-----------------------------------------------------------------------------*/
//Kitao更新。データのReadはセカンダリバッファへ読み込むようにした。v0.80。
BOOL
CDIF_ReadSector(
	Uint8*		pBuf,				// 読み込んだセクタデータの保存先 
	Uint32		sector,				// セクタ番号 
	Uint32		nSectors,			// 読み出すセクタ数 
	BOOL		bCallback)
{
	waitDeviceBusy();
	_bDeviceBusy = TRUE;

	clearCdArg();
	_CdArg.command = CDIF_READ;
	_CdArg.pBuf = pBuf;
	_CdArg.startLBA = sector;
	_CdArg.endLBA   = sector + nSectors;
	_CdArg.bCallback = bCallback;

	SetEvent(_hEvent);

	return TRUE;
}


//Kitao追加。データのReadはセカンダリバッファへ読み込むようにした。CDインストールしてあったとき用。v2.24追加
BOOL
CDIF_ReadSectorHDD(
	Uint8*		pBuf,		// 読み込んだセクタデータの保存先 
	Sint32		track,		// 読み込むトラックナンバー
	Uint32		addr,		// 読み込むアドレス。トラック(＝ファイル)の先頭を0x0000とする。
	Sint32		nSectors,	// 読み出すセクタ数 
	BOOL		bCallback)
{
	waitDeviceBusy();
	_bDeviceBusy = TRUE;

	clearCdArg();
	_CdArg.command = CDIF_READHDD;
	_CdArg.pBuf = pBuf;
	_CdArg.track = track; //トラックナンバーを格納
	_CdArg.startLBA = addr; //アドレスを格納
	_CdArg.endLBA   = nSectors; //セクター数を格納
	_CdArg.bCallback = bCallback;

	SetEvent(_hEvent);

	return TRUE;
}


//Kitao追加。CD-DA初回読み込み用
BOOL
CDIF_ReadCddaSector(
	Uint8*		pBuf,				// 読み込んだセクタデータの保存先 
	Uint32		sector,				// セクタ番号 
	Sint32		nSectors,			// 読み出すセクタ数 
	BOOL		bCallback)
{
	waitDeviceBusy();
	_bDeviceBusy = TRUE;

	clearCdArg();
	_CdArg.command = CDIF_READCDDA;
	_CdArg.pBuf = pBuf;
	_CdArg.startLBA = sector;
	_CdArg.endLBA   = sector + nSectors;
	_CdArg.bCallback = bCallback;

	SetEvent(_hEvent);

	return TRUE;
}

//Kitao追加。CD-DA初回読み込み用。CDインストールしてあったとき用。v2.24追加
BOOL
CDIF_ReadCddaSectorHDD(
	Uint8*		pBuf,		// 読み込んだセクタデータの保存先 
	Sint32		track,		// 読み込むトラックナンバー
	Uint32		addr,		// 読み込むアドレス。トラック(＝ファイル(WAVEヘッダがあるので45バイト目から))の先頭を0x0000とする。
	Sint32		nSectors,	// 読み出すセクタ数 
	BOOL		bCallback)
{
	waitDeviceBusy();
	_bDeviceBusy = TRUE;

	clearCdArg();
	_CdArg.command = CDIF_READCDDAHDD;
	_CdArg.pBuf = pBuf;
	_CdArg.track = track; //トラックナンバーを格納
	_CdArg.startLBA = 44 + addr; //アドレスを格納。※44=WAVEヘッダぶん
	_CdArg.endLBA   = nSectors; //セクター数を格納
	_CdArg.bCallback = bCallback;

	SetEvent(_hEvent);

	return TRUE;
}


//音楽トラックへシークを行う。Kitao更新。初回バッファへのREADも行うようにした。v2.29
BOOL
CDIF_Seek(
	Uint8*		pBuf,		// 読み込んだセクタデータの保存先 
	Sint32		track,		// 読み込むトラックナンバー
	Uint32		sector,		// セクタ番号 
	Sint32		nSectors,	// 読み出すセクタ数 
	BOOL		bCallback)
{
	waitDeviceBusy();
	_bDeviceBusy = TRUE;

	clearCdArg();
	_CdArg.command = CDIF_SEEK;
	_CdArg.pBuf = pBuf;
	_CdArg.track = track; //トラックナンバーを格納。v2.32追加
	_CdArg.startLBA = sector;
	_CdArg.endLBA   = sector + nSectors;
	_CdArg.bCallback = bCallback;

	SetEvent(_hEvent);

	return TRUE;
}

//Kitao追加。CDIF_Seek(音楽トラックへシークを行う)のHDD版。トラックデータをCDインストールしてあった場合にハードディスクからデータを読み込む。v2.29
BOOL
CDIF_SeekHDD(
	Uint8*		pBuf,		// 読み込んだセクタデータの保存先 
	Sint32		track,		// 読み込むトラックナンバー
	Uint32		addr,		// 読み込むアドレス。トラック(＝ファイル(WAVEヘッダがあるので45バイト目から))の先頭を0x0000とする。
	Sint32		nSectors,	// 読み出すセクタ数 
	BOOL		bCallback)
{
	waitDeviceBusy();
	_bDeviceBusy = TRUE;

	clearCdArg();
	_CdArg.command = CDIF_SEEKHDD;
	_CdArg.pBuf = pBuf;
	_CdArg.track = track; //トラックナンバーを格納
	_CdArg.startLBA = 44 + addr; //アドレスを格納。※44=WAVEヘッダぶん
	_CdArg.endLBA   = nSectors; //セクター数を格納
	_CdArg.bCallback = bCallback;

	SetEvent(_hEvent);

	return TRUE;
}

//Kitao追加。再生を開始する。実際には再生の合図を出すだけで何もしない。コールバックにて、実機に近いウェイトを発生させるための処理。v2.29
BOOL
CDIF_PlayCdda(
	BOOL	bCallback)
{
	waitDeviceBusy();
	_bDeviceBusy = TRUE;

	clearCdArg();
	_CdArg.command = CDIF_PLAYCDDA;
	_CdArg.bCallback = bCallback;

	SetEvent(_hEvent);

	return TRUE;
}


//Kitao追加。CD-DA追加読み込み用。ここはAPUスレッド上で呼び出される。
BOOL
CDIF_ReadCddaSector2(
	Uint8*		pBuf,				// 読み込んだセクタデータの保存先 
	Uint32		sector,				// セクタ番号 
	Sint32		nSectors,			// 読み出すセクタ数 
	BOOL		bCallback)
{
	int		i;

	//CDデバイスを使用中の場合終わるまで待つ。APUスレッド上で実行されるのでAPUビジー待ちはしない。
	//※CD-DA音源再生中なので他でCDデバイス使用中のことは無いはずだが、念のためにここでも待つようにしておく。
	i = 0;
	while (_bDeviceBusy && i<40000)
	{
		Sleep(1); //※40秒経過判定のためSleep(0)は駄目。v2.05
		i++;
	}
	_bDeviceBusy = TRUE;

	clearCdArg();
	_CdArg.command = CDIF_READCDDA2;
	_CdArg.pBuf = pBuf;
	_CdArg.startLBA = sector;
	_CdArg.endLBA   = sector + nSectors;
	_CdArg.bCallback = bCallback;

	SetEvent(_hEvent);

	return TRUE;
}

//Kitao追加。CD-DA追加読み込み用。ここはAPUスレッド上で呼び出される。CDインストールしてあったとき用。v2.24追加
BOOL
CDIF_ReadCddaSector2HDD(
	Uint8*		pBuf,		// 読み込んだセクタデータの保存先 
	Sint32		track,		// 読み込むトラックナンバー
	Uint32		addr,		// 読み込むアドレス。トラック(＝ファイル(WAVEヘッダがあるので45バイト目から))の先頭を0x0000とする。
	Sint32		nSectors,	// 読み出すセクタ数 
	BOOL		bCallback)
{
	int		i;

	//CDデバイスを使用中の場合終わるまで待つ。APUスレッド上で実行されるのでAPUビジー待ちはしない。
	//※CD-DA音源再生中なので他でCDデバイス使用中のことは無いはずだが、念のためにここでも待つようにしておく。
	i = 0;
	while (_bDeviceBusy && i<40000)
	{
		Sleep(1); //※40秒経過判定のためSleep(0)は駄目。v2.05
		i++;
	}
	_bDeviceBusy = TRUE;

	clearCdArg();
	_CdArg.command = CDIF_READCDDA2HDD;
	_CdArg.pBuf = pBuf;
	_CdArg.track = track; //トラックナンバーを格納
	_CdArg.startLBA = 44 + addr; //アドレスを格納。※44=WAVEヘッダぶん
	_CdArg.endLBA   = nSectors; //セクター数を格納
	_CdArg.bCallback = bCallback;

	SetEvent(_hEvent);

	return TRUE;
}


/*-----------------------------------------------------------------------------
	[IsDeviceBusy]
		CDROMデバイスがコマンド実行中かどうかを返します。

	return:
		BOOL			TRUE --- device is busy
		BOOL			FALSE -- device is idle
-----------------------------------------------------------------------------*/
BOOL
CDIF_IsDeviceBusy()
{
	return _bDeviceBusy;
}


//Kitao追加。データをReadする直前用。セクターの先読みも行う。
BOOL
CDIF_SeekData(
	Uint8*		pBuf,		// 読み込んだセクタデータの保存先
	Uint32		sector,		// セクタ番号
	Uint32		nSectors,	// 読み出すセクタ数
	BOOL		bCallback)
{
	waitDeviceBusy();
	_bDeviceBusy = TRUE;

	clearCdArg();
	_CdArg.command = CDIF_SEEKDATA;
	_CdArg.pBuf = pBuf;
	_CdArg.startLBA = sector;
	_CdArg.endLBA   = sector + nSectors;
	_CdArg.bCallback = bCallback;

	SetEvent(_hEvent);

	return TRUE;
}

//Kitao追加。CDIF_SeekDataのHDD版。トラックデータをCDインストールしてあった場合にハードディスクからデータを読み込む。
BOOL
CDIF_SeekDataHDD(
	Uint8*		pBuf,		// 読み込んだセクタデータの保存先
	Sint32		track,		// 読み込むトラックナンバー
	Uint32		addr,		// 読み込むアドレス。トラック(＝ファイル)の先頭を0x0000とする。
	Uint32		nSectors,	// 読み出すセクタ数
	BOOL		bCallback)
{
	waitDeviceBusy();
	_bDeviceBusy = TRUE;

	clearCdArg();
	_CdArg.command = CDIF_SEEKDATAHDD;
	_CdArg.pBuf = pBuf;
	_CdArg.track = track; //トラックナンバーを格納
	_CdArg.startLBA = addr; //アドレスを格納
	_CdArg.endLBA   = nSectors; //セクター数を格納
	_CdArg.bCallback = bCallback;

	SetEvent(_hEvent);

	return TRUE;
}

//Kitao追加。ハード的に音楽トラックへシークする。ステートロード時に使用。シングルスレッドで動作。
BOOL
CDIF_SeekCdda(
	Uint8	minStart,
	Uint8	secStart,
	Uint8	frmStart)
{
	BYTE		cdb[10];
	BOOL		bSuccess;

	waitDeviceBusy();
	if (APP_GetCueFile()) //Cueファイルから起動するモードの場合。v2.24
		return TRUE;
	_bDeviceBusy = TRUE;

	clearCdArg();
	_CdArg.startLBA = msf2lba(minStart, secStart, frmStart);

	cdb_seek(cdb, (CdArg*)&_CdArg);
	bSuccess = execute_command(cdb, (CdArg*)&_CdArg);
	if (bSuccess)
		_CdArg.bPlaying = FALSE;
	_bDeviceBusy = FALSE;

	return bSuccess;
}


/*-----------------------------------------------------------------------------
	[ReadSubChannelQ]
		ＣＤ再生中にサブＱチャネルを読み出します。
-----------------------------------------------------------------------------*/
//※現在非使用
BOOL
CDIF_ReadSubChannelQ(
	Uint8*		pBuf,		// 10-byte buffer
	BOOL		bCallback)
{
	waitDeviceBusy();
	_bDeviceBusy = TRUE;

	clearCdArg();
	_CdArg.command  = CDIF_SUBQ;
	_CdArg.pBuf     = pBuf;
	_CdArg.bCallback = bCallback;

	SetEvent(_hEvent);

	return TRUE;
}


//Kitao追加。ISOインストール時のセクターリード処理。
BOOL
CDIF_CDInstall(
	Uint8*		pBuf,		// 読み込んだセクタデータの保存先 
	Uint32		sector,		// セクタ番号 
	Uint32		nSectors,	// 読み出すセクタ数 
	BOOL		bCallback)
{
	waitDeviceBusy();
	_bDeviceBusy = TRUE;

	clearCdArg();
	_CdArg.command = CDIF_INSTALL;
	_CdArg.pBuf = pBuf;
	_CdArg.startLBA = sector;
	_CdArg.endLBA   = sector + nSectors;
	_CdArg.bCallback = bCallback;

	SetEvent(_hEvent);

	return TRUE;
}

//Kitao追加。WAVインストール時のセクターリード処理。
BOOL
CDIF_CDInstallWav(
	Uint8*		pBuf,		// 読み込んだセクタデータの保存先 
	Sint32		track,		// 読み込むトラックナンバー
	Uint32		sector,		// セクタ番号 
	Uint32		nSectors,	// 読み出すセクタ数 
	BOOL		bCallback)
{
	waitDeviceBusy();
	_bDeviceBusy = TRUE;

	clearCdArg();
	_CdArg.command = CDIF_INSTALLWAV;
	_CdArg.pBuf = pBuf;
	_CdArg.track = track; //トラックナンバーを格納。v2.32追加
	_CdArg.startLBA = sector;
	_CdArg.endLBA   = sector + nSectors;
	_CdArg.bCallback = bCallback;

	SetEvent(_hEvent);

	return TRUE;
}


//Kitao追加
Sint32
CDIF_GetDriveLetters(
	int	n)
{
	return _DriveLetters[n];
}


//Kitao追加
Sint32
CDIF_GetDeviceInUse()
{
	return _DeviceInUse;
}

//Kitao追加。v2.31
BOOL
CDIF_GetBadInstalled()
{
	return _bBadInstalled;
}
