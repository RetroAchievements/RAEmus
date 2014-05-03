/******************************************************************************
Ootake
・PSG,ADPCM,CDDAそれぞれにDirectSoundバッファを用意することで、各サウンドのダイ
  ナミックレンジを最大限に取り、音質を向上させた。
・バッファのブロックを４つに分けることで、音の遅延とパソコンへの負荷を軽減させ
  た。
・初回の再生時にはウェイトを入れることで、初回再生に起こるノイズを解消した。
・現状は再生サンプルレートは44.1KHz固定とした。(CD-DA再生時の速度アップのため)
・バッファ書き込み時のメモリロックは起動時と終了時の１回のみ行うようにして高速
  化した。v1.02
・CoInitializeはDirectSoundCreate()を使った場合は不要のようなのでカット。これ
  がS_FALSEを返すことで強制終了していた環境があれば、動くようになるかもしれな
  い。v1.04。COINIT_MULTITHREADEDをメインスレッドで行っていることを考慮して、
  Audioスレッドでも念のためCoInitializeExを行うようにした。v2.19
・クリティカルセクションは必要ない(書き込みが同時に行われるわけではない)ような
  ので、省略し高速化した。v1.09

Copyright(C)2006-2010 Kitao Nakamura.
	改造版・後継版を公開なさるときは必ずソースコードを添付してください。
	その際に事後でかまいませんので、ひとことお知らせいただけると幸いです。
	商的な利用は禁じます。
	あとは「GNU General Public License(一般公衆利用許諾契約書)」に準じます。

*******************************************************************************
	[AudioOut.c]
		オーディオインタフェイスを DirectSound を利用して実装します。

		Implement audio interface using DirectSound.

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
#define DIRECTSOUND_VERSION	0x0900	//Kitao追加。9はパンチと品のバランスよし(どのゲームも無難以上)＆CDDA最良。8はパンチが強い(ソルジャーブレイドは最適だった)。7以下だと音にややパンチが欠ける(特にソルジャーブレイド)。(うちの環境で)

#define _WIN32_DCOM //v2.19追加

#include <stdio.h>
#include <dsound.h>
#include "AudioOut.h"
#include "WinMain.h"
#include "Printf.h"
#include "App.h"

//Kitao更新。ソフトでPSG,ADPCM,CDDAをミックスして音を鳴らした場合、ダイナミックレンジを1/3に下げなくてはいけないので音質が大きく落ちてしまう。そのため各Ch専用のバッファで鳴らすようにした。
static	LPDIRECTSOUND			_pDS	= NULL;
static	LPDIRECTSOUNDBUFFER		_pDSBP	= NULL; //Kitao追加。プライマリ用
static	LPDIRECTSOUNDBUFFER		_pDSB1	= NULL; //Kitao追加。PSG再生用
static	LPDIRECTSOUNDBUFFER		_pDSB2	= NULL; //Kitao追加。ADPCM再生用
static	LPDIRECTSOUNDBUFFER		_pDSB3	= NULL; //Kitao追加。CDDA再生用
static	LPVOID					_LpvPtr1; //Kitao追加。PSG再生用。少しでも速度アップするため配列にせずPSG,ADPCM,CDDA用それぞれの変数を用意。
static	DWORD					_DwBytes1; 
static	LPVOID					_LpvPtr2; //Kitao追加。ADPCM再生用
static	DWORD					_DwBytes2; 
static	LPVOID					_LpvPtr3; //Kitao追加。CDDA再生用
static	DWORD					_DwBytes3; 

//Kitao追加。WAVファイル出力用
static	FILE*					_fpOutputWAV = NULL;
static  Sint32					_OutputWavMode;
static	DWORD					_OutputWavFileSize;
static  BOOL					_bOutputWavExecute = FALSE;
static	Sint32					_OutputWavWaitFinish;

static	LPDIRECTSOUNDNOTIFY		_LpDSN1; //Kitao更新。この構造体もリリースはDeinit時にした。3chぶん用意。
static	LPDIRECTSOUNDNOTIFY		_LpDSN2; //			  v2.36記：3ch独立させてバッファを監視することでそれぞれの音源の発生タイミングがしっかりと合う。
static	LPDIRECTSOUNDNOTIFY		_LpDSN3; //					   1chぶんのイベントだけで監視を済ましたら監視以外のチャンネルではタイミングがうまく取れなくて音質が落ちた。ドラゴンスレイヤー英雄伝説のADPCMドラムがわかりやすい。
static	DSBPOSITIONNOTIFY		_PosNotify1[AOUT_BUFFERRATE+1]; //Kitao追加。PSG再生用
static	DSBPOSITIONNOTIFY		_PosNotify2[AOUT_BUFFERRATE+1]; //Kitao追加。ADPCM再生用
static	DSBPOSITIONNOTIFY		_PosNotify3[AOUT_BUFFERRATE+1]; //Kitao追加。CDDA再生用
static	HANDLE					_hEvent1[AOUT_BUFFERRATE+1]; //Kitao追加。PSG再生用
static	HANDLE					_hEvent2[AOUT_BUFFERRATE+1]; //Kitao追加。ADPCM再生用
static	HANDLE					_hEvent3[AOUT_BUFFERRATE+1]; //Kitao追加。CDDA再生用

static	DWORD				_dwBufSize;
static	HANDLE				_hThread;
static	DWORD				_dwThreadID;

static	volatile BOOL		_bPlay; //v2.05更新。volatileに。
static	volatile BOOL		_bPlayStart; //Kitao追加。初回再生時はバッファ全体ぶんの時間をウェイトする（PSGキューに値が用意される時間を待つ）ようにした。
static	volatile BOOL		_bThreadEnd; //Kitao追加。スレッドを終了したいときTRUEにして知らせる。

static	Sint16*				_pAudioBuf1 = NULL; //Kitao追加。Output時のPSG用
static	Sint16*				_pAudioBuf2 = NULL; //Kitao追加。Output時のADPCM用
static	Sint16*				_pAudioBuf3 = NULL; //Kitao追加。Output時のCDDA用
static	Sint16*				_pAudioBuf0 = NULL; //Kitao追加。Output時の波形合成用
static 	void				(*_pCallBack)(int ch, Sint16* pBuf, Sint32 nSamples) = NULL; //Kitao更新。ch(チャンネルナンバー)を追加

static	BOOL				_bAudioInit = FALSE;


//v2.36更新
static void
buffer_lock_1()
{
	HRESULT		hr;

	hr = _pDSB1->Lock(0, _dwBufSize, &_LpvPtr1, &_DwBytes1, NULL, NULL, 0); //書き込み先のポインタを得る。v2.36更新。第2ブロックの指定をNULLにすることで、第1ブロックのみでロックできる。
	if (hr ==DSERR_BUFFERLOST) //失敗した場合リストアしてからもう一度試みる
	{
		_pDSB1->Restore();
		_pDSB1->Lock(0, _dwBufSize, &_LpvPtr1, &_DwBytes1, NULL, NULL, 0);
	}
}
static void
buffer_lock_2()
{
	HRESULT		hr;

	hr = _pDSB2->Lock(0, _dwBufSize, &_LpvPtr2, &_DwBytes2, NULL, NULL, 0);
	if (hr ==DSERR_BUFFERLOST)
	{
		_pDSB2->Restore();
		_pDSB2->Lock(0, _dwBufSize, &_LpvPtr2, &_DwBytes2, NULL, NULL, 0);
	}
}
static void
buffer_lock_3()
{
	HRESULT		hr;

	hr = _pDSB3->Lock(0, _dwBufSize, &_LpvPtr3, &_DwBytes3, NULL, NULL, 0);
	if (hr ==DSERR_BUFFERLOST)
	{
		_pDSB3->Restore();
		_pDSB3->Lock(0, _dwBufSize, &_LpvPtr3, &_DwBytes3, NULL, NULL, 0);
	}
}
static void
buffer_unlock_1()
{
	_pDSB1->Unlock(_LpvPtr1, _DwBytes1, NULL, NULL);
}
static void
buffer_unlock_2()
{
	_pDSB2->Unlock(_LpvPtr2, _DwBytes2, NULL, NULL);
}
static void
buffer_unlock_3()
{
	_pDSB3->Unlock(_LpvPtr3, _DwBytes3, NULL, NULL);
}

/*-----------------------------------------------------------------------------
	[write_streaming_buffer]
		DirectSoundBuffer に出力する音声データを書き込みます。
-----------------------------------------------------------------------------*/
//Kitao更新。v1.02でロックとアンロックをここでしないようにしたが、PC環境によってはすメモリのアンロックをしない音が再生されないドライバがあるらしい(MSDN談。ユーザーの方からも報告があった)ので、ここでロックとアンロックをする。v2.36
//			 _pAudioBufを廃止し、直接DirectSoundのバッファへ書き込むようにして高速化。v2.36

//Kitao更新。ここはPSG専用。
static inline void
write_streaming_buffer_1(
	DWORD	dwOffset)
{
	buffer_lock_1();
	if (_bPlay && !_bPlayStart) //Kitao追加。初回再生時はバッファ全体ぶんの時間をウェイトする（PSGキューに値が用意される時間を待つ）ようにした。
		_pCallBack(1, (Sint16*)((DWORD)_LpvPtr1 + dwOffset), _DwBytes1/4); //Kitao更新。1ch目を処理 ※/4=「1サンプルが4バイト」の意味
	else
		ZeroMemory((LPVOID)((DWORD)_LpvPtr1 + dwOffset), _DwBytes1);
	buffer_unlock_1();
}

//Kitao追加。ここはADPCM専用。
static inline void
write_streaming_buffer_2(
	DWORD	dwOffset)
{
	buffer_lock_2();
	if (_bPlay && !_bPlayStart)
		_pCallBack(2, (Sint16*)((DWORD)_LpvPtr2 + dwOffset), _DwBytes2/4);
	else
		ZeroMemory((LPVOID)((DWORD)_LpvPtr2 + dwOffset), _DwBytes2);
	buffer_unlock_2();
}

//Kitao更新。ここはCDDA専用。
static inline void
write_streaming_buffer_3(
	DWORD	dwOffset)
{
	buffer_lock_3();
	if (_bPlay && !_bPlayStart)
		_pCallBack(3, (Sint16*)((DWORD)_LpvPtr3 + dwOffset), _DwBytes3/4);
	else
		ZeroMemory((LPVOID)((DWORD)_LpvPtr3 + dwOffset), _DwBytes3);
	buffer_unlock_3();
}

//Kitao追加
static DWORD
nosound_check(
	Sint16*		pAudioBuf)
{
	int		i,j;
	DWORD	d = _dwBufSize;

	i = 0;
	
	if (_OutputWavWaitFinish < 4)
	{
		if (_OutputWavWaitFinish < 3)
		{	//ポーズ復帰後の無音状態が終わるまで待つ
			while (i < (int)_dwBufSize/2) //16ビット単位なので/2
			{
				if (*(pAudioBuf+i) != 0)
					break; //音があればbreak;
				else
					d -= 2;
				i++;
			}
			if (d != 0)
				_OutputWavWaitFinish = 3;
			else
			{	//バッファの最後まで無音だった場合は、無音のときに録音を開始したと推定
				if (++_OutputWavWaitFinish == 3) //誤認を防ぐため、３バッファぶんまで無音かどうか確かめる。※多く確かめすぎても逆に誤認することがある。
					_OutputWavWaitFinish = 4; //次に音が発生したらすぐに録音開始。
				return 0;
			}
		}
		//現在鳴っている曲が終わるまで待つ(_OutputWavWaitFinish == 3)
		while (i < (int)_dwBufSize/2) //16ビット単位なので/2
		{
			if (d < 256) //256…小さすぎると曲中で無音判定になってしまう場合がる。大きすぎると、よく使われるhes用再生プログラムの曲切り替えで無音判定にならない。
				return 0;
			for (j=0; j<256/2; j++) //16ビット単位なので/2
				if	(*(pAudioBuf+i+j) != 0)
					break;
			if (j >= 256/2)
				break; //256バイトが連続して00なら無音と推定しbreak;
			else
				d -= 4; //ステレオなので4バイトずつ進めてチェック
			i++;
			i++;
		}
		if (d != 0)
			_OutputWavWaitFinish = 4;
		else
			return 0;
	}
	
	//音が鳴り始めるまで待つ。鳴り始めていない場合d=0。(_OutputWavWaitFinish == 4)
	while (i < (int)_dwBufSize/2) //16ビット単位なので/2
	{
		if (*(pAudioBuf+i) != 0)
			break; //音があればbreak;
		else
			d -= 2;
		i++;
	}
	if (d != 0)
	{
		PRINTF("Start Output \"%s\".", APP_GetWavFileName());
		_OutputWavWaitFinish = 5; //録音スタート
	}

	return d;
}


/*-----------------------------------------------------------------------------
	[playback_thread]
		サウンドバッファの更新を行なうスレッドです。 
-----------------------------------------------------------------------------*/
static DWORD WINAPI
playback_thread(
	LPVOID	param)
{
	int			i;
	Sint32		a;
	DWORD		d;
	DWORD		n;
	DWORD		dwOffset = 0;

	//CoInitializeEx(NULL, COINIT_MULTITHREADED); //v2.19追加。それぞれのスレッドでCoInitializeExしないと安定しない環境もあるかもしれない。(COINIT_MULTITHREADEDの場合、COMを使うスレッドそれぞれでCoInitializeExすることをMSが推奨)

	while (TRUE)
	{
		if (_bThreadEnd)
			ExitThread(TRUE);

		//PSGチャンネルの notification を待つ 
		n = WaitForMultipleObjects(AOUT_BUFFERRATE+1, _hEvent1, FALSE, INFINITE);
		if (n != WAIT_OBJECT_0 + 0) //stopじゃなければ
		{
			//バッファの分岐点（先頭含む）を通知するイベントが発生した場合の処理 
			if (n == AOUT_BUFFERRATE)
				dwOffset = 0;
			else
				dwOffset = n * _dwBufSize;
			if (_fpOutputWAV != NULL) //Kiao追加。WAVファイルへ出力時
			{
				if (_bPlay && !_bPlayStart)
					_pCallBack(1, _pAudioBuf1, _dwBufSize/4);
				else
					ZeroMemory(_pAudioBuf1, _dwBufSize);
				buffer_lock_1();
				CopyMemory((LPVOID)((DWORD)_LpvPtr1 + dwOffset), (LPBYTE)_pAudioBuf1, _DwBytes1);
				buffer_unlock_1();
			}
			else //通常
				write_streaming_buffer_1(dwOffset); //PSGチャンネル
		}

		if (APP_GetCDGame()) //Huカードゲームの処理を軽くする。v2.36追加
		{
			//ADPCMチャンネルの notification を待つ 
			n = WaitForMultipleObjects(AOUT_BUFFERRATE+1, _hEvent2, FALSE, INFINITE);
			if (n != WAIT_OBJECT_0 + 0) //stopじゃなければ
			{
				//バッファの分岐点（先頭含む）を通知するイベントが発生した場合の処理 
				if (n == AOUT_BUFFERRATE)
					dwOffset = 0;
				else
					dwOffset = n * _dwBufSize;
				if (_fpOutputWAV != NULL) //Kiao追加。WAVファイルへ出力時
				{
					if (_bPlay && !_bPlayStart)
						_pCallBack(2, _pAudioBuf2, _dwBufSize/4);
					else
						ZeroMemory(_pAudioBuf2, _dwBufSize);
					buffer_lock_2();
					CopyMemory((LPVOID)((DWORD)_LpvPtr2 + dwOffset), (LPBYTE)_pAudioBuf2, _DwBytes2);
					buffer_unlock_2();
				}
				else //通常
					write_streaming_buffer_2(dwOffset); //ADPCMチャンネル
			}

			//CDDAチャンネルの notification を待つ 
			n = WaitForMultipleObjects(AOUT_BUFFERRATE+1, _hEvent3, FALSE, INFINITE);
			if (n != WAIT_OBJECT_0 + 0) //stopじゃなければ
			{
				//バッファの分岐点（先頭含む）を通知するイベントが発生した場合の処理 
				if (n == AOUT_BUFFERRATE)
					dwOffset = 0;
				else
					dwOffset = n * _dwBufSize;
				if (_fpOutputWAV != NULL) //Kiao追加。WAVファイルへ出力時
				{
					if (_bPlay && !_bPlayStart)
						_pCallBack(3, _pAudioBuf3, _dwBufSize/4);
					else
						ZeroMemory(_pAudioBuf3, _dwBufSize);
					buffer_lock_3();
					CopyMemory((LPVOID)((DWORD)_LpvPtr3 + dwOffset), (LPBYTE)_pAudioBuf3, _DwBytes3);
					buffer_unlock_3();
				}
				else //通常
					write_streaming_buffer_3(dwOffset); //CDDAチャンネル
			}
		}

		//Kitao追加。WAVファイルへ出力処理
		if (_fpOutputWAV != NULL)
		{
			_bOutputWavExecute = TRUE;//排他処理用
			switch (_OutputWavMode)
			{
				case 1: //PSGチャンネルのみ
					if (_OutputWavWaitFinish < 5) //再生開始時に無音状態の場合、音が鳴るまで書き込まない。
					{
						d = nosound_check(_pAudioBuf1);
						if (d > 0)
							fwrite(_pAudioBuf1+(_dwBufSize-d)/2, d, 1, _fpOutputWAV);
					}
					else
					{
						d = _dwBufSize;
						fwrite(_pAudioBuf1, d, 1, _fpOutputWAV);
					}
					break;
				case 2: //ADPCMチャンネルのみ
					if (_OutputWavWaitFinish < 5) //再生開始時に無音状態の場合、音が鳴るまで書き込まない。
					{
						d = nosound_check(_pAudioBuf2);
						if (d > 0)
							fwrite(_pAudioBuf2+(_dwBufSize-d)/2, d, 1, _fpOutputWAV);
					}
					else
					{
						d = _dwBufSize;
						fwrite(_pAudioBuf2, d, 1, _fpOutputWAV);
					}
					break;
				case 3: //CDDAチャンネルのみ
					if (_OutputWavWaitFinish < 5) //再生開始時に無音状態の場合、音が鳴るまで書き込まない。
					{
						d = nosound_check(_pAudioBuf3);
						if (d > 0)
							fwrite(_pAudioBuf3+(_dwBufSize-d)/2, d, 1, _fpOutputWAV);
					}
					else
					{
						d = _dwBufSize;
						fwrite(_pAudioBuf3, d, 1, _fpOutputWAV);
					}
					break;
				case 12: //PSG+ADPCMチャンネル(ドラゴンスレイヤー英雄伝説などで必須)
					for (i = 0; i < (int)_dwBufSize/2; i++) //16ビット単位なので/2
					{
						a = *(_pAudioBuf1+i) + *(_pAudioBuf2+i);
						if (a < -32768) //サチレーション
						{
							PRINTF("WAV Output Saturation. (Recommend Volume Down)%d", a);
							a = -32768;
						}
						if (a > 32767) //サチレーション
						{
							PRINTF("WAV Output Saturation. (Recommend Volume Down)%d", a);
							a = 32767;
						}
						*(_pAudioBuf0+i) = (Sint16)a;
					}
					if (_OutputWavWaitFinish < 5) //再生開始時に無音状態の場合、音が鳴るまで書き込まない。
					{
						d = nosound_check(_pAudioBuf0);
						if (d > 0)
							fwrite(_pAudioBuf0+(_dwBufSize-d)/2, d, 1, _fpOutputWAV);
					}
					else
					{
						d = _dwBufSize;
						fwrite(_pAudioBuf0, d, 1, _fpOutputWAV);
					}
					break;
				case 123: //PSG+ADPCM+CD全て(各チャンネルの音量を落とさなくちゃいけないので音質は下がる)
					for (i = 0; i < (int)_dwBufSize/2; i++) //16ビット単位なので/2
					{
						a = *(_pAudioBuf1+i) + *(_pAudioBuf2+i) + *(_pAudioBuf3+i);
						if (a < -32768) //サチレーション
						{
							PRINTF("WAV Output Saturation. (Recommend Volume Down)%d", a);
							a = -32768;
						}
						if (a > 32767) //サチレーション
						{
							PRINTF("WAV Output Saturation. (Recommend Volume Down)%d", a);
							a = 32767;
						}
						*(_pAudioBuf0+i) = (Sint16)a;
					}
					if (_OutputWavWaitFinish < 5) //再生開始時に無音状態の場合、音が鳴るまで書き込まない。
					{
						d = nosound_check(_pAudioBuf0);
						if (d > 0)
							fwrite(_pAudioBuf0+(_dwBufSize-d)/2, d, 1, _fpOutputWAV);
					}
					else
					{
						d = _dwBufSize;
						fwrite(_pAudioBuf0, d, 1, _fpOutputWAV);
					}
					break;
				default: //通常はありえないが念のため
					d = _dwBufSize;
					fwrite(_pAudioBuf1, d, 1, _fpOutputWAV);
					break;
			}
			_OutputWavFileSize += d;
			_bOutputWavExecute = FALSE;//排他処理用
			if (_OutputWavFileSize > 0xFFF00000) //バッファオーバーになりそうなら
				APP_OutputWavEnd();
		}

		if ((_bPlay)&&(n == AOUT_BUFFERRATE)) //初回は最終地点まで待ったら、ウェイト解除。
			_bPlayStart = FALSE;
	}

	//CoUninitialize(); //v2.19追加

	return 0;
}


/*-----------------------------------------------------------------------------
	Deinit DirectSound

-----------------------------------------------------------------------------*/
#define SAFE_RELEASE(p)		{ if (p) {(p)->Release(); (p) = NULL;} }
static BOOL
d_deinit() 	//Kitao更新。3chぶんのリソースを開放
{
	int		i; //Kitao追加

	if (!_bAudioInit)
		return FALSE;

	_bPlay = FALSE;

	if (_pDSB1 != NULL)
	{
		// スレッドの終了を待つ 
		_pDSB1->Play(0, 0, DSBPLAY_LOOPING);//もし再生していなかった場合は、_bThreadEndを受け付けないのでここで再生。
		_pDSB2->Play(0, 0, DSBPLAY_LOOPING);
		_pDSB3->Play(0, 0, DSBPLAY_LOOPING);
		_bThreadEnd = TRUE;
		_pDSB1->Stop();
		_pDSB2->Stop();
		_pDSB3->Stop();
		WaitForSingleObject(_hThread, INFINITE);
	}

	CloseHandle(_hThread);

	for (i = 0; i<=AOUT_BUFFERRATE; i++)
	{
		if (_hEvent1[i])	CloseHandle(_hEvent1[i]);
		if (_hEvent2[i])	CloseHandle(_hEvent2[i]);
		if (_hEvent3[i])	CloseHandle(_hEvent3[i]);
		_hEvent1[i] = NULL;
		_hEvent2[i] = NULL;
		_hEvent3[i] = NULL;
	}

	SAFE_RELEASE(_LpDSN1);//Kitao更新
	SAFE_RELEASE(_LpDSN2);
	SAFE_RELEASE(_LpDSN3);

	SAFE_RELEASE(_pDSB1);
	SAFE_RELEASE(_pDSB2);
	SAFE_RELEASE(_pDSB3);
	SAFE_RELEASE(_pDSBP);
	SAFE_RELEASE(_pDS);

	GlobalFree(_pAudioBuf1);
	GlobalFree(_pAudioBuf2);
	GlobalFree(_pAudioBuf3);
	GlobalFree(_pAudioBuf0);
	_pAudioBuf1 = NULL;
	_pAudioBuf2 = NULL;
	_pAudioBuf3 = NULL;
	_pAudioBuf0 = NULL;
	_dwBufSize = 0;

	return TRUE;
}


/*-----------------------------------------------------------------------------
	Initialize DirectSound

-----------------------------------------------------------------------------*/
static BOOL
d_init(
	Sint32	soundType, //Kitao追加。1…通常(ソフトバッファ＆ミキシング。環境にもよるかもしれないがベストな音質)。2…ハードバッファ＆ソフトミキシング(それなりにいい音が鳴る)。
	HWND	hWnd,
	WORD	nChannels,
	WORD	nSamplesPerSec,
	WORD	wBitsPerSample,
	DWORD	dwBufSize)			// in bytes
{
	int 					i;//Kitao追加
	DSBUFFERDESC			dsbd;
	WAVEFORMATEX			waveFormat;//Kitao更新

	// Create IDirectSound 
	if (FAILED(DirectSoundCreate(NULL, &_pDS, NULL)))
	{
		MessageBox(WINMAIN_GetHwnd(), "ERROR: DIRECTSOUND::DirectSoundCreate() failed.    ", "Ootake", MB_OK); //Kitao追加
		return FALSE;
	}

	/*
	** Set coop level to DSSCL_PRIORITY
	**
	** プライマリバッファのフォーマットを設定できるよう、プライマリ協調レベル
	** を設定する。デフォルトのフォーマットに変更を加えない場合、入力の
	** フォーマットにかかわりなく、出力は 8 ビット、22 kHz フォーマットになる。
	** IDirectSoundBuffer::SetFormat の呼び出しが失敗しても問題はない点に
	** 注意する。DirectSound は単純に、利用できる中で最も近いフォーマットに
	** 設定する。
	*/
	if (FAILED(_pDS->SetCooperativeLevel(hWnd, DSSCL_PRIORITY)))
	{
		MessageBox(WINMAIN_GetHwnd(), "ERROR: DIRECTSOUND::SetCooperativeLevel() failed.    ", "Ootake", MB_OK); //Kitao更新
		return FALSE;
	}

	/*
	** Get the primary buffer.
	**
	** プライマリバッファのフォーマットを設定するには、最初に
	** DSBUFFERDESC 構造体でそのフォーマットを記述し、次にその記述を
	** IDirectSound::CreateSoundBuffer メソッドに渡す。 
	*/
	ZeroMemory(&dsbd, sizeof(DSBUFFERDESC));
	dsbd.dwSize			= sizeof(DSBUFFERDESC);
	dsbd.dwFlags		= DSBCAPS_PRIMARYBUFFER;
	dsbd.dwBufferBytes	= 0;
	dsbd.lpwfxFormat	= NULL;

	if (FAILED(_pDS->CreateSoundBuffer(&dsbd, &_pDSBP, NULL))) //Kitao更新。v1.03。プライマリは作成した後すぐには開放しないようにした。
	{
		MessageBox(WINMAIN_GetHwnd(), "ERROR: DIRECTSOUND::CreateSoundBuffer() failed.    ", "Ootake", MB_OK); //Kitao更新
		return FALSE;
	}

	/*
	** Set primary buffer to desired format.
	**
	** プライマリバッファオブジェクトを取得した後で、希望のウェーブ
	** フォーマットを記述し、その記述を IDirectSoundBuffer::SetFormat
	** メソッドに渡す。
	*/
	ZeroMemory(&waveFormat, sizeof(WAVEFORMATEX));
	waveFormat.wFormatTag		= WAVE_FORMAT_PCM;
	waveFormat.nChannels		= nChannels; 
	waveFormat.wBitsPerSample	= wBitsPerSample;
	waveFormat.nBlockAlign		= waveFormat.nChannels * waveFormat.wBitsPerSample / 8;
	waveFormat.nSamplesPerSec	= nSamplesPerSec;
	waveFormat.nAvgBytesPerSec	= waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;

	if (FAILED(_pDSBP->SetFormat(&waveFormat)))
	{
		MessageBox(WINMAIN_GetHwnd(), "ERROR: DIRECTSOUNDBUFFER::SetFormat() failed.    ", "Ootake", MB_OK); //Kitao更新
//		return FALSE;
	}

	// DSBUFFERDESC 構造体を設定する。
	ZeroMemory(&dsbd, sizeof(DSBUFFERDESC));
	dsbd.dwSize	= sizeof(DSBUFFERDESC);
	switch (soundType) //Kitao更新。バッファ設定を選択できるようにした。v1.03。v1.31から２択に絞った。
	{
		case 2:
			//Kitao更新。1の設定のほうがベストと思うが、バッファをDSBCAPS_STATICでハードメモリ上にしたら音は大人しいが音の解像度が上がる感じでこれも良し。
			dsbd.dwFlags = DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GLOBALFOCUS | DSBCAPS_STATIC | DSBCAPS_LOCSOFTWARE;
			break;
		default: //=1
			//Kitao更新。グローバルフォーカスにした。＆ハードウェアミキシングだとノイズが入りやすいのでソフトミキシングに。PCエンジンの内蔵音源はこの設定が一番高音が心に響く感じ。
			dsbd.dwFlags = DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GLOBALFOCUS | DSBCAPS_LOCSOFTWARE;
			break;
	}
	dsbd.dwBufferBytes	= dwBufSize * AOUT_BUFFERRATE; //Kitao更新。AOUT_BUFFERRATE倍ぶん用意する。
	dsbd.lpwfxFormat	= &waveFormat;

	// セカンダリバッファを作成する Kitao更新。3chぶん作成。
	if (FAILED(_pDS->CreateSoundBuffer(&dsbd, &_pDSB1, NULL)))	
	{
		MessageBox(WINMAIN_GetHwnd(), "ERROR: AudioOut: Failed creating secondary buffer1.    ", "Ootake", MB_OK); //Kitao更新
		return FALSE;
	}
	if (FAILED(_pDS->CreateSoundBuffer(&dsbd, &_pDSB2, NULL)))	
	{
		MessageBox(WINMAIN_GetHwnd(), "ERROR: AudioOut: Failed creating secondary buffer2.    ", "Ootake", MB_OK); //Kitao更新
		return FALSE;
	}
	if (FAILED(_pDS->CreateSoundBuffer(&dsbd, &_pDSB3, NULL)))	
	{
		MessageBox(WINMAIN_GetHwnd(), "ERROR: AudioOut: Failed creating secondary buffer3.    ", "Ootake", MB_OK); //Kitao更新
		return FALSE;
	}

	//Kitao更新。3chぶん用意。[0]をstop notificationとし、[1]～[AOUT_BUFFERRATE]までを分岐点イベントとした。
	for (i =0; i<=AOUT_BUFFERRATE; i++)
	{
		_hEvent1[i] = CreateEvent(NULL, FALSE, FALSE, NULL); //v2.36更新。自動リセットするようにした。
		_hEvent2[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
		_hEvent3[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
	}

	// DIRECTSOUNDNOTIFY のインタフェイスを得る 
	if (FAILED(_pDSB1->QueryInterface(IID_IDirectSoundNotify, (void**)&_LpDSN1)))	return FALSE;
	if (FAILED(_pDSB2->QueryInterface(IID_IDirectSoundNotify, (void**)&_LpDSN2)))	return FALSE;
	if (FAILED(_pDSB3->QueryInterface(IID_IDirectSoundNotify, (void**)&_LpDSN3)))	return FALSE;

	// Kitao更新。再生が停止されたときの[0]で処理することにして、[1]～[AOUT_BUFFERRATE]ぶんまでを音の分岐点通知用とした。
	// 再生が停止されたときの notification 用 
	_PosNotify1[0].dwOffset = DSBPN_OFFSETSTOP;
	_PosNotify1[0].hEventNotify = _hEvent1[0];
	_PosNotify2[0].dwOffset = DSBPN_OFFSETSTOP;
	_PosNotify2[0].hEventNotify = _hEvent2[0];
	_PosNotify3[0].dwOffset = DSBPN_OFFSETSTOP;
	_PosNotify3[0].hEventNotify = _hEvent3[0];
	// Kitao更新。バッファの分岐点(先頭含む)通知用 
	for (i = 1; i<=AOUT_BUFFERRATE; i++)
	{
		//_PosNotify1[i].dwOffset = (i-1)*dwBufSize;
		//v2.36更新。各バッファの切れ目よりも手前(半分の位置)でイベントを起こすようにした。反応が遅いドライバでもこれなら確実で、半分の位置なら早すぎる心配もないようだ。
		if (i == 1)
		{
			_PosNotify1[i].dwOffset = AOUT_BUFFERRATE*dwBufSize - dwBufSize/2;
			_PosNotify2[i].dwOffset = AOUT_BUFFERRATE*dwBufSize - dwBufSize/2;
			_PosNotify3[i].dwOffset = AOUT_BUFFERRATE*dwBufSize - dwBufSize/2;
		}
		else
		{
			_PosNotify1[i].dwOffset = (i-1)*dwBufSize - dwBufSize/2;
			_PosNotify2[i].dwOffset = (i-1)*dwBufSize - dwBufSize/2;
			_PosNotify3[i].dwOffset = (i-1)*dwBufSize - dwBufSize/2;
		}
		_PosNotify1[i].hEventNotify = _hEvent1[i];
		_PosNotify2[i].hEventNotify = _hEvent2[i];
		_PosNotify3[i].hEventNotify = _hEvent3[i];
	}

	// notification を設定する 
	if (FAILED(_LpDSN1->SetNotificationPositions(AOUT_BUFFERRATE+1, _PosNotify1)))	return FALSE;
	if (FAILED(_LpDSN2->SetNotificationPositions(AOUT_BUFFERRATE+1, _PosNotify2)))	return FALSE;
	if (FAILED(_LpDSN3->SetNotificationPositions(AOUT_BUFFERRATE+1, _PosNotify3)))	return FALSE;

	// オーディオバッファを確保する 
	_pAudioBuf1 = (Sint16*)GlobalAlloc(GMEM_FIXED, dwBufSize);
	_pAudioBuf2 = (Sint16*)GlobalAlloc(GMEM_FIXED, dwBufSize);
	_pAudioBuf3 = (Sint16*)GlobalAlloc(GMEM_FIXED, dwBufSize);
	_pAudioBuf0 = (Sint16*)GlobalAlloc(GMEM_FIXED, dwBufSize);
	if (_pAudioBuf1 == NULL)
	{
		d_deinit();
		return FALSE;
	}

	//Kitao更新。高速化のため、メモリのロックは最初に１度だけ行うようにした。v1.02
	_dwBufSize = dwBufSize;
	_bPlay = FALSE;

	//スレッドを開始する前にオーディオ初期化完了フラグをたてる。
	//[2004.04.28] fixed
	_bAudioInit = TRUE;

	//スレッドを作成し実行する 
	_bThreadEnd = FALSE;
	_hThread = CreateThread(NULL, 0, playback_thread, NULL, 0, &_dwThreadID);
	if (_hThread == NULL)
	{
		d_deinit();
		_bAudioInit = FALSE;
		return FALSE;
	}

	//スレッドの優先順位を上げる。v2.36追加。効果が無かったためカット
	//OpenThread(THREAD_SET_INFORMATION, TRUE, _dwThreadID);
	//if (SetThreadPriority(_hThread, THREAD_PRIORITY_HIGHEST) == NULL)
	//	return FALSE;
		
	return TRUE;
}


/*-----------------------------------------------------------------------------
	[Init]
		
-----------------------------------------------------------------------------*/
BOOL
AOUT_Init(
	Sint32		soundType,	//Kitao追加
	Uint32		bufSize,	// in samples 
	Uint32		sampleRate,
	void		(*pCallBack)(int ch, Sint16* pBuf, Sint32 nSamples)) //Kitao更新。ch(チャンネルナンバー)を追加
{
	if (d_init(soundType, WINMAIN_GetHwnd(), 2, (WORD)sampleRate, 16, (DWORD)bufSize*2*2))
	{
		_pCallBack = pCallBack;
		return TRUE;
	}

	return FALSE;
}


/*-----------------------------------------------------------------------------
	[Play]
		
-----------------------------------------------------------------------------*/
void
AOUT_Play(
	BOOL	bPlay)
{
	if (!_bAudioInit)
		return;

	_bPlay = bPlay;
	if (_bPlay)
	{
		_pDSB1->Play(0, 0, DSBPLAY_LOOPING);
		_pDSB2->Play(0, 0, DSBPLAY_LOOPING);
		_pDSB3->Play(0, 0, DSBPLAY_LOOPING);
	}
}


/*-----------------------------------------------------------------------------
	[Deinit]
		
-----------------------------------------------------------------------------*/
void
AOUT_Deinit()
{
	if (!_bAudioInit)
		return;

	d_deinit();

	_bAudioInit = FALSE;
}


//Kitao追加
void
AOUT_SetPlayStart()
{
	int i;

	if (!_bAudioInit)
		return;

	// 再生をストップ
	_pDSB1->Stop();
	_pDSB2->Stop();
	_pDSB3->Stop();

	// WAVファイル出力用Bufferに無音を書き込む 
	ZeroMemory(_pAudioBuf1, _dwBufSize);
	ZeroMemory(_pAudioBuf2, _dwBufSize);
	ZeroMemory(_pAudioBuf3, _dwBufSize);

	// ストリーミングバッファを無音で埋める
	for (i = 0; i<AOUT_BUFFERRATE; i++) //Kitao追加
	{
		write_streaming_buffer_1(i*_dwBufSize);
		write_streaming_buffer_2(i*_dwBufSize);
		write_streaming_buffer_3(i*_dwBufSize);
	}

	// 再生位置を0にする
	_pDSB1->SetCurrentPosition(0);
	_pDSB2->SetCurrentPosition(0);
	_pDSB3->SetCurrentPosition(0);
	
	_bPlayStart = TRUE;// 初回再生を表す
}

//Kitao追加
void
AOUT_SetFpOutputWAV(
	FILE*	fp,
	Sint32	mode)
{
	DWORD	t1, t2;

	t1 = timeGetTime();
	t2 = t1 + 10000;//10秒以上待った場合は、メインスレッドでハード的なトラブルが出た可能性が高いと想定し、Sleep(1)へ切り替えてOSの安定に備える。
	while (_bOutputWavExecute) //書き込み処理中だった場合終わるまで待つ
		WINMAIN_SafetySleepZero(t1, t2); //安全にSleep(0)を行う。v2.42更新

	_OutputWavMode = mode;
	if (APP_GetOutputWavNext())
		_OutputWavWaitFinish = 0;
	else
		_OutputWavWaitFinish = 4; //通常モードの場合、待ちが終わったことにする。
	_fpOutputWAV = fp; //この瞬間にWAV出力が開始される
}

//Kitao追加
FILE*
AOUT_GetFpOutputWAV()
{
	return _fpOutputWAV;
}

//Kitao追加
void
AOUT_SetOutputWavFileSize(
	DWORD	size)
{
	_OutputWavFileSize = size;
}

//Kitao追加
DWORD
AOUT_GetOutputWavFileSize()
{
	return _OutputWavFileSize;
}

//Kitao追加
Sint32
AOUT_GetOutputWavWaitFinish()
{
	return _OutputWavWaitFinish;
}

//Kitao追加
Sint32
AOUT_GetPlay()
{
	return _bPlay;
}
