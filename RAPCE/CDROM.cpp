/******************************************************************************
Ootake
・ステートロードをした際に、ステートセーブ時のCD-DAを再生するようにした。
・CD-DA再生をセクターからデータを直接読んでWAVデータとしてDirectSoundで再生する
  ようにした。（フェードアウト・インに未対応のドライブや、仮想ドライブでも問題
  なく動作するようになった）
・ポート0x0DのBit6だけがセットされた場合もADPCMを再生するようにした。（スチーム
  ハーツで確認）
・ポート0x0Fに0xCが書き込まれたときはCD-DAのフェードアウトと共に、ADPCMをフェー
  ドインするようにした。銀河婦警伝説サファイアの２面開始時にADPCMが鳴るようにな
  った。v0.83
・0x180F番地に0x01が書き込まれたときにも、フェードアウト解除(フェードイン)を行
  うようにした。イース３でCD-DAが止まる問題を解消。v0.78。
・最終トラックのCD-DAが再生されないことがあった問題を解消した。
・CDのアクセススピードを実機並みの速度に落とす機能を付けた。(アクセス時間をウェ
  イトに使っているソフトで必要)
・フェードアウト・インの動作を「CdFader.c」からこちらへ移動し統合した。
・６ボタンパッド対応ソフトを起動した場合、自動で６ボタンパッドをつなぐようにし
  た。
・マルチタップ接続時に問題のあるソフトを起動した場合、自動でマルチタップをはず
  すようにした。（超兄貴等）
・現状のエミュレータ実装で動作に問題があるソフトの場合、特別処理をするようにし
  た。(ぽっぷるメイル等)
・同人ソフト等TOCデータベースにないソフトにも対応するため、TOCが合わなくても起
  動するようにした。
・v0.53。CD-DA再生時にトラックの途中から再生するソフトの場合でも、トラックの初
  めから再生されてしまうことがあった不具合を修正した。
・v0.53。CD-DA再生時にプリギャップを考慮するようにした。一部のゲームでCD-DA再
  生箇所がずれていた不具合を解消した。
・v0.59。CD-DA消音時のノイズ軽減のため、消音時はフェードアウトするようにした。
・v0.59。CD-DA再生スタート位置をセットした瞬間に再生も開始するゲーム(レッドア
  ラート,ヴァリスIIなど)に対応した。
・v0.60。CD-DA再生時に再生開始をしない場合だけシークをおこなうようにした。
  (Athlon64x2などの速すぎるパソコンでCD-DAが開始されない不具合を解消)
・v0.61。秒単位ではなくフレーム(バイト)単位で曲の終了を判定するようにした。
  (曲リピート時のタイムラグを解消)
・CD-ROMの読み込みタイミングを実機に近づけた。具体的には、シーク指令があったら
  なるべく早くPCE側に結果を返すようにし、キャッシュを64Kバイトに減らして大きな
  連続読み込み時でもリードエラーが出にくいようにした。ステートセーブの容量も減
  らせた。
・v0.80。シングルスレッドでCD-ROMアクセスをすると、遅いドライブの場合重くなっ
  てBGMが途切れてしまうことがあるので、セカンダリバッファ(64Kバイト)を用意して
  別スレッドで先読みしておくようにした。
・v0.61。シーク完了を返すのが早すぎると問題が起こるソフトのために、シーク時だ
  けウェイトを入れる仕組みを実装した。"ファインティングストリート"で使用
・v0.62。execute_get_dir_info()の不具合を修正（_CmdArgBuffer[2]をINTに変換）し
  ヴァリスIII,IVのオープニングデモで画面が崩れる問題を解消した。
・v0.64。ADPCMメモリへのDMA読み込みをCPU等と並列に処理できるようにし、実機の動
  きに近づけた。
・v0.67。CD-DAのフェードイン＆フェードアウトの時間を、実機に近い聴こえ具合にな
  るように調整した。
・v0.68。CD-DAをポーズ中もbPlayingはTRUEのままにしておくようにした。ポーズ中に
  再生コマンドが来た場合はポーズを解除して再生を続けるようにした。（ウィザード
  リィI・IIのCD-DA←→PSGへ行き来するときの動作が実機と同様になった）
・v0.78。再生中のCD-DA音量を返す部分の実装をした。内蔵のCDプレイヤーソフトで、
  音量メーターが正常に表示されるようになった。
・v0.78。CD-DAの全曲再生時間を返す部分の実装をした。内蔵のCDプレイヤーソフトで
  全曲再生時間が正常に表示されるようになった。
・v0.83。CDシークの際に、FastSeek設定の場合、パソコンのCDシークが終わっていな
  くても、すぐにCDシークが完了したことにしてPCE側に伝えるようにした。こうする
  ことで実機と同じタイミングを再現できる。これで「ぽっぷるメイル」のビジュアル
  シーンが再現できるようになった。
・リセットせずにCDを交換できるようにした。「ＣＤバトル 光の勇者たち」を遊べる
  ようになった。v1.00
・CD-ROMアクセス速度を上げるため、インストール機能を付けた。「シャーロックホー
  ムズ」や「イース４」などが環境に依存せず快適に動くようになったはず。v1.00
・CD-ROMアクセス時の時間経過とCPU消費サイクルを実機に近づけた。「イース４」が
  パッチ処理なしで口パクと音声がピッタリくるようになった。おそらく他の多くのソ
  フトでも音ずれが解消したはず。v1.02
・CD-DA再生の際に、実機並みのタイミングに合わせるため、バッファ秒間(とめきメモ
  リアル，ダブルドラゴンIIやサザンアイズ,ぽっぷるメイルなどに合わせた)は先に音
  を発生させてからPCE側に処理完了を返すようにした。v1.38。v2.32更新
・DMA読み込みの際に、ReadByteCountが0のときは、ReadByteCountがセットされるまで
  待つようにした。「ロードス島戦記１」「コブラ２」「鏡の国のレジェンド」が動く
  ようになった。v1.31
・$180D(ADPCM制御)に書き込まれたデータのbit1が立っていた場合、$180Aへの空読み
  の回数を設定するようにした。ロードス島戦記１のオープニングデモの乱れを解消。
  v1.31
・$1808の読み込み完了時に、_CheckCountAfterReadが0の場合、ポート0はクリアする
  した。雀偵物語３が動くようになった。v1.34
・ADPCM再生用メモリのDMA転送処理で、dataが0x02のときにもWriteAddrをセットする
  ようにした。「天外魔境ZIRIA スーパーCD版」と、「鏡の国のレジェンド」の名前読
  み上げのADPCM再生が動くようになった。v1.38
・CD-DAのplayModeが1のときはDriveBusy状態にしないようにした。「聖夜物語」が動
  くようになった。v1.62

Copyright(C)2006-2011 Kitao Nakamura.
	改造版・後継版を公開なさるときは必ずソースコードを添付してください。
	その際に事後でかまいませんので、ひとことお知らせいただけると幸いです。
	商的な利用は禁じます。
	あとは「GNU General Public License(一般公衆利用許諾契約書)」に準じます。

*******************************************************************************
	[CDROM.c]
		CD-ROM^2 ドライブを記述します。
		Implements the CD-ROM^2 drive hardware.

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

#define DIRECTINPUT_VERSION	0x0800	//Kitao追加。環境にもよるかもしれないが、DirectInput5が軽い。7だとやや遅延あり。スペースハリアーがわかりやすい。

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//#include <unistd.h> //Kitao追加。gcc用
#include "CDROM.h"
#include "TocDB.h"
#include "MainBoard.h"
#include "CDInterface.h" //Kitao更新。AspiCdrom→CDInterfaceへ名称変更
#include "Printf.h"
#include "App.h"
#include "WinMain.h"
#include "Input.h"
#include "Screen.h"
#include "CDInstall.h" //Kitao追加

extern Sint32 gCPU_ClockCount; //v2.00追加。高速化のためカウンタをグローバル化。

#define BCD(A)	((A / 10) * 16 + (A % 10))		// convert INT --> BCD
#define INT(B)	((B / 16) * 10 + (B % 16))		// convert BCD --> INT

//Kitao更新。Port[0]=PCE側からのサイン のパラメータ
#define SIGNAL_BUSY		0x80
#define SIGNAL_REQ		0x40
#define SIGNAL_CD		0x10
#define SIGNAL_IO		0x08

//Kitao更新。Port[3]=ドライブ側からのサイン のパラメータ
#define SIGNAL_ACK		0x40
#define SIGNAL_DONE		0x20
#define SIGNAL_BRAM		0x10
#define SIGNAL_ADPCM2	0x08
#define SIGNAL_ADPCM1	0x04
#define SIGNAL_CDDAVOL	0x02

#define RESET_DELAY		1	//1。Kitao更新。実機よりCPUパワーが取られているようなので、DELAYをなるべく少なくしてアクセスを高速化。0だとスプリガンmkIIがスタートできない。v1.40


typedef struct
{
	Uint8	minStart;
	Uint8	secStart;
	Uint8	frameStart;

	Uint8	minEnd;
	Uint8	secEnd;
	Uint8	frameEnd;

	Uint8	subqMinTrack;
	Uint8	subqSecTrack;
	Uint8	subqFrmTrack;
	Uint8	subqMinTotal;
	Uint8	subqSecTotal;
	Uint8	subqFrmTotal;
	Uint8	subqTrackNum;

	Sint32	elapsedSec; //v0.80から非使用。古いステートセーブデータのために形は残しておく。
	Sint32	playEndSec;
	Sint32	searchMode;
	Sint32	playMode;

	BOOL	bRepeat;
	BOOL	bPlaying;
	BOOL	bPaused;
	BOOL	bSeeking;
	BOOL	bSeekDone;
	BOOL	bInterrupt;
} AudioTrack;


static BOOL			_bBRAMEnabled;
static Uint8		_Port[15];

static DISCINFO		_DiscInfo; //Kitao追加
static Uint8		_OldReadBuffer[2048*256];	// 512KB。現在非使用。v0.60以前のステートセーブを読み込むために残してある。
static Uint8		_Old2ReadBuffer[2048];	// 現在非使用。v0.79以前のステートセーブを読み込むために残してある。
static Uint8		_ReadBuffer[2048*32];	// Kitao更新。一度に多くアクセスした場合のウェイトでリードエラーになってしまうことがあるのでバッファは64KBにし、セカンダリバッファに続きを先読みするようにした。
static Uint8		_ReadBuffer2[2048*32];	// Kitao追加。CDアクセス時に重くならないように、次のセクターを別スレッドで先読みしておくようにした。v0.80。
static volatile BOOL	_bReadBuffer2Full;	// Kitao追加
static BOOL			_bReadSeekDone;	// Kitao追加。※ステートセーブの必要なし
static Sint32		_ReadSectorAddr;	// Kitao追加
static Sint32		_ReadBufferIndex;	// 読み出しバッファのインデックス
static Sint32		_ReadByteCount;		// 読み出せるバイト数  注：ダウンカウンタ
static Sint32		_CheckCountAfterRead;
static Sint32		_ResetDelayCount;
static BOOL			_bAdpcmDma = FALSE; //Kitao追加。ADPCMメモリへDMA読み込み中(アドレスセット待機中も含む)ならTRUE。CPUの動きと並列に進めるようにした。
static BOOL			_bAdpcmDmaStart = FALSE; //Kitao追加。DMA読み込みを開始できたらTRUE。アドレスがセットされるのを待機している間はFALSE。v1.31
static Sint32		_DmaClockCount; //Kitao追加

static Sint32		_InstallRequiredHDD; //Kitao追加
static BOOL			_bInstallBusy; //Kitao追加
static BOOL			_bInstallSuccess; //Kitao追加
static Sint32		_CDInstall; //Kitao追加。0=未インストール，1=データインストール済み。2=フルインストール済み。v1.02
static BOOL			_bWavTrackInstalled; //Kitao追加。CDフルインストールしたwavファイルを演奏中はTRUEになる。v2.24

static Sint32		_ReadCddaFirst = 1; //各曲最初にCDIF_READCDDAをおこなうときは1。追加読み込み時に1になっていた場合(次の曲が再生開始された)は読み込まないようにするため必要。
static Sint32		_CDDAReadBufferSizeSector;
static Sint32		_CDDAReadBufferSizeSector2;
static Sint32		_CDDAReadBufferSize;
static Sint32		_CDDAReadBufferSize2;
static Uint8		_OldCDDAReadBuffer[2352*128]; //Kitao追加。現在非使用。v0.99以前のステートセーブを読み込むために残してある。
static Uint8		_CDDAReadBuffer[2352*512];//Kitao追加。CDDA読み込みバッファ。CDアクセス速度が充分な速度になって、値を大きくしても遅れなくなったため、v1.00から512固定にした。読み落としの乱れがなくなると音質はだいぶ上がる。
static volatile Sint32	_CDDAReadBufferIndex = 0; //読み出しバッファのインデックス //Kitao追加
static volatile Sint32	_CDDAReadByteCount = 0;	//読み出しすバイト数。再生が始まったらカウンタとして使う。0になったら再生終了 //Kitao追加
static volatile Sint32	_CDDASectorAddr; //読み出すCDDAのセクターアドレス //Kitao追加
static volatile Sint32	_CDDASectorEndAddr; //読み出すCDDAの最終セクターアドレス //Kitao追加
static Sint32			_PrevCDDASectorAddr;	//今回シークした位置を記憶しておく。次回同じ位置でシークした場合、処理をはしょって高速化(実機も同様の動き)。v2.29
static Sint32			_PrevCDDASectorEndAddr; //
static Sint32			_CDDAAjustCount = 0; //CDDAの再生スピード微調整用のカウンタ。v2.32
static volatile BOOL	_bCDDASeeking = FALSE; //Kitao追加。音楽トラックのシーク処理中はTRUE。v2.29
static volatile Sint32	_FadeOut1; //Kitao追加。ノイズ軽減のため、消音時はフェードアウトで消音する。Leftチャンネル用。
static volatile Sint32	_FadeOut2; //Kitao追加。Rightチャンネル用。
static volatile BOOL	_bReadCdda2 = FALSE; //Kitao追加。CDDAのバッファ追加アクセス中はTRUEになる。

static BOOL			_bFastCD; //Kitao追加。CDアクセスにウェイトを入れないならTRUE
static BOOL			_bFastSeek; //0.82追加。CDデータアクセスのシーク時に、高速でシークを完了させるならTRUE。ゲームによっては音ずれの原因になる。
static BOOL			_bAccessWait; //v0.86追加。TRUEの場合、FastCDがオンでも、シーク＆データリード時間分だけはウェイトを入れる。
static BOOL			_bSeekWait; //v0.61追加。TRUEの場合、FastCDがオンでも、シーク時間ぶんだけはウェイトを入れる。
static BOOL			_bSeekWait2; //v2.05追加。_bSeekWaitより少ないウェイトを入れる。これをTRUEにするときは_bSeekWaitもTRUEにすること。
static BOOL			_bSeekWait3; //v2.34追加。_bSeekWait2より少ないウェイトを入れる。これをTRUEにするときは_bSeekWaitもTRUEにすること。
static BOOL			_bReadWait; //v2.03追加。TRUEの場合、FastCDがオンでも、データリード時間ぶんだけはウェイトを入れる。
static Sint32		_ReadWaitCount; //v2.40追加。データリード時のウェイト量。
static BOOL			_bPlaySeekWait; //v0.72追加。TRUEの場合、FastCDがオンでも、曲再生時のシーク時間分だけはウェイトを入れる。
static BOOL			_bPlaySeekWait2; //v2.35追加。TRUEの場合、FastCDがオンでも、曲再生時の連続シーク時の時間分だけはウェイトを入れる。
static BOOL			_bFastAdpcmDma; //v2.16追加。ADPCMのDMA転送開始までのウェイトを速めにする場合TRUE。CDの内周外周シミュレーションをしない限り、実機と同じタイミングの完全な再現は無理なのでこれを利用する。
static Sint32		_CDAccessCount; //Kitao追加。CDアクセスウェイト用カウンタ
static BOOL			_bResetCDAccessCount = FALSE; //Kitao追加。v1.02

//Kitao追加。CDアクセス速度に依存するゲームは個別処理で対応する。
//			 常時FastCDを切って遅いアクセス設定で使っている場合は、下記の大部分の個別処理は必要ないが、
//			 通常は快適なアクセスでのプレイが理想なので、遅いアクセスを必要とするゲームだけを個別処理する。
static BOOL			_bSherlock; //Kitao追加。シャーロックホームズ用パッチ
static Sint32		_PopfulMail; //Kitao追加。ぽっぷるメイル用パッチ
static BOOL			_bDoubleDragon; //Kitao追加。ダブルドラゴンII用パッチ
static BOOL			_bGradiusII; //Kitao追加。グラディウスII用パッチ
static Sint32		_GradiusIIwait; //Kitao追加
static BOOL			_bNekketsu; //Kitao追加。ダウンタウン熱血物語用パッチ
static BOOL			_bDarius; //Kitao追加。スーパーダライアス用パッチ
static BOOL			_bBaseballer; //Kitao追加。熱血レジェンドベースボーラー用パッチ
static BOOL			_bAya; //Kitao追加。AYA用パッチ
static BOOL			_bOrgel; //Kitao追加。オルゴール用パッチ
static BOOL			_bYs4; //Kitao追加。イース４用パッチ
static BOOL			_bLime; //Kitao追加。パステルライム用パッチ

//Kitao追加。CdFader.cを廃止してこちらへ統合した。（CDDA再生部の速度アップ＋フェード状況のステートセーブのため）
static Sint32		_InitialCdVolume;
static Sint32		_CurrentCdVolume;
static Sint32		_VolumeStep;
static BOOL			_bFadeIn  = FALSE;
static BOOL			_bFadeOut = FALSE;
static double		_CdVolumeEffect = 0.0;//Kitao追加
static double		_VOL = 0.0;//Kitao追加。v1.08

static Sint32		_FadeClockCount = 0;
static Sint32		_FadeCycle;
//

// コマンドバッファ等 
static Sint32		_Command;
static Sint32		_ArgsLeft;
static Uint8		_CmdArgBuffer[10];
static Sint32		_CmdArgBufferIndex;

static BOOL			_bCommandReset = TRUE;
static BOOL			_bCommandReceived = FALSE;
static BOOL			_bCommandDone = FALSE;
static BOOL			_bDriveBusy = FALSE;
static BOOL			_bError = FALSE;		// set when command execution fail

static volatile AudioTrack	_AudioTrack; //v2.05更新。volatileに。
static Sint32		_ClockCountSub = 0; //Kitao追加
static Sint32		_ClockCount = 0; //v0.80から非使用。v0.79以前のステートセーブを読み込むために残してある。
static BOOL			_bCdromInit = FALSE;
static Sint32		_PlayTrackNum = 0; //Kitao追加。v2.33更新。現在演奏中のトラックではなく、演奏を開始したトラックなので_PlayTrackNumとした。
static Sint32		_CddaIntroCount = 0; //Kitao追加
static Sint8		_SubqMinTrack; //Kitao追加
static Sint8		_SubqSecTrack; //Kitao追加
static Sint8		_SubqFrmTrack; //Kitao追加

static BOOL			_bAutoDisconnectMultiTap = FALSE;//Kitao追加
static BOOL			_bAutoF1NoReset = FALSE; //Kitao追加。v1.00
static Sint32		_AutoSpriteLimit = 0; //Kitao追加
static Sint32		_AutoNonSpriteLimit = 0; //Kitao追加
static Sint32		_AutoFastCD = 0; //Kitao追加
static Sint32		_AutoSlowCD = 0; //Kitao追加
static Sint32		_AutoFastSeek = 0; //Kitao追加
static Sint32		_AutoNormalSeek = 0; //Kitao追加
static Sint32		_AutoSlowAccess = 0; //Kitao追加
static Sint32		_AutoRetAccess = 0; //Kitao追加
static Sint32		_AutoSlowSeekWait = 0; //Kitao追加
static Sint32		_AutoRetSeekWait = 0; //Kitao追加
static Sint32		_AutoSlowReadWait = 0; //Kitao追加
static Sint32		_AutoRetReadWait = 0; //Kitao追加
static Sint32		_AutoSlowPlaySeekWait = 0; //Kitao追加
static Sint32		_AutoRetPlaySeekWait = 0; //Kitao追加
static Sint32		_AutoRasterTiming = 0; //Kitao追加
static Sint32		_AutoSetRasterTiming = 0; //Kitao追加
static Sint32		_AutoRetRasterTiming = 0; //Kitao追加
static Sint32		_AutoRealStretched = -1; //Kitao追加。自動でストレッチさせるばあいは、0以上の値にする。その値は、変更前のStartStretchModeとする。v2.22，v2.64


static BOOL
check_disc_toc()
{
	int		i;
	int		lastTrack = CDIF_GetLastTrack();
	Uint32	msft;
	Uint32	lba;
	const 	DISCINFO*	pDI;

	memset(&_DiscInfo, 0, sizeof(DISCINFO));

	//Kitao更新。i=1に１トラック目のデータを入れる。lastTrack+1はリードアウト。
	for (i = 1; i <= lastTrack+1; i++)
	{
		msft = CDIF_GetTrackStartPositionMSF(i);
		lba  = CDIF_GetTrackStartPositionLBA(i);

		if (i != lastTrack+1)
		{
			_DiscInfo.TOC[i].trackNum = i;
			_DiscInfo.TOC[i].isAudio = ((msft & 0xFF) == 0);
		}

		_DiscInfo.TOC[i].min   = (Uint8)((msft >> 24) & 0xFF);
		_DiscInfo.TOC[i].sec   = (Uint8)((msft >> 16) & 0xFF);
		_DiscInfo.TOC[i].frame = (Uint8)((msft >>  8) & 0xFF);

		_DiscInfo.TOC[i].lbaH  = (Uint8)((lba >> 16) & 0xFF);
		_DiscInfo.TOC[i].lbaM  = (Uint8)((lba >>  8) & 0xFF);
		_DiscInfo.TOC[i].lbaL  = (Uint8)((lba      ) & 0xFF);
	}

	pDI = TOCDB_IsMatch(&_DiscInfo);

	return (pDI != NULL);
}


//Kitao追加。自動で、各ゲーム毎に一番遊びやすい設定にする。
static void
check_game()
{
	char*	pGameTitle;
	char	cdMessage[256];

	//自動でCDアクセスを速く(または遅く)する設定にしていた場合は、デフォルト設定に戻す。
	if ((_AutoFastCD != 0)||(_AutoSlowCD != 0))
	{
		_bFastCD = APP_GetDefaultFastCD(); //CDアクセス速度をデフォルトに戻す。
		_AutoFastCD = 0;
		_AutoSlowCD = 0;
	}
	//自動でCDシークを速くしていた場合は、デフォルト設定に戻す。
	if ((_AutoFastSeek != 0)||(_AutoNormalSeek != 0))
	{
		_bFastSeek = APP_GetDefaultFastSeek(); //CDシーク速度をデフォルトに戻す。
		_AutoFastSeek = 0;
		_AutoNormalSeek = 0;
	}
	_bSeekWait = FALSE;
	_bSeekWait2 = FALSE;
	_bSeekWait3 = FALSE;
	_bReadWait = FALSE;
	_ReadWaitCount = 23; //23。_bReadWaitオン時や_bFastSeekオフ時のウェイト。真怨霊戦記OK。実機では46サイクル前後掛かると思われるが、シーク状況により誤差があるので、通常は少な目のウェイトにしておく。
	_bAccessWait = FALSE;
	_bPlaySeekWait = FALSE;
	_bPlaySeekWait2 = FALSE;
	_bFastAdpcmDma = FALSE;
	_AutoSlowAccess = 0;
	_AutoRetAccess = 0;
	_AutoSlowSeekWait = 0;
	_AutoRetSeekWait = 0;
	_AutoSlowReadWait = 0;
	_AutoRetReadWait = 0;
	_AutoSlowPlaySeekWait = 0;
	_AutoRetPlaySeekWait = 0;
	_PlayTrackNum = 0;
	//ラスタータイミング切り替えパッチを解除
	_AutoRasterTiming = 0;
	//スプライト欠け再現パッチを解除
	_AutoNonSpriteLimit = 0;
	//シャーロックホームズ用パッチを解除
	_bSherlock = FALSE;
	//ぽっぷるメイル用パッチを解除
	_PopfulMail = 0;
	//ダブルドラゴンII用パッチを解除
	_bDoubleDragon = FALSE;
	//グラディウスII用パッチを解除
	_bGradiusII = FALSE;
	//ダウンタウン熱血物語用パッチを解除
	_bNekketsu = FALSE;
	//スーパーダライアス用パッチを解除
	_bDarius = FALSE;
	//熱血レジェンドベースボーラー用パッチを解除
	_bBaseballer = FALSE;
	//AYA用パッチを解除
	_bAya = FALSE;
	//AYA用パッチを解除
	_bOrgel = FALSE;
	//イース４用パッチを解除
	_bYs4 = FALSE;
	//パステルライム用パッチを解除
	_bLime = FALSE;
	
	//自動でマルチタップを無効にしていた場合
	_bAutoDisconnectMultiTap = FALSE; //MainBoard.c でマルチタップは既に有効済みなので、ここではフラグのクリアだけ。
	//自動でF1キーをノーリセット(ＣＤ入れ替えのみ)にしていた場合は、デフォルトに戻す。
	if (_bAutoF1NoReset)
	{
		APP_SetF1NoReset(FALSE);
		_bAutoF1NoReset = FALSE;
	}
	//自動でリアルストレッチへ変更していた場合
	if (_AutoRealStretched != -1)
	{
		APP_SetStartStretchMode(_AutoRealStretched); //元のストレッチモードへ戻す。v2.64
		_AutoRealStretched = -1;
	}
	
	//※前ゲームで自動で設定したOverClock設定，３ボタンパッド設定は、APP.c のrestore_setting()で、すでに元に戻し済み。
	
	//Kitao追加。各ゲームごとのパッチ当て
	if (APP_GetCDGame())
	{
		//CDGameを遊んでいてF1キーでリセットした or Huカードを遊んでいてCDに変えたときのための処理（各パッチを解除する）。
		MAINBOARD_SetSuperGrafx(0); //スーパーグラフィックスモードを解除。
		VDC_Init(); //ラスタタイミングをデフォルトに戻す。
					//自動で不具合対策のためのオーバークロックをしていた場合は、元の速度に戻す。
					//スプライト欠け再現をデフォルトに戻す。
					//ウェイトパッチをデフォルト(無効)に戻す。
					//強制ラスタ割り込み設定をデフォルト(無効)に戻す。
					//強制VBlank割り込み設定をデフォルト(無効)に戻す。
					//真女神転生用パッチを無効に戻す。
		
		pGameTitle = TOCDB_GetGameTitle();
		
		switch (_CDInstall)
		{
			case 1: //データトラックインストール済み
				sprintf(cdMessage, "Icd: %s", pGameTitle);
				break;
			case 2: //データ＆音楽トラックフルインストール済み
				if (CDIF_GetBadInstalled())
					sprintf(cdMessage, "Bcd: %s", pGameTitle); //古いOotakeでリッピング失敗のイメージファイルの場合。v2.31追加
				else
					sprintf(cdMessage, "Fcd: %s", pGameTitle);
				break;
			default:
				sprintf(cdMessage, "CD: %s", pGameTitle);
				break;
		}
		if ((!APP_GetFullScreen())&&(APP_GetHideMessage()))
			PRINTF("%s - %s", APP_GetAppName(), cdMessage);
		else
			PRINTF("%s", cdMessage);
		
		//起動から終了まで確実にマルチタップを無効にしなければならないゲーム
		if ((strcmp(pGameTitle,"Linda Cube (J)") == 0)|| //リンダキューブ。タイトル画面で必要。
			(strcmp(pGameTitle,"Linda Cube [first] (J)") == 0)|| //リンダキューブ初回出荷(バグあり)版
			(strcmp(pGameTitle,"Emerald Dragon (J)") == 0)|| //エメラルドドラゴン。MB128のユーティリティを使うのに必要。
			(strcmp(pGameTitle,"Shin Megami Tensei (J)") == 0)|| //真女神転生。マルチタップでMB128を誤認識してしまう不具合(実機でも起こる)に対応。
			(strcmp(pGameTitle,"Bazaar de Gozahru no Game de Gozahru (J)") == 0)|| //バザールでござーる。外さないとリモコン操作が効かない。v2.08
			(strcmp(pGameTitle,"Ginga Ojousama Densetsu Yuna II (J)") == 0)) //銀河お嬢様伝説ユナ２。タイトル画面で必要。
		{
				JOYPAD_ConnectMultiTap(FALSE);//マルチタップを無効に。
				_bAutoDisconnectMultiTap = TRUE;//過去バージョンのステートセーブを読み込んだ場合も無効をキープし続けるために必要。
		}
		
		//デフォルトでマルチタップを無効にしたほうがいいゲーム
		if ((strcmp(pGameTitle,"Chou Aniki (J)") == 0)|| //超兄貴。オプションの弾が出るように。
			(strcmp(pGameTitle,"Card Angels (J)") == 0)) //カードエンジェルス。男の裏技が使えるように。
				JOYPAD_ConnectMultiTap(FALSE);//マルチタップを無効に。

		//MB128を無効にしなければならないゲーム
		if (strcmp(pGameTitle,"Magicoal (J)") == 0) //マジクール。現状はロード時にフリーズするのでMB128を外す。修正予定。v2.17
			JOYPAD_ConnectMB128(FALSE);//MB128を無効に。
		
		if ((strcmp(pGameTitle,"Advanced Variable Geo (J)") == 0)|| //アドヴァンストV.G.
			(strcmp(pGameTitle,"Emerald Dragon (J)") == 0)|| //エメラルドドラゴン
			(strcmp(pGameTitle,"Kakutou Haou Densetsu Algunos (J)") == 0)|| //格闘覇王伝説アルガノス
			(strcmp(pGameTitle,"Kabuki Ittouryoudan (J)") == 0)|| //天外魔境カブキ一刀涼談
			(strcmp(pGameTitle,"Garou Densetsu II (J)") == 0)|| //餓狼伝説２
			(strcmp(pGameTitle,"Garou Densetsu II - Sample Disc (J)") == 0)|| //餓狼伝説２サンプルディスク
			(strcmp(pGameTitle,"Garou Densetsu Special (J)") == 0)|| //餓狼伝説SPECIAL
			(strcmp(pGameTitle,"Ginga Ojousama Densetsu Yuna II (J)") == 0)|| //銀河お嬢様伝説ユナ２
			(strcmp(pGameTitle,"Super Real Mahjong P II & III Custom (J)") == 0)|| //スーパーリアル麻雀P-II/III カスタム
			(strcmp(pGameTitle,"Super Real Mahjong P V Custom (J)") == 0)|| //スーパーリアル麻雀P-V カスタム
			(strcmp(pGameTitle,"FirePro Jyoshi - Dome ChoujouKessen (J)") == 0)|| //ファイプロ女子憧夢超女対戦
			(strcmp(pGameTitle,"Flash Hiders (J)") == 0)|| //フラッシュハイダース
			(strcmp(pGameTitle,"Princess Maker II (J)") == 0)|| //プリンセスメーカー２
			//(strcmp(pGameTitle,"Martial Champions (J)") == 0)|| //マーシャルチャンピオン ※現状は６ボタンにするとI,II,RUNボタンが効かなくなる不具合があり。３，２ボタンパッドで支障が無いので、６ボタンをつながないようにした。v0.98
			(strcmp(pGameTitle,"Mahjong Sword (J)") == 0)|| //マージャンソード・プリンセスクエスト外伝
			(strcmp(pGameTitle,"Ryuuko no Ken (J)") == 0)|| //龍虎の拳
			(strcmp(pGameTitle,"Linda Cube (J)") == 0)|| //リンダキューブ
			(strcmp(pGameTitle,"Linda Cube [first] (J)") == 0)|| //リンダキューブ初回出荷(バグあり)版
			(strcmp(pGameTitle,"World Heroes II (J)") == 0)) //ワールドヒーローズ２
			{
				JOYPAD_UseSixButton(TRUE);//自動で６ボタンパッド設定にする。
				PRINTF("Connected 6-button pad.  %s", cdMessage);
			}
		
		if ((strcmp(pGameTitle,"Martial Champions (J)") == 0)|| //マーシャルチャンピオン
			(strcmp(pGameTitle,"Card Angels (J)") == 0)) //カードエンジェルス
			{
				if (!JOYPAD_GetConnectThreeButton()) //起動時２ボタンパッドをつないでいた場合だけ
				{
					JOYPAD_UseThreeButton(TRUE);//自動で３ボタンパッド設定にする。
					PRINTF("Connected 3-button pad.  %s", cdMessage);
				}
			}
		
		if ((strcmp(pGameTitle,"Forgotten Worlds (J)") == 0)|| //フォゴットンワールド
			(strcmp(pGameTitle,"Forgotten Worlds (U)") == 0)) //フォゴットンワールド(U)
			{
				JOYPAD_SetSwapSelRun(TRUE); //SELボタンとRUNボタンを入れ替える
				if (!JOYPAD_GetConnectThreeButton()) //起動時２ボタンパッドをつないでいた場合
				{
					JOYPAD_UseThreeButton(TRUE);//自動で３ボタンパッド設定にする。
					PRINTF("Connected 3-button pad.  %s", cdMessage);
				}
				else //最初から３ボタンパッドをつないでいた場合
					PRINTF("Swapped Select&Run Buttons.  %s", cdMessage);
			}
		
		if ((strcmp(pGameTitle,"A. III (J)") == 0)|| //Ａ列車で行こうIII
			(strcmp(pGameTitle,"The Atlas (J)") == 0)|| //THE ATLAS
			(strcmp(pGameTitle,"Eikan ha Kimini (J)") == 0)|| //栄冠は君に
			(strcmp(pGameTitle,"Tokimeki Memorial (J)") == 0)|| //ときめきメモリアル
			(strcmp(pGameTitle,"Vasteel II (J)") == 0)|| //バスティール２
			(strcmp(pGameTitle,"Power Golf 2 - Golfer (J)") == 0)|| //パワーゴルフ２
			(strcmp(pGameTitle,"Hatsukoi Monogatari (J)") == 0)|| //初恋物語
			(strcmp(pGameTitle,"Metal Angel (J)") == 0)|| //メタルエンジェル
			(strcmp(pGameTitle,"Nemurenuyoru no Chiisana Ohanashi (J)") == 0)|| //原由子の「眠れる夜の小さなお話」
			(strcmp(pGameTitle,"Doukyuusei (J)") == 0)) //同級生
			{
				JOYPAD_ConnectMultiTap(FALSE);
				JOYPAD_ConnectMouse(TRUE);
				PRINTF("Mouse [ WheelDown = RUN ].  %s", cdMessage);
			}//自動でマウスをつなげる。※ブランディッシュ、プリンセスメーカー２、雀偵物語３はパッドがやりやすいと思うのでそのままで
		if (strcmp(pGameTitle,"1552 Tenka Tairan (J)") == 0) //1552天下大乱
		{
			JOYPAD_ConnectMultiTap(FALSE);
			JOYPAD_ConnectMouse(TRUE);
			PRINTF("Mouse [ Do WheelUp(SELECT) after started. ].  %s", cdMessage);
		}//自動でマウスをつなげる。マウス操作への切り替え(SELECTボタン)操作の説明。
		if (strcmp(pGameTitle,"Lemmings (J)") == 0) //レミングス
		{
			JOYPAD_ConnectMultiTap(FALSE);
			JOYPAD_ConnectMouse(TRUE);
			JOYPAD_SetSwapIandII(TRUE);
			PRINTF("Mouse [ WheelDown = RUN ].  Swapped I & II Buttons.  %s", cdMessage);
		}//自動でマウスをつなげる。IボタンとIIボタンを入れ替える。
		
		//ラスタ割り込みのタイミングを設定(より綺麗に動かすため)
		if (strcmp(pGameTitle,"Super Darius II (J)") == 0) //スーパーダライアスII(ハイスコア表示時)
				VDC_SetAutoRasterTiming(5); //MORE LATE
		if ((strcmp(pGameTitle,"Magicoal (J)") == 0)|| //マジクール
			(strcmp(pGameTitle,"Legion (J)") == 0)) //レギオン
				VDC_SetAutoRasterTiming(3); //LATE
		
		//スクリーンのノンストレッチorフルストレッチ設定で大きな問題があるソフト(ライン単位で解像度を変更しているゲーム)は自動でリアルストレッチにする
		if ((strcmp(pGameTitle,"Ryuuko no Ken (J)") == 0)|| //龍虎の拳
			(strcmp(pGameTitle,"Yami no Ketsuzoku Harukanaru Kioku (J)") == 0)) //闇の血族(解像度ではなくソースドット数の変更。これもリアルストレッチが必要)
		{
			_AutoRealStretched = APP_GetStartStretchMode(); //自動変更前のストレッチ設定を保存。v2.64
			if (_AutoRealStretched != 1)
				APP_SetStartStretchMode(1); //v2.64。//設定ファイルには保存せず、一時的にリアルストレッチへ変更。v2.64
		}		

		//スプライト制限を再現するソフト
		if ((strcmp(pGameTitle,"Tengai Makyou - Fuuun Kabuki Den (J)") == 0)|| //風雲カブキ伝。メッセージウィンドウにキャラが被らないように。
			(strcmp(pGameTitle,"Lodoss Tou Senki II (J)") == 0)|| //ロードス島戦記II。ザクソン村のパーンの家のビジュアルシーンなどで、ごみが出ないように。
			(strcmp(pGameTitle,"Princess Maker II (J)") == 0)|| //プリンセスメーカー２。デモを見た後のタイトル画面で右下に半円のゴミが出ないように。v2.10
			(strcmp(pGameTitle,"Ranma 1-2 (J)") == 0)|| //らんま1/2。OPデモのプライオリティ。v2.61
			(strcmp(pGameTitle,"Bonanza Bros. (J)") == 0)|| //ボナンザブラザーズ。タイトル画面のプライオリティ。v2.34
			(strcmp(pGameTitle,"Nekketsu Koukou Soccer Hen (J)") == 0)|| //熱血高校サッカー編。ステータス表示にキャラが被らないように。
			(strcmp(pGameTitle,"GALAXY Deka GAYVAN (J)") == 0)|| //GALAXY刑事ガイバン。OPデモのテロップ文字が乱れないように。
			(strcmp(pGameTitle,"Nemurenuyoru no Chiisana Ohanashi (J)") == 0)|| //原由子の「眠れる夜の小さなお話」。スプライト制限を利用したビジュアルシーンがある。v2.17。v2.26
			(strcmp(pGameTitle,"Black Hole Assault (J)") == 0)|| //ブラックホールアサルト。スプライト制限を利用したビジュアルシーンがある。v2.34
			(strcmp(pGameTitle,"Quiz Tonosama no Yabou (J)") == 0)) //クイズ殿様の野望。オープニングデモでスプライト制限を再現する。
				VDC_SetAutoPerformSpriteLimit(TRUE);
		
		//if ((strcmp(pGameTitle,"A. III (J)") == 0)|| //Ａ列車で行こうIII
		//	(strcmp(pGameTitle,"Vasteel II (J)") == 0)) //バスティール２
		//		VDC_SetAutoOverClock(6);//快適に遊ぶために速度アップ設定にする。
		
		//CDシーク&リードを低速(実機並み。動画などの大きなデータ以外は高速シークのまま。)にするゲーム
		if ((strcmp(pGameTitle,"Uchuu Senkan Yamato (J)") == 0)|| //宇宙戦艦ヤマト。デモ画面が速く進みすぎるのを解消。
			(strcmp(pGameTitle,"Metal Angel II (J)") == 0)|| //メタルエンジェル２。イベントシーンで画面化け＆音ズレしないために必要。
			(strcmp(pGameTitle,"Shin Onryou Senki (J)") == 0)) //真怨霊戦記。デモ画面が速く進みすぎるのを解消。
		{
	 		//ビジュアルシーンの音ずれを防ぐため、CDアクセスを速くする設定に。
			_bFastCD = TRUE;
			_AutoFastCD = -1; //ゲーム起動直後からFastCDをオンにした印(あとで元に戻すために必要)
	 		//ビジュアルシーンの音ずれを防ぐため、CDシークをノーマル設定に。
			_bFastSeek = FALSE;
			_AutoNormalSeek = -1; //ゲーム起動直後からFastSeekをオフにした印(あとで元に戻すために必要)
			//ＣＤインストール推奨の場合、メッセージを表示。
			//  宇宙戦艦ヤマトは、アクセスが遅いと稀に途中でADPCM音が消えてしまうことを解消。
			//  メタルエンジェル２は、イベントシーン(序盤の歓迎会シーン等)で画面化けしないために必要。
			//  真怨霊戦記は、タイトル画面(スタートして少し経ってからのタイトル)でタイミングを合わせるため必要。
			if (_CDInstall == 0)
				PRINTF("Recommend \"Install\".  %s", cdMessage); //インストール推奨
		}

		//CDシークを遅く(実機よりは速い。リードアクセスは速いまま。)するゲーム
		if ((strcmp(pGameTitle,"Fighting Street (J)") == 0)||
			(strcmp(pGameTitle,"Fighting Street (U)") == 0)|| //ファイティングストリート。勝負が決したとき(しゃがみで決着が付いたとき等)にADPCMが発生されないことがある問題を解消。
			(strcmp(pGameTitle,"Jantei Monogatari (J)") == 0)|| //雀偵物語。ポイントの技選択画面で乱れることがある問題が解消。v2.34
			(strcmp(pGameTitle,"DE-JA (J)") == 0)|| //DE・JA。画面の乱れを実機並みに軽減。v2.40
			(strcmp(pGameTitle,"Psychic Storm (J)") == 0)|| //サイキックストーム。スタート時など画面切り替え時に乱れが出ることがある問題が解消。v2.17
			(strcmp(pGameTitle,"Pop'n Magic (J)") == 0)) //ぽっぷ'nまじっく。スタートデモが乱れないようにするため必要。
				_bSeekWait = TRUE; //CDシークを遅くする設定に。

		//CDシークを遅く(実機よりは速い。リードアクセスは速いまま。)するゲーム
		if (strcmp(pGameTitle,"Vasteel (J)") == 0) //バスティール。デモシーンで音声が途切れないために必要。v2.34
		{
			_bSeekWait = TRUE; //CDシークを遅くする設定に。
			_bSeekWait2 = TRUE; //通常の_bSeekWaitより少な目のウェイト。
		}

		//CDシークをやや遅く(実機よりは速い。シーク瞬間時は速いままで、リードアクセス時にウェイトが入る。)するゲーム
/*		if ((strcmp(pGameTitle,"Psychic Storm (J)") == 0)) //サイキックストーム。※現状_bSeekWaitで行うようにしたためカット。v2.17
		{
			_bFastSeek = FALSE;
			_AutoNormalSeek = -1; //ゲーム起動直後からFastSeekをオフにした印(あとで元に戻すために必要)
		}
*/
		//CDリードアクセスを遅く(シーク等は速いまま)するゲーム
		if ((strcmp(pGameTitle,"3 x 3 Eyes - Sanjiyan Hensei (J)") == 0)|| //サザンアイズ。ノンアーケードカード時やゲーム中のデモ画面が速く進みすぎるのを解消。
			(strcmp(pGameTitle,"Garou Densetsu II (J)") == 0)|| //餓狼伝説２。勝利後やゲームオーバーのメッセージ表示が消えるのが早すぎるのを解消。v2.60
			(strcmp(pGameTitle,"Jantei Monogatari III (J)") == 0)) //雀偵物語３。ビジュアルシーンで画面が乱れるのを解消。
		{
			_bReadWait = TRUE; //CDリードアクセスを遅くする設定に。
		}

 		//CDアクセスが速すぎると画面が速く進みすぎるので実機並のアクセスウェイトを入れる。
		//  ゲームによっては、起動した最初から遅くすると待ち時間が長いので、タイトル画面など任意のタイミング(CDDAの曲ナンバーを合図とした)で遅くする。
		if (strcmp(pGameTitle,"Mirai Shonen Conan (J)") == 0) //未来少年コナン。ステージ開始時にアクセスが早いとフリーズ。
			_bAccessWait = TRUE; //ステージ開始のタイミングでステートロードされる場合も想定して、ゲーム最初から最後までアクセスウェイトを入れておく。
 		if (strcmp(pGameTitle,"Brandish (J)") == 0) //ブランディッシュ
		{
			_AutoSlowAccess = 4; //電源オン後のデモ途中。このトラックNo.のCDDA再生がおこなわれたと同時に、以降のCDアクセスを遅くする。
			_AutoRetAccess = 1000; //スタート画面BGM。4以外のトラックNo.で何かCDDA再生がおこなわれたと同時に、以降のCDアクセスを速くする(元に戻す)。
		}
		if (strcmp(pGameTitle,"Mugen Senshi Valis (J)") == 0) //ヴァリス１
		{
	 		//アクセスが速すぎると起動直後に止まってしまうため、CDシークを遅くする設定に。
			_bSeekWait = TRUE;
			_AutoSlowPlaySeekWait = 24; //オープニングデモ後半のトラックが再生されるとき実機並みのシークウェイトを入れる。口パクの音ズレが解消。
			_AutoRetPlaySeekWait = 1000; //上以外の曲が掛かったら速いシークウェイトに戻す。
		}
 		if (strcmp(pGameTitle,"Double Dragon II (J)") == 0) //ダブルドラゴンII
 		{
 			//スタート画面でRUNボタン押した直後のタイトルコールを最後まで発声させる。ステージ開始時のステージ数表示(CDアクセス中)を適度な長さに。
			_bSeekWait = TRUE;
			_bReadWait = TRUE;
			_bDoubleDragon = TRUE; //スタート画面でRUNボタン押した直後のアクセスだけ実機並の遅さにする。
		}

		//CDアクセスを高速(デフォルト)にし、CDインストール推奨(データトラックへの高速アクセスが必要)のゲーム。
		if ((strcmp(pGameTitle,"Private Eyedol (J)") == 0)|| //プライベートアイドル。シーン２前編のスタートデモで必要。
			(strcmp(pGameTitle,"Tenshi no Uta (J)") == 0)|| //天使の詩。タイトル画面直後や、オープニング直後に止まってしまわないように。街名の表示時間などもちょうどいい感じになる。v2.56
			(strcmp(pGameTitle,"Tenshi no Uta II (J)") == 0)|| //天使の詩２。デモが画面化け＆フリーズしないために必要。v2.56
			(strcmp(pGameTitle,"Dungeon Explorer II (J)") == 0)|| //ダンジョンエクスプローラーII。デモが画面化け＆音ズレしないために必要。
			(strcmp(pGameTitle,"Dungeon Explorer II (U)") == 0)|| //ダンジョンエクスプローラーII(US版)
			(strcmp(pGameTitle,"Travellers! Densetsu wo Buttobase (J)") == 0)|| //とらべらーず！。イベントシーンで音ズレしないために必要。
			(strcmp(pGameTitle,"Arunamu no Kiba (J)") == 0)) //アルナムの牙。スタートデモでフリーズしないために必要。
		{	//CDインストール推奨
			_bFastCD = TRUE;
			_AutoFastCD = -1; //ゲーム起動直後からFastCDをオンにした印(あとで元に戻すために必要)
			_bFastSeek = TRUE;
			_AutoNormalSeek = -1; //ゲーム起動直後からFastSeekをオンにした印(あとで元に戻すために必要)
			if (_CDInstall == 0)
				PRINTF("Recommend \"Install\".  %s", cdMessage); //インストール推奨
			if ((strcmp(pGameTitle,"Tenshi no Uta (J)") == 0)|| //天使の詩。タイトル画面直後や、オープニング直後に止まってしまわないように。街名の表示時間などもちょうどいい感じになる。v2.56
				(strcmp(pGameTitle,"Tenshi no Uta II (J)") == 0)) //天使の詩２。デモが画面化け＆フリーズしないために必要。v2.56
			{
				_bSeekWait = TRUE; //CDシークだけ遅くする設定に。
				_bSeekWait2 = TRUE; //短めのウェイトでOK。_bSeekWaitのみだと遅いので。
			}
			if ((strcmp(pGameTitle,"Dungeon Explorer II (J)") == 0)|| //ダンジョンエクスプローラーII
				(strcmp(pGameTitle,"Dungeon Explorer II (U)") == 0)) //ダンジョンエクスプローラーII(US版)
			{
				//ビジュアルシーンで画面が化けないように、ごく短いウェイト(実機より短い)を入れる。
				_bSeekWait = TRUE;
				_bSeekWait3 = TRUE; //ごく短めのウェイト。_bSeekWaitのみだとウェイトが大きすぎて音ズレがある。
			}
		}

		//CDアクセスを高速(デフォルト)にし、CDフルインストール推奨(音楽トラックへの高速アクセスが必要)のゲーム。
		if (strcmp(pGameTitle,"Doraemon Nobita no Dorabian Night (J)") == 0) //ドラえもん(SCD版)。面クリア時などに画面が乱れないために必要。
		{	//CDフルインストール推奨
			_bFastCD = TRUE;
			_AutoFastCD = -1; //ゲーム起動直後からFastCDをオンにした印(あとで元に戻すために必要)
			_bFastSeek = TRUE;
			_AutoNormalSeek = -1; //ゲーム起動直後からFastSeekをオンにした印(あとで元に戻すために必要)
			if (_CDInstall != 2)
				PRINTF("Recommend \"FullInstall\".  %s", cdMessage); //インストール推奨
		}
		
		if 	(strcmp(pGameTitle,"Popful Mail (J)") == 0) //ぽっぷるメイル
		{
			_PopfulMail = 1;
			//タイミングの問題か処理が追いつかず、画面化けや黒画面状態が起こるため、CPUの速度を上げて対処する。
			VDC_SetAutoOverClock(200);//ターボ２倍
			//スタートデモ開始のメイルが走る場面で、スプライト欠けを再現する必要がある。
			VDC_SetAutoPerformSpriteLimit(TRUE);
	 		//ビジュアルシーンの音ずれを防ぐため、CDアクセスを速くする設定に。
			_bFastCD = TRUE;
			_AutoFastCD = -1; //ゲーム起動直後からFastCDをオンにした印(あとで元に戻すために必要)
	 		//ビジュアルシーンの音ずれを防ぐため、CDシークをノーマル設定に。
			_bFastSeek = FALSE;
			_AutoNormalSeek = -1; //ゲーム起動直後からFastSeekをオフにした印(あとで元に戻すために必要)
			
			if (_CDInstall == 0)
				PRINTF("Recommend \"Install\".  %s", cdMessage); //インストール推奨
		}
		
		if ((strcmp(pGameTitle,"Sherlock Holmes no Tantei Kouza (J)") == 0)|| //シャーロックホームズ１
			(strcmp(pGameTitle,"Sherlock Holmes Consulting Detective (U)") == 0)|| //シャーロックホームズ１USA
			(strcmp(pGameTitle,"Sherlock Holmes no Tantei Kouza II (J)") == 0)|| //シャーロックホームズ２
			(strcmp(pGameTitle,"Sherlock Holmes Volume II (U)") == 0)) //シャーロックホームズ２USA
		{
			_bSherlock = TRUE; //ビジュアルシーンのCDアクセス時に最適なウェイトを入れて実機と同様の動きにする。
			
	 		//CDアクセスを速くする設定に
			_bFastCD = TRUE;
			_AutoFastCD = -1; //ゲーム起動直後からFastCDをオンにした印(あとで元に戻すために必要)
	 		//ビジュアルシーンの音ずれを防ぐため、CDシークを高速設定に。
			_bFastSeek = TRUE;
			_AutoFastSeek = -1; //ゲーム起動直後からFastSeekをオンにした印(あとで元に戻すために必要)
			
			if (_CDInstall == 0)
				PRINTF("Recommend \"Install\".  %s", cdMessage); //インストール推奨
		}
		
 		if (strcmp(pGameTitle,"Ys IV - The Dawn of Ys (J)") == 0) //イース４。有翼人種が出てくるイベントシーンでの音ずれ解消。
 		{
			_bYs4 = TRUE; //微妙なタイミングのビジュアルシーンの音ズレを解消する。
			
	 		//ビジュアルシーンの音ずれを防ぐため、CDアクセスを速くする設定に。
			_bFastCD = TRUE;
			_AutoFastCD = -1; //ゲーム起動直後からFastCDをオンにした印(あとで元に戻すために必要)
	 		//ビジュアルシーンの音ずれを防ぐため、CDシークをノーマル設定に。
			_bFastSeek = FALSE;
			_AutoNormalSeek = -1; //ゲーム起動直後からFastSeekをオフにした印(あとで元に戻すために必要)
			
			JOYPAD_SetRenshaSpeedMax(3); //連射速度を最大Lowまでに抑制する。
			
			if (_CDInstall == 0)
				PRINTF("Recommend \"Install\".  %s", cdMessage); //インストール推奨。アクセスが遅いとイベントシーンで１ラインの乱れが出ることがある。
		}
		
		if (strcmp(pGameTitle,"CD Battle Hikari no Yuushatachi (J)") == 0) //ＣＤバトル 光の勇者たち
		{
			APP_SetF1NoReset(TRUE);
			_bAutoF1NoReset = TRUE;
			PRINTF("F1 key for \"CD Replace\".  %s", cdMessage);
		}
		
		if ((strcmp(pGameTitle,"Ginga Fukei Densetsu Sapphire (J)") == 0)|| //銀河婦警伝説サファイア
			(strcmp(pGameTitle,"Ginga Fukei Densetsu Sapphire [bootleg] (J)") == 0))
		{
	 		//起動時の長い読み込み部分だけは、CDシークを高速設定に。
			_bFastSeek = TRUE;
			_AutoNormalSeek = 1000; //何かのCDDA再生がおこなわれたと同時に、以降はノーマルシーク設定にする。
		}
		
		if (strcmp(pGameTitle,"Kaze no Densetsu Xanadu II (J)") == 0) //風の伝説ザナドゥII
		{
			//プロローグ中にRUNボタンを押したときに出る画面で、スプライト欠けを再現する必要がある。
			_AutoSpriteLimit = 11; //電源オン後のデモ途中。このトラックNo.のCDDA再生がおこなわれたと同時に、以降はスプライトオーバーを再現する。
			_AutoNonSpriteLimit = -1; //CDDA再生がストップしたと同時に、以降はスプライトオーバーを再現しない。
		}
		if (strcmp(pGameTitle,"Exile - Toki no Hazama he (J)") == 0) //エグザイル
		{
			//ラストボス退治後のビジュアルシーンで、スプライト欠けを再現する必要がある。
			_AutoSpriteLimit = 13; //このトラックNo.のCDDA再生がおこなわれたと同時に、以降はスプライトオーバーを再現する。
			_AutoNonSpriteLimit = -1; //CDDA再生がストップしたと同時に、以降はスプライトオーバーを再現しない。
		}
		
		if (strcmp(pGameTitle,"Gradius II - Gofer no Yabou (J)") == 0) //グラディウスII
		{
			if (APP_GetAutoGradiusII())
			{
				 //起動時にコントローラ２の下ボタンを押しっぱなしにして、レーザー・スプレッドボムがちらつかない設定にする。
				_bGradiusII = TRUE;
				_GradiusIIwait = 0;
			}
		}
		
		if (strcmp(pGameTitle,"Downtown Nekketsu Monogatari (J)") == 0) //ダウンタウン熱血物語
			_bNekketsu = TRUE; //実機並みのCDアクセス速度に落とし、本屋のお姉さんの「またのごらいてんを おまちしています」のADPCM音声を最後まで発声させる。
		
		if (strcmp(pGameTitle,"Super Darius (J)") == 0) //スーパーダライアス
			_bDarius = TRUE; //実機並みのCDアクセス速度に落とし、スタート直後のイントロミュージック(ADPCM)を最後まで発声させる。
		
		if (strcmp(pGameTitle,"Chou Aniki (J)") == 0) //超兄貴。オープニングのタイトル画面で乱れないように。
		{
			_AutoSlowPlaySeekWait = 25; //オープニングの曲が始まったら、音楽トラックへのシーク速度を実機並にする。
			_AutoRetPlaySeekWait = 1000; //オープニング以外の曲再生を合図に、速いアクセスに戻す。
		}

		if (strcmp(pGameTitle,"Nekketsu Legend Baseballer (J)") == 0) //熱血レジェンドベースボーラー
		{
			_bBaseballer = TRUE; //実機並みのCDアクセス速度に落とし、ビジュアルシーンの音ズレを解消する。
			
	 		//CDアクセスを速くする設定に
			_bFastCD = TRUE;
			_AutoFastCD = -1; //ゲーム起動直後からFastCDをオンにした印(あとで元に戻すために必要)
	 		//ビジュアルシーンの音ずれを防ぐため、CDシークを高速設定に。
			_bFastSeek = TRUE;
			_AutoFastSeek = -1; //ゲーム起動直後からFastSeekをオンにした印(あとで元に戻すために必要)
			
			if (_CDInstall == 0)
				PRINTF("Recommend \"Install\".  %s", cdMessage); //インストール推奨
		}
		
 		if ((strcmp(pGameTitle,"Rayxanber II (J)") == 0)|| //ライザンバーII
 			(strcmp(pGameTitle,"Kakutou Haou Densetsu Algunos (J)") == 0)) //アルガノス
				JOYPAD_SetRenshaSpeedMax(2); //連射速度を最大Middleまでに抑制する。

 		if ((strcmp(pGameTitle,"Shadow of the Beast (J)") == 0)|| //シャドーオブザビースト(J)
 			(strcmp(pGameTitle,"Shadow of the Beast (U)") == 0))  //シャドーオブザビースト(U)
				JOYPAD_SetRenshaSpeedMax(3); //連射速度を最大Lowまでに抑制する。

		if (strcmp(pGameTitle,"Seisenshi Denshou - Jantaku no Kishi (J)") == 0) //聖戦士伝承－雀卓の騎士－。ADPCM音声の音とぎれを解消(軽減)
			_bFastAdpcmDma = TRUE; //ADPCMのDMA転送開始を通常より早めのタイミングにする。

		if (strcmp(pGameTitle,"Shin Megami Tensei (J)") == 0) //真女神転生
		{
			//戦闘シーンなどでメッセージウィンドウと、攻撃や魔法効果が被った時に、スプライト欠けを再現する必要がある。v2.20
			VDC_SetAutoPerformSpriteLimit(TRUE);
			
			if (APP_GetAutoShinMegamiTensei()) //最適化有効(デフォルト)に設定してあれば
			{
				//画面描画時などに時々１フレームの乱れが出るのを防ぐ。※実機でも出るが綺麗になるので実施。v2.20
				VDC_SetForceRaster(TRUE);
				VDC_SetForceVBlank(TRUE); //v2.24追加
				//タイミングの問題で稀にオートマップ画面が画面化けするのを防ぐため、若干CPUの速度を上げて対処する。※実機でも起こるが快適プレイのため実施。v2.20
				VDC_SetShinMegamiTensei(TRUE);
			}
		}

		if (strcmp(pGameTitle,"Aya (J)") == 0) //AYA。アクセスが速すぎてメッセージがすぐ消えてしまう場面を実機並みの遅さにする。
			_bAya = TRUE;

		if (strcmp(pGameTitle,"Orgel (J)") == 0) //オルゴール。アクセスが速すぎて場面が速く進みすぎるところを実機並みの遅さにする。
			_bOrgel = TRUE;

		if (strcmp(pGameTitle,"Pastel Lime (J)") == 0) //パステルライム
			_bLime = TRUE; //微妙なタイミングを取り、終盤の音途切れを解消する。
	}
}


static BOOL
check_cdrom2_disc(
	int dn)	//Kitao更新。dn=ドライブナンバー(0～9)
{
	BOOL	ret = FALSE;

	_CDInstall = 0; //Kitao追加。トラックのインストール状況

	if (CDIF_SelectDevice(dn))
	{
		//TOCをチェック
		check_disc_toc();
		//トラックのインストール状況をチェック
		_CDInstall = CDROM_CheckCDInstall();
		if ((APP_GetCDGame())&&(APP_GetCueFile())&&(_CDInstall != 2)) //CUE起動時にファイルが足りない場合
		{
			TOCDB_ClearGameTitle(); //ゲーム名をクリア
			PRINTF("CD: not ready.");
		}
		else
		{
			if (_CDInstall == 2)
				_bWavTrackInstalled = TRUE;
			else
				_bWavTrackInstalled = FALSE;
			ret = TRUE;
		}
		/*
		int t=10;
		//PRINTF("DiscInfo Test [%d] = %X, %X, %X", t,_DiscInfo.TOC[t].min,_DiscInfo.TOC[t].sec,_DiscInfo.TOC[t].frame);
		//PRINTF("DiscInfo Test [%d] = %X, %X, %X", t,_DiscInfo.TOC[t].lbaH,_DiscInfo.TOC[t].lbaM,_DiscInfo.TOC[t].lbaL);
		//PRINTF("DiscInfo Test [%d] = %X", t,_DiscInfo.TOC[t].isAudio);
		*/
	}
	else
	{
		//Kitao追加。ゲーム名をクリア
		TOCDB_ClearGameTitle(); //ゲーム名をクリア
		if (APP_GetCDGame())
		{
			if ((!APP_GetFullScreen())&&(APP_GetHideMessage()))
				PRINTF("%s - CD: not ready.", APP_GetAppName());
			else
				PRINTF("CD: not ready.");
		}
	}

	check_game(); //Kitao追加。各ゲームの最適設定をチェック。設定の初期化も行うので、Huカードゲーム起動時も必ずここを呼ぶこと。v2.64更新
	return ret;
}


static void
lba2msf(
	Uint32		lba,
	Uint8*		m,
	Uint8*		s,
	Uint8*		f)
{
	lba += 150; //Kitao追加v0.53。150=１トラック開始までのスタート秒数ぶん(プリギャップ)を足す。
	*m = (Uint8)(lba / 75 / 60);
	*s = (Uint8)((lba - *m * 75 * 60) / 75);
	*f = (Uint8)(lba - (*m * 75 * 60) - (*s * 75));
}

//Kitao追加v0.53
static Uint32
msf2lba(
	Uint8		m,
	Uint8		s,
	Uint8		f)
{
	return (m*60 + s)*75 + f - 150; //１秒間は75フレーム。150=１トラック開始までのスタート秒数ぶん(プリギャップ)を引く。
}


static Uint8
get_first_track()
{
	return (Uint8)(CDIF_GetFirstTrack());
}


static Uint8
get_last_track()
{
	return (Uint8)(CDIF_GetLastTrack());
}


//Kitao追加。曲のエンドポジションセクターアドレスを得る。プリギャップも考慮。
static void
get_track_end_position(
	const Uint8		track, //※次トラックのナンバー
	Uint8*			min,
	Uint8*			sec,
	Uint8*			frame)
{
	Uint32	lba;

	*min   = _DiscInfo.TOC[track].min;
	*sec   = _DiscInfo.TOC[track].sec;
	*frame = _DiscInfo.TOC[track].frame;

	if ((_DiscInfo.TOC[track].trackNum != 0)&&(_DiscInfo.TOC[track].isAudio == 0)) //次のトラックナンバーがあり、それがデータトラックなら。
	{	//プリギャップ(CDDA→DATA)領域ぶん(２秒)を取り除き、正確な曲の長さを返す。ゼロヨンチャンプIIのオープニング曲などで必要。
		lba = (_DiscInfo.TOC[track].lbaH<<16) + (_DiscInfo.TOC[track].lbaM<<8) + _DiscInfo.TOC[track].lbaL;
		*min = (Uint8)(lba / 75 / 60);
		*sec = (Uint8)((lba - *min * 75 * 60) / 75);
		*frame = (Uint8)(lba - (*min * 75 * 60) - (*sec * 75));
	}
}


static Uint8
get_track_number_by_msf(
	Uint8		m,
	Uint8		s,
	Uint8		f)
{
	return (Uint8)(CDIF_GetTrackNumber(m, s, f));
}


//Kitao追加
static void
setSubqTrackInfo()
{
	_SubqMinTrack = _DiscInfo.TOC[_AudioTrack.subqTrackNum+1].min - _DiscInfo.TOC[_AudioTrack.subqTrackNum].min;
	_SubqSecTrack = _DiscInfo.TOC[_AudioTrack.subqTrackNum+1].sec - _DiscInfo.TOC[_AudioTrack.subqTrackNum].sec;
	if (_SubqSecTrack < 0)
	{ 
		_SubqSecTrack += 60;
		--_SubqMinTrack;
	}
	_SubqFrmTrack = _DiscInfo.TOC[_AudioTrack.subqTrackNum+1].frame - _DiscInfo.TOC[_AudioTrack.subqTrackNum].frame;
	if (_SubqFrmTrack < 0)
	{
		_SubqFrmTrack += 75;
		--_SubqSecTrack;
	}
}


static void
simulate_subchannel_q()
{
	if (++_AudioTrack.subqFrmTrack >= 75)
	{
		_AudioTrack.subqFrmTrack = 0;
		if (++_AudioTrack.subqSecTrack >= 60)
		{
			_AudioTrack.subqSecTrack = 0;
			++_AudioTrack.subqMinTrack;
		}
	}

	if (++_AudioTrack.subqFrmTotal >= 75)
	{
		_AudioTrack.subqFrmTotal = 0;
		if (++_AudioTrack.subqSecTotal >= 60)
		{
			_AudioTrack.subqSecTotal = 0;
			++_AudioTrack.subqMinTotal;
		}
	}

	//Kitao追加。トラックの最後まで来た場合、subqの値をリセットする。
	if (_AudioTrack.subqTrackNum > 0)
		if ((_AudioTrack.subqMinTrack >= _SubqMinTrack)&&
			(_AudioTrack.subqSecTrack >= _SubqSecTrack)&&
			(_AudioTrack.subqFrmTrack >= _SubqFrmTrack))
		{
			_AudioTrack.subqMinTrack = 0;
			_AudioTrack.subqSecTrack = 0;
			_AudioTrack.subqFrmTrack = 0;
			++_AudioTrack.subqTrackNum;
			setSubqTrackInfo();
		}
}


//Kitao追加
static void
set_VOL()
{
	if (_CdVolumeEffect == 0.0)
		_VOL = 0.0; //ミュート
	else if (_CdVolumeEffect == 3)
		_VOL = (double)_CurrentCdVolume / 65535.0 * 3.0/4.0; // 3/4。v1.29追加
	else
	 	_VOL = (double)_CurrentCdVolume / 65535.0 / _CdVolumeEffect;
	_VOL *= 1.053510; //PSGの音量アップを優先し、CD音量も5.3510%アップさせるようにした。ドラキュラX，グラディウスII，イースI・II最適。27%辺りまでは音質的にも大丈夫そう(音量の大きなイース３)。v2.39,v2.62
}


//インデックスがバッファの先頭か中間地点に来ているかどうかをチェック。来ていたらCDから新たなデータが読み込まれれる。v2.32
static inline void
checkCDDAReadBufferIndex()
{
	Sint32	a;
	Sint32	b;

	//インデックスがバッファの最後に達していたら、インデックスを0に戻す。
	if (_CDDAReadBufferIndex == _CDDAReadBufferSize)
		_CDDAReadBufferIndex = 0;
	//インデックスがバッファの先頭なら、バッファの後半にデータを追加読み込みする。（初回再生時にも、すぐ行われることになる）
	if (_CDDAReadBufferIndex == 0)
	{
		a = _CDDASectorEndAddr - _CDDASectorAddr;
		if (a > _CDDAReadBufferSizeSector2)
			a = _CDDAReadBufferSizeSector2;
		if (a > 0) //v1.67更新
		{
			while (_bReadCdda2)
				Sleep(1); //処理落ちが重なって、前回の読み出しがまだ終わっていない場合、終わるまで(cd_callbackにて_bReadCdda2がFALSEになるまで)待つ。v2.17
			_bReadCdda2 = TRUE; //v2.17追加
			if (_bWavTrackInstalled) //CDインストールされたwavファイルを演奏中の場合
			{	//v2.33更新。フルインストール時でも、グラディウスIIのモアイ面など複数トラックをまたがって再生する場合に対応した。
				b = _PlayTrackNum;
				while (_DiscInfo.TOC[b+1].isAudio == 1) //次のトラックがCDDAトラックなら、次のトラックに突入しているかどうかをチェック
				{
					if (_CDDASectorAddr >= ((_DiscInfo.TOC[b+1].lbaH<<16)+(_DiscInfo.TOC[b+1].lbaM<<8)+_DiscInfo.TOC[b+1].lbaL))
						++b;  //次のトラック以降へ突入している。引き続きその次のトラックもチェック。
					else
						break; //今チェックしたトラック内に再生すべきセクターがある。
				}
				CDIF_ReadCddaSector2HDD(_CDDAReadBuffer + _CDDAReadBufferSize2, b,
									    (_CDDASectorAddr-((_DiscInfo.TOC[b].lbaH<<16)+(_DiscInfo.TOC[b].lbaM<<8)+ _DiscInfo.TOC[b].lbaL))*2352, a, TRUE);//バッファ後半部分に、CDDAの先頭セクターからWAVデータを読み出す。
			}
			else //通常の場合。CD-ROMドライブから読み込み
				CDIF_ReadCddaSector2(_CDDAReadBuffer + _CDDAReadBufferSize2, _CDDASectorAddr, a, TRUE);//バッファ後半部分に、CDDAの先頭セクターからWAVデータを読み出す。
			//読み出し成功の場合、cd_callbackにて通知される。この段階ではまだ読み込み完了していない。
		}
	}
	//インデックスがバッファの中間に達したら、バッファの前半にデータを追加読み込みする。
	if (_CDDAReadBufferIndex == _CDDAReadBufferSize2)
	{
		a = _CDDASectorEndAddr - _CDDASectorAddr;
		if (a > _CDDAReadBufferSizeSector2)
			a = _CDDAReadBufferSizeSector2;
		if (a > 0) //v1.67更新
		{
			while (_bReadCdda2)
				Sleep(1); //処理落ちが重なって、前回の読み出しがまだ終わっていない場合、終わるまで(cd_callbackにて_bReadCdda2がFALSEになるまで)待つ。v2.17
			_bReadCdda2 = TRUE; //v2.17追加
			if (_bWavTrackInstalled) //CDインストールされたwavファイルを演奏中の場合
			{	//v2.33更新。フルインストール時でも、グラディウスIIのモアイ面など複数トラックをまたがって再生する場合に対応した。
				b = _PlayTrackNum;
				while (_DiscInfo.TOC[b+1].isAudio == 1) //次のトラックがCDDAトラックなら、次のトラックに突入しているかどうかをチェック
				{
					if (_CDDASectorAddr >= ((_DiscInfo.TOC[b+1].lbaH<<16)+(_DiscInfo.TOC[b+1].lbaM<<8)+_DiscInfo.TOC[b+1].lbaL))
						++b;  //次のトラック以降へ突入している。引き続きその次のトラックもチェック。
					else
						break; //今チェックしたトラック内に再生すべきセクターがある。
				}
				CDIF_ReadCddaSector2HDD(_CDDAReadBuffer, b, 
										(_CDDASectorAddr-((_DiscInfo.TOC[b].lbaH<<16)+(_DiscInfo.TOC[b].lbaM<<8)+_DiscInfo.TOC[b].lbaL))*2352, a, TRUE);//バッファ前半部分に、CDDAの先頭セクターからWAVデータを読み出す。
			}
			else //通常の場合。CD-ROMドライブから読み込み
				CDIF_ReadCddaSector2(_CDDAReadBuffer, _CDDASectorAddr, a, TRUE);//バッファ前半部分に、CDDAの先頭セクターからWAVデータを読み出す。
			//読み出し成功の場合、cd_callbackにて通知される。この段階ではまだ読み込み完了していない。
		}
	}
}
		
//Kitao追加。CDDAをWAVデータで読み込み、ダイレクトサウンドで再生するようにした。
void
CDROM_Mix(
	Sint16*			pDst,				// 出力先バッファ //Kitao更新。CDDA専用バッファにしたためSint16に。
	Sint32			nSample)			// 書き出すサンプル数 
{
	Sint32	i;
	Sint32	sample1 = 0; //Leftチャンネル用
	Sint32	sample2 = 0; //Rightチャンネル用
	Sint32	cddaAdjust; //v2.32追加

	if ((!_AudioTrack.bPlaying)||(_AudioTrack.bPaused)||(_CDDAReadByteCount == 0)) //v2.29更新
	{
		if ((_FadeOut1 == 0)&&(_FadeOut2 == 0))
			return; //Hu-CardなどCD-DAを使わないゲームで処理が重くならないように、_FadeOutが0なら即リターン。
		
		//ノイズ軽減のためフェードアウトで消音する。
		for (i = 0; i < nSample; i++)
		{
			if (_FadeOut1 > 0)
			{
				_FadeOut1 -= 300; //小さく引きすぎると逆にノイズが出る(200だと駄目)。大きすぎる(800だと駄目)と効果なし。
				if (_FadeOut1 < 0)
					_FadeOut1 = 0;
			}
			else if (_FadeOut1 < 0)
			{
				_FadeOut1 += 300;
				if (_FadeOut1 > 0)
					_FadeOut1 = 0;
			}
			if (_FadeOut2 > 0)
			{
				_FadeOut2 -= 300;
				if (_FadeOut2 < 0)
					_FadeOut2 = 0;
			}
			else if (_FadeOut2 < 0)
			{
				_FadeOut2 += 300;
				if (_FadeOut2 > 0)
					_FadeOut2 = 0;
			}
			*pDst++ = (Sint16)_FadeOut1;
			*pDst++ = (Sint16)_FadeOut2;
		}
		return;
	}

	//PRINTF("%d", (int)_CDDAReadByteCount); //test用

	cddaAdjust = APP_GetCddaAdjust();
	for (i = 0; i < nSample; i++)	// mixing loop
	{
		//CD-DA再生速度の微調整。遅送り処理。v2.32追加
		if (cddaAdjust < 0)
			if (--_CDDAAjustCount <= cddaAdjust) //cddaAdjustの絶対値=何フレームに１回足踏みするか
			{	
				//前回のsampleでもう一度再生して足踏みする。初回で呼ばれた場合(i==0)は前回最終のsample(_FadeOut)を利用。
				if (i == 0)
				{
					sample1 = _FadeOut1;
					sample2 = _FadeOut2;
				}
				*pDst++ = (Sint16)sample1;
				*pDst++ = (Sint16)sample2;
				_CDDAAjustCount = 0;
				continue;
			}

		//インデックスがバッファの先頭か中間地点に来ているかどうかをチェック。来ていたらCDから新たなデータが読み込まれれる。v2.32更新
		checkCDDAReadBufferIndex();

		//v1.67更新
		sample1 = (Sint32)((Sint16)(_CDDAReadBuffer[_CDDAReadBufferIndex] | (_CDDAReadBuffer[_CDDAReadBufferIndex + 1] << 8)) * _VOL);
		_CDDAReadBufferIndex++;
		_CDDAReadBufferIndex++;
		sample2 = (Sint32)((Sint16)(_CDDAReadBuffer[_CDDAReadBufferIndex] | (_CDDAReadBuffer[_CDDAReadBufferIndex + 1] << 8)) * _VOL);
		_CDDAReadBufferIndex++;
		_CDDAReadBufferIndex++;
		//PRINTF("CD Sample %d, %d", sample1,sample2); //test用
		if (_CDDAReadByteCount > 0) //v2.04追加。メインスレッドで0にされたタイミングによっては有り得るかもしれないので念のため。
			_CDDAReadByteCount -= 4; //4バイト読み込んだ
		//Kitao追加。サブチャンネルの情報は、ここで１フレーム（1/75秒）ごとに更新するようにした。
		if (++_ClockCountSub == 2352/4) //１フレーム（1/75秒）経過したら
		{
			simulate_subchannel_q();
			_ClockCountSub = 0;
			//PRINTF("%d: %d %d %d",　_AudioTrack.subqTrackNum,_AudioTrack.subqMinTrack,_AudioTrack.subqSecTrack,_AudioTrack.subqFrmTrack); //test用
		}

		//CD-DA再生速度の微調整。早送り処理。v2.32追加
		if (cddaAdjust > 0)
		{
			//前回早送りした場合、今回の波形に前回飛ばした波形を合成する。ノイズ対策。v2.46
			if (_CDDAAjustCount == cddaAdjust) //cddaAdjust=何フレームに１回だけ早送りして進めるか
			{	
				sample1 = (_FadeOut1 + sample1) / 2;
				sample2 = (_FadeOut2 + sample2) / 2;
			}
			//早送りチェック
			if (++_CDDAAjustCount >= cddaAdjust) //cddaAdjust=何フレームに１回だけ早送りして進めるか
				if (_CDDAReadByteCount > 0) //まだ曲が終了していない場合のみ
				{	
					checkCDDAReadBufferIndex(); //インデックスがバッファの先頭か中間地点に来ているかどうかをチェック。来ていたらCDから新たなデータが読み込まれれる。
					//次の波形を合成。v2.46更新
					sample1 = (sample1 + (Sint32)((Sint16)(_CDDAReadBuffer[_CDDAReadBufferIndex] | (_CDDAReadBuffer[_CDDAReadBufferIndex + 1] << 8)) * _VOL)) / 2;
					_CDDAReadBufferIndex++;
					_CDDAReadBufferIndex++;
					sample2 = (sample2 + (Sint32)((Sint16)(_CDDAReadBuffer[_CDDAReadBufferIndex] | (_CDDAReadBuffer[_CDDAReadBufferIndex + 1] << 8)) * _VOL)) / 2;
					_CDDAReadBufferIndex++;
					_CDDAReadBufferIndex++;
					if (_CDDAReadByteCount > 0) //メインスレッドで0にされたタイミングによっては有り得るかもしれないので念のため。
						_CDDAReadByteCount -= 4; //よぶんに(早送りで)4バイト読み込んだ
					if (++_ClockCountSub == 2352/4) //１フレーム（1/75秒）経過したら
					{
						simulate_subchannel_q();
						_ClockCountSub = 0;
					}
					_CDDAAjustCount = 0;
				}
		}
	
		if (sample1> 32767) sample1= 32767; //Volを25%アップしたのでサチレーションチェック。v2.39
		if (sample1<-32768) sample1=-32768; //
		*pDst++ = (Sint16)sample1; //CDDA専用のバッファにしたので、プラスせず直接値を書き換える。
		if (sample2> 32767) sample2= 32767; //Volを25%アップしたのでサチレーションチェック。v2.39
		if (sample2<-32768) sample2=-32768; //
		*pDst++ = (Sint16)sample2; //ステレオ2chぶん書き込み完了
		
		_FadeOut1 = sample1; //最後の波形値
		_FadeOut2 = sample2; //2chぶん

		if (_CDDAReadByteCount <= 0)
			break;
	}
}


static void
update_irq_state()
{
	Uint8 irq = _Port[2] & _Port[3] & (0x4|0x8|0x10|0x20|0x40); //Kitao更新
	
	if (irq != 0)
	{
		INTCTRL_Request(INTCTRL_IRQ2);
		//PRINTF("CD IRQ = %X",irq); //test用
	}
	else
	{
		INTCTRL_Cancel(INTCTRL_IRQ2); //v1.57更新。v1.37でカットしたが、やっぱり必要。これがないと麻雀レモンエンジェルでフリーズ。
	}
}

void
adpcm_state_notification_callback_function(
	Uint32		adpcmState)
{
/*
{
	char s[100];
	sprintf(s,"ADPCM = %X", adpcmState);
	int ok;
	ok = MessageBox(WINMAIN_GetHwnd(), s, "Test", MB_YESNO); //Kitao Test
	if (ok != IDYES)
		WINMAIN_SetBreakTrap(FALSE);
}
*/
	switch (adpcmState)
	{
		case ADPCM_STATE_STOPPED:
			_Port[3] &= ~(SIGNAL_ADPCM1 | SIGNAL_ADPCM2);
			break;
		case ADPCM_STATE_HALF_PLAYED:
			_Port[3] |= SIGNAL_ADPCM1;
			_Port[3] &= ~SIGNAL_ADPCM2;
			break;
		case ADPCM_STATE_FULL_PLAYED:
			_Port[3] &= ~SIGNAL_ADPCM1;
			_Port[3] |= SIGNAL_ADPCM2;
			break;
		//case ADPCM_STATE_NORMAL:
		//	_Port[3] |= (SIGNAL_ADPCM1 | SIGNAL_ADPCM2); //Kitao更新
		//	break;
	}
	update_irq_state();
}


//Kitao追加
static void
callback_seekdata_sub()
{
	DWORD	t1, t2;

	_bReadBuffer2Full = TRUE; //CPU(メイン)スレッドでSleepループしている場合、これをCDアクセスのウェイトより前にTRUEにしないと、_CDAccessCountが減らずにOotakeごとフリーズする可能性がある。v2.00

	//CDアクセスのウェイト機能を追加。ゲームによってはCDアクセス中にCPUパワーを使うものがあり、その場合ノンウェイトだと問題が起こる。
	t1 = timeGetTime();
	t2 = t1 + 10000;//10秒以上待った場合は、CDアクセスにハード的なトラブルが出た可能性が高いと想定し、Sleep(1)へ切り替えてOSの安定に備える。
	while ((_CDAccessCount > 0)&&(!_bResetCDAccessCount)) //CPUスレッドの動作によってアクセスカウンタが0になるまで待つ
		WINMAIN_SafetySleepZero(t1, t2); //安全にSleep(0)を行う。v2.42更新 
	
	if (!_bReadSeekDone)
	{
		_Port[0] = 0xC8;
		_Port[3] |= SIGNAL_ACK;
		update_irq_state();
		_bError = FALSE;
		_bCommandDone = TRUE;
	}
	_bAdpcmDmaStart = TRUE; //ADPCMのDMA読み込みスタート許可。DMA読み込み開始時にReadByteCountが0だった場合、ReadByteCountがセットされるまで待つようにした。ロードス島戦記１。v1.31

	//グラディウスIIで、起動時にレーザー・スプレッドボムがちらつかない設定にする。
	if (_bGradiusII)
	{
		if (_GradiusIIwait == 0) //RUNボタンが押されて起動が開始された
			INPUT_SetGradiusII(TRUE); //コントローラ２の下ボタンを押しっぱなし開始
		if (++_GradiusIIwait == 14) //この回数ぶんexecute_seek_sector()が呼ばれたら、CDリード(環境によって時間が変化)が完了。
		{
			MAINBOARD_SetGradiusII(); //１秒間待ってから、コントローラ２の下ボタンを離して、SELECT,RUNが効くようにする。
			_bGradiusII = FALSE;
		}
	}
}

//Kitao追加。シーク＝「演奏開始＋ポーズ」を行うようにした。これでファージアスの邪皇帝のエンディングで音が鳴る。スプリガンmkIIも同様。v1.65
static void
callback_seek_sub()
{
	Sint8	min, sec, frame;
	DWORD	t1, t2;

	_PrevCDDASectorAddr    = _CDDASectorAddr;	 //今回シークした位置を記憶しておく。次回同じ位置でシークした場合、処理をはしょって高速化(実機も同様の動き)。v2.29
	_PrevCDDASectorEndAddr = _CDDASectorEndAddr; //

	_CDDAReadBufferIndex = 0;
	_CDDAAjustCount = 0; //v2.32追加
	_CDDAReadByteCount = 2352*(_CDDASectorEndAddr - _CDDASectorAddr);
	_CDDASectorAddr += _CDDAReadBufferSizeSector2;//次に読み込むときのためのセクターアドレスを更新
	if (_CDDASectorAddr > _CDDASectorEndAddr) 
		_CDDASectorAddr = _CDDASectorEndAddr; //v1.67追加

	_AudioTrack.playEndSec = (_CDDAReadByteCount /2352 / 75) + 1;//75セクタで１秒。「+1」は再生終了前にリピートしてしまうのを確実に防ぐためにある 
	_ClockCount = 0; //ステートセーブ時に正確な時間経過を保存するためにも使う ※v0.80から非使用
	_ClockCountSub = 0;

	// initialize sub Q channel for simulation  Kitao更新。曲(トラック)の途中から再生する場合にも対応した。
	_AudioTrack.subqTrackNum = (Uint8)_PlayTrackNum;
	setSubqTrackInfo();
	min = _AudioTrack.minStart - _DiscInfo.TOC[_AudioTrack.subqTrackNum].min;
	sec = _AudioTrack.secStart - _DiscInfo.TOC[_AudioTrack.subqTrackNum].sec;
	if (sec < 0)
	{
		sec += 60;
		--min;
	}
	frame = _AudioTrack.frameStart - _DiscInfo.TOC[_AudioTrack.subqTrackNum].frame;
	if (frame < 0)
	{
		frame += 75;
		--sec;
	}
	_AudioTrack.subqMinTrack = min;
	_AudioTrack.subqSecTrack = sec;
	_AudioTrack.subqFrmTrack = frame;
	_AudioTrack.subqMinTotal = _AudioTrack.minStart;
	_AudioTrack.subqSecTotal = _AudioTrack.secStart;
	_AudioTrack.subqFrmTotal = _AudioTrack.frameStart;

	//CDアクセスのウェイト機能を追加。ゲームによってはシーク時間をウェイトとして使っているものがあり、その場合ノンウェイトだと問題が起こる。
	//v2.36更新。PC環境ごとの動作に差が出ないように、変数設定処理まで終わってからウェイト待ちするようにした。
	t1 = timeGetTime();
	t2 = t1 + 10000;//10秒以上待った場合は、CDアクセスにハード的なトラブルが出た可能性が高いと想定し、Sleep(1)へ切り替えてOSの安定に備える。
	while ((_CDAccessCount > 0)&&(!_bResetCDAccessCount)) //CPUスレッドの動作によってアクセスカウンタが0になるまで待つ
		WINMAIN_SafetySleepZero(t1, t2); //安全にSleep(0)を行う。v2.42更新 
}

//Kitao追加。v2.28
static void
callback_seek_sub2()
{
	_AudioTrack.bSeekDone = TRUE; //シークが完了している合図。再生が始まったらFALSEになる。v2.29記

	//PCE側に結果を返す
	_Port[0] = 0xD8;
	_ReadByteCount = 0;
	_CheckCountAfterRead = 2;
	_bError = FALSE;
	_bCommandDone = TRUE;

	//Kitao追加。完了の割り込みを起こす。
	//_Port[2]が0x20のとき(コマンド処理中)に起こすとファージアスの邪皇帝エンディングでフリーズ。0x08のときは起こす(スーパーダライアス３面ボス)。v1.67
	//PRINTF("%X", _Port[2]); //test用
	if ((_Port[2] & 0x20) == 0) //コマンド処理中でなければ
	{
		_Port[3] |= SIGNAL_DONE; //Kitao参考：スプリガンmkIIのBGMを鳴らす＆面クリア時に必要。ヴァリス１のボス戦,スーパーダライアス３面ボスで固まらないために必要。
		update_irq_state();
	}
}

//Kitao追加
static void
callback_play_sub()
{
	if (_AudioTrack.playMode == 2) //Kitao更新。playModeが1のときはDriveBusyにしない。聖夜物語がついに動いた!。v1.62。
		_bDriveBusy = TRUE;

	if (_ReadCddaFirst == 1) //初めの再生スタートなら、PCE側に処理を終えたことを返す。
	{
		_ReadCddaFirst = 0;
		
		//PRINTF("PlayMode = %d",_AudioTrack.playMode);//Test用
		if ((_AudioTrack.playMode == 2)||(_AudioTrack.playMode == 3)) //2…ブランディッシュ，3…サザンアイズなど
		{
			//実機並みのタイミングに合わせるため、バッファ(デフォルト時…約0.075秒)間は先に音を発生させてから、PCE側に処理完了を返す。v1.63,v2.32更新
			//  v1.30更新。メインスレッドからCPUカウントで正確にウェイトするようにした。おそらく処理が速くないパソコン環境でも安定したはず。
			_CddaIntroCount = (Sint32)(7159090.0 * (((double)APP_GetSoundBufferSize()/(44100.0/2.0)) - 0.0167*APP_GetCddaDelayFrame())); //値が小さいほど画が早く(音が遅く)なる。-0.0167*APP_GetCddaDelayFrame()は液晶表示を考慮してAPP_GetCddaDelayFrame()ぶんフレーム遅延。
			if (_CddaIntroCount <= 0) _CddaIntroCount = 1;
	
			_AudioTrack.bPaused = FALSE; //この瞬間に音の再生が開始される
		}
		else //プレイモードが1のリピート再生の場合、すぐにPCE側に結果を返す。BURAIなどで必要。
		{
			_Port[0] = 0xD8;
			_ReadByteCount = 0;
			_CheckCountAfterRead = 2;
			
			_bError = FALSE;
			_bCommandDone = TRUE;

			_AudioTrack.bPaused = FALSE; //この瞬間に音の再生が開始される
		}
	}
	else
	{
		_ReadCddaFirst = 0;
		_AudioTrack.bPaused = FALSE; //この瞬間に音の再生が開始される
	}
}

//CDIFからのコールバックファンクション
//A callback function for notifying that command is successfully completed.
static void
cd_callback(
	Uint32		flag)
{
	//PRINTF("CD Callback %X", flag); //Kitaoテスト用

	switch (flag & ~CDIF_ERROR) //Kitao更新。エラーを考慮
	{
		case CDIF_SEEKDATA: //Kitao追加。データリード用のシーク(エミュレータではデータをバッファに先読み)
		case CDIF_SEEKDATAHDD: //Kitao追加。データをインストールしてあった場合用のシーク
			callback_seekdata_sub();
			break;

		case CDIF_READ: //Kitao更新。セカンダリバッファに先読み
		case CDIF_READHDD: //データをインストールしてあった場合用のシーク。v2.24追加
			_bReadBuffer2Full = TRUE;
			//※ここはエミュレータ側のハード処理なので、PCE側に_bError,_bCommandDoneは返さない。
			break;

		case CDIF_SEEK: //音楽トラックへのシーク時の、初回バッファの先読みREAD処理。v2.29
		case CDIF_SEEKHDD: //v2.24追加
			callback_seek_sub();
			callback_seek_sub2();
			_bCDDASeeking = FALSE;
			break;

		case CDIF_READCDDA://Kitao追加。音楽トラックのデジタル再生。初回バッファのREAD処理まで行う。
		case CDIF_READCDDAHDD://Kitao追加。CDインストール時用。v2.24
		case CDIF_PLAYCDDA://Kitao追加。バッファREAD処理は行わない。v2.29
			callback_seek_sub(); //シーク処理
			callback_play_sub(); //再生処理
			break;

		case CDIF_READCDDA2://Kitao追加。すでに再生し、追加でバッファデータを読み込んだ場合。
		case CDIF_READCDDA2HDD://Kitao追加。CDインストール時用。v2.24
			if (_ReadCddaFirst == 0) //次の曲を再生する合図が来ていなければ
			{
				_CDDASectorAddr += _CDDAReadBufferSizeSector2;//次に読み込むときのためのセクターアドレスを更新
				if (_CDDASectorAddr > _CDDASectorEndAddr) 
					_CDDASectorAddr = _CDDASectorEndAddr; //v1.67追加
			}
			_bReadCdda2 = FALSE; //v2.17追加
			//※ここはエミュレータ側のハード処理なので、PCE側に_bError,_bCommandDoneは返さない。
			break;

		case CDIF_INSTALL: //Kitao追加。インストール処理
		case CDIF_INSTALLWAV: //v2.24追加
			if (flag & CDIF_ERROR)
				_bInstallSuccess = FALSE;
			else
				_bInstallSuccess = TRUE;
			_bInstallBusy = FALSE;
			break;
	}
}


//Kitao追加
void
gamePatch()
{
	if (_PlayTrackNum == 0)
		return;

	//Kitao追加。CDアクセスの速度を速い設定(デフォルト)、または遅い設定にする必要があるソフトの場合、特定トラックの曲がかかったら、それを合図にここで設定を切り替える。
	if ((_AutoFastCD == 1000)||(_PlayTrackNum == _AutoFastCD))
		{_bFastCD = TRUE; APP_UpdateMenu();}
	if ((_AutoSlowCD == 1000)||(_PlayTrackNum == _AutoSlowCD))
		{_bFastCD = FALSE; APP_UpdateMenu();}
	//Kitao追加。CDアクセスの速度を実機並にする必要があるソフトの場合、特定トラックの曲がかかったら、それを合図にここで遅くする。
	if ((_AutoSlowAccess == 1000)||(_PlayTrackNum == _AutoSlowAccess))
		_bAccessWait = TRUE;
	else if ((_AutoRetAccess == 1000)||(_PlayTrackNum == _AutoRetAccess)) //遅くする必要がなくなったら元に戻す
		_bAccessWait = FALSE;

	//Kitao追加。CDシークの速度を遅く(実機よりは速い)する必要があるソフトの場合、特定トラックの曲がかかったら、それを合図にここで遅くする。
	if ((_AutoSlowSeekWait == 1000)||(_PlayTrackNum == _AutoSlowSeekWait))
		_bSeekWait = TRUE;
	else if ((_AutoRetSeekWait == 1000)||(_PlayTrackNum == _AutoRetSeekWait)) //遅くする必要がなくなったら元に戻す
		_bSeekWait = FALSE;

	//Kitao追加。CDリードの速度を遅く(実機よりは速い)する必要があるソフトの場合、特定トラックの曲がかかったら、それを合図にここで遅くする。
	if ((_AutoSlowReadWait == 1000)||(_PlayTrackNum == _AutoSlowReadWait))
		_bReadWait = TRUE;
	else if ((_AutoRetReadWait == 1000)||(_PlayTrackNum == _AutoRetReadWait)) //遅くする必要がなくなったら元に戻す
		_bReadWait = FALSE;

	//Kitao追加。CDシーク＆リードの速度を高速にする必要があるソフトの場合、特定トラックの曲がかかったら、それを合図にここで速くする。※v1.02から現在非使用
	if ((_AutoFastSeek == 1000)||(_PlayTrackNum == _AutoFastSeek))
		{_bFastSeek = TRUE; APP_UpdateMenu();}
	else if ((_AutoNormalSeek == 1000)||(_PlayTrackNum == _AutoNormalSeek)) //速くする必要がなくなったら元に戻す
		{_bFastSeek = FALSE; APP_UpdateMenu();}

	//Kitao追加。CDDAシークの速度を遅くする必要があるソフトの場合、特定トラックの曲がかかったら、それを合図にここで遅くする。
	if ((_AutoSlowPlaySeekWait == 1000)||(_PlayTrackNum == _AutoSlowPlaySeekWait))
		_bPlaySeekWait = TRUE;
	else if ((_AutoRetPlaySeekWait == 1000)||(_PlayTrackNum == _AutoRetPlaySeekWait)) //遅くする必要がなくなったら元に戻す
		_bPlaySeekWait = FALSE;

	//Kitao追加。スプライトオーバー再現する必要があるソフトの場合、特定トラックの曲がかかったら、それを合図にここでスプライトオーバー再現設定に変更する。
	if (_PlayTrackNum == _AutoSpriteLimit)
		{VDC_SetAutoPerformSpriteLimit(TRUE); APP_UpdateMenu();} 

	//Kitao追加。ラスタータイミングを切り替える必要があるソフトの場合、特定トラックの曲がかかったら、それを合図にここでラスタタイミングを切り替える。
	if (_PlayTrackNum == _AutoRasterTiming)
		{VDC_SetRasterTiming(_AutoSetRasterTiming); APP_UpdateMenu();}

	//Kitao追加。ぽっぷるメイル用パッチ処理。スタートデモで速度落とし中にRUNボタン押したときのために、曲がかかったら常にオーバークロック状態に戻す。
	if (_PopfulMail != 0)
		if (VDC_GetOverClockType() < 200)
			APP_SetAutoOverClock(200);//再び速い速度に。手動で変更されていたときのために、メニュー表示も更新する。

	//v2.28更新。APP_UpdateMenu()（メニュー表示を更新）は処理が重いので、変更があった場合のみ行うようにした。
}


//Kitao追加。stop_trackと違い、PCE側に結果は返さない。
void
CDROM_Stop(
	BOOL	bClearParameter) //bClearParameter=TRUEなら、PCE側から参照される再生状態パラメータもSTOP状態にする。
{
	//PRINTF("CDDA Stop = %d", _PlayTrackNum);//Kitao test用

	_CDDAReadByteCount = 0; //Kitao更新。CDDAを再生していた場合停止される。これによりCDDAデータへのアクセス予約もストップするので、ハード的なstopよりも前に行うことが重要。
	_CddaIntroCount = 0; //v1.23追加

	if (bClearParameter)
	{
		_AudioTrack.bPlaying  = FALSE;
		_AudioTrack.bSeekDone = FALSE;
		_AudioTrack.bPaused   = FALSE;
		_PlayTrackNum = 0;//Kitao追加。どの曲を再生していたか(再生を開始したトラック)
	}

	//Kitao追加。スプライトオーバー再現する必要があるソフトの場合で、スプライトオーバー再現の必要がなくなったら元に戻す(再現しない設定にする)。
	if (_AutoNonSpriteLimit == -1)
		VDC_SetAutoPerformSpriteLimit(FALSE);

	//Kitao追加。ラスタイミングを変更する必要があるソフトの場合で、ラスタイミングを変更する必要がなくなったら元に戻す(再現しない設定にする)。
	if (_AutoRasterTiming != 0)
		VDC_SetRasterTiming(_AutoRetRasterTiming);
}


static void
stop_track() //Kitao更新。WAVでCD再生するようにしたためハード的なストップはしない。PCE側に結果を返す。
{
	CDROM_Stop(TRUE); //TRUE…PCE側のパラメータもSTOP状態にする。

	//PCE側に結果を返す
	_Port[0] = 0xD8;
	_ReadByteCount = 0;
	_CheckCountAfterRead = 2;

	_bError = FALSE;
	_bCommandDone = TRUE;
}

static void
seek_track() //v2.05更新。_AudioTrack専用とした。
{
	Sint32	a;
	char	fileName[MAX_PATH+1];
	FILE*	fp;
	DWORD	t1, t2;

	CDROM_Stop(FALSE); //CDDA演奏中ならストップする。※シーク完了中の可能性があるので再生パラメータはSTOPにしない。
					   //演奏中でない場合(_AudioTrack.bPlaying=FALSE)でも、CDROM_MIXカウンターが0でない(CDDA読み込み最中の)こともあるためCDROM_Stop()は呼ぶ必要がある。

	//トラックナンバーを取得
 	_PlayTrackNum = get_track_number_by_msf(_AudioTrack.minStart, _AudioTrack.secStart, _AudioTrack.frameStart);
//Kitaoテスト用
/*
//if (WINMAIN_GetBreakTrap())
{
	char s[100];
	sprintf(s, "Seek TrackNo = %d", (int)_PlayTrackNum);
	int ok;
	ok = MessageBox(WINMAIN_GetHwnd(), s, "Test", MB_YESNO);
	if (ok != IDYES)
		WINMAIN_SetBreakTrap(FALSE);
}
*/
	//v2.29記。(_PlayTrackNum == 0)のときも、シーク処理は行う。ドラえもんで必要。

	//特定のゲームで必要な動作をおこなう
	gamePatch();

	t1 = timeGetTime();
	t2 = t1 + 10000;//10秒以上待った場合は、CDアクセスにハード的なトラブルが出た可能性が高いと想定し、Sleep(1)へ切り替えてOSの安定に備える。
	while (_bCDDASeeking)
		WINMAIN_SafetySleepZero(t1, t2); //連続してシークされて、前回のシーク処理が終わっていなかった場合は待つ。恐らく無いケースだが念のため。v2.29。安全にSleep(0)を行う。v2.42更新

	_CDDASectorAddr = msf2lba(_AudioTrack.minStart, _AudioTrack.secStart, _AudioTrack.frameStart);
	_CDDASectorEndAddr = msf2lba(_AudioTrack.minEnd, _AudioTrack.secEnd, _AudioTrack.frameEnd);

	//すでにシークが済んでいる場合は、バッファへの読み込みを省略して、すぐ帰還する。
	if ((_AudioTrack.bSeekDone)&&
		(_CDDASectorAddr == _PrevCDDASectorAddr)&&(_CDDASectorEndAddr == _PrevCDDASectorEndAddr))
	{
		_CDAccessCount = 0; //メインスレッドで処理するので必ず0。※1以上だとフリーズ
		callback_seek_sub();
		callback_seek_sub2(); //PCE側に結果が返る。この早いタイミングで返さないと、タイミングが合わないゲーム(ドラえもん等)あり。v2.29
		return;
	}

	//シークの際のウェイトを決定
	if ((!_bFastCD)||(_bPlaySeekWait)) //リアルCD速度モードの場合。（_bPlaySeekWaitは、曲再生時のシーク時間ぶんだけウェイトするモード）
		_CDAccessCount = (Sint32)(7159090.0 * 1.614); //1.614秒(play_track()時と合わせて1.615秒)。ヴァリス１のOPで合わせた。超兄貴のタイトルロゴ乱れが解消(実機でも稀に乱れる)。ヘッドの位置によって(ゲームと場面によって)多少変わってくる。v2.29,v2.36,v2.37,v2.39,v2.41
	else //通常（シークもノーウェイト）の場合
		_CDAccessCount = 0; //ここでウェイトを入れると、ドラえもん等で画面乱れ。サイキックストームのタイトル画面の音も乱れる。play_track()でウェイトが入るのでここではノーウェイトとする。v2.28更新

	//シーク処理。初回バッファぶんを先読みREADする。
	_AudioTrack.bSeekDone = FALSE;
	_AudioTrack.bPaused = FALSE;
	_AudioTrack.bPlaying = FALSE; //この段階では再生中にしない
	a = _CDDASectorEndAddr - _CDDASectorAddr;
	if (a > _CDDAReadBufferSizeSector2)
		a = _CDDAReadBufferSizeSector2;
	_bCDDASeeking = TRUE;
	CDROM_SetInstallWavFileName(_PlayTrackNum, fileName);
	if ((fp = fopen(fileName, "rb")) != NULL)
	{	//wavファイルがあった場合
		fclose(fp);
		_bWavTrackInstalled = TRUE;
		CDIF_SeekHDD(_CDDAReadBuffer, _PlayTrackNum, (_CDDASectorAddr-((_DiscInfo.TOC[_PlayTrackNum].lbaH<<16)+(_DiscInfo.TOC[_PlayTrackNum].lbaM<<8)+_DiscInfo.TOC[_PlayTrackNum].lbaL))*2352, a, TRUE); //インストールしたファイルから読み込み。
	}
	else //通常の場合。CD-ROMドライブから読み込み
	{
		_bWavTrackInstalled = FALSE;
		CDIF_Seek(_CDDAReadBuffer, _PlayTrackNum, _CDDASectorAddr, a, TRUE);//CDDAの先頭セクターからWAVデータを読み出す。
	}
	//シーク＆読み出し成功の場合、cd_callbackにて通知される。この段階ではまだ読み込み完了していない。
}

//Kitao更新。CDROMのセクターから直接CDDAのWAVデータを読み出して再生するようにした。v2.05から_AudioTrack専用とした。
static void
play_track(
	BOOL	bRepeat) //bRepeat…リピート再生時の場合TRUEで呼ぶ。リピートの場合でも初回の再生時はFALSEで呼ぶ。v1.36追加
{
	Sint32	a;
	char	fileName[MAX_PATH+1];
	FILE*	fp;
	DWORD	t1, t2;

	if (_AudioTrack.bPlaying) //Kitao追加。すでに演奏中の場合、ポーズを解除してそのままの状態でリターン。ウィザードリィI・II
	{
		_AudioTrack.bPaused = FALSE;
		
		_ReadCddaFirst = 0;
		
		_Port[0] = 0xD8;
		_ReadByteCount = 0;
		_CheckCountAfterRead = 2;
		
		_bError = FALSE;
		_bCommandDone = TRUE;
		return; 
	}

	//Kitao追加。フェード用の音量を初期化。ここで行うことで、熱血大運動会の「ここまでの・・・」のアナウンス音声が正常に。v1.31
	if (!bRepeat) //リピートじゃない場合。リピート時にフェードを初期化すると、天外２の戦闘シーンでフィールド音量が復活してしまう不具合あり。v1.36
	{
		CDROM_Stop(FALSE); //CDDA演奏中ならストップする。※シーク完了中の可能性があるので再生パラメータはSTOPにしない。
						   //演奏中でない場合(_AudioTrack.bPlaying=FALSE)でも、CDROM_MIXカウンターが0でない(CDDA読み込み最中の)こともあるためCDROM_Stop()は呼ぶ必要がある。

		//フェード用の音量を初期化
		if (_AudioTrack.playMode != 3) 	//「AudioTrack.playMode == 3」の時に入れると、レインボーアイランドのボス戦でやられたあとにCDDAが乱れる。
		{
			_CurrentCdVolume = _InitialCdVolume;
			set_VOL();
			_bFadeOut = FALSE;
			_bFadeIn  = FALSE;
			_FadeCycle = 0;
		}
	}

	//トラックナンバーを取得
 	_PlayTrackNum = get_track_number_by_msf(_AudioTrack.minStart, _AudioTrack.secStart, _AudioTrack.frameStart);
//Kitaoテスト用
/*
//if (WINMAIN_GetBreakTrap())
{
	char s[100];
	sprintf(s, "Play TrackNo = %d", (int)_PlayTrackNum);
	int ok;
	ok = MessageBox(WINMAIN_GetHwnd(), s, "Test", MB_YESNO);
	if (ok != IDYES)
		WINMAIN_SetBreakTrap(FALSE);
}
*/
	if (_PlayTrackNum == 0)
	{	//トラックナンバーが0のときは、CDDAをストップさせる。スターモビールで必要。v1.11
		stop_track(); //PCE側にもストップ完了の合図を伝える。
		return;
	}

	//特定のゲームで必要な動作をおこなう
	gamePatch();

	_CDDASectorAddr = msf2lba(_AudioTrack.minStart, _AudioTrack.secStart, _AudioTrack.frameStart);
	_CDDASectorEndAddr = msf2lba(_AudioTrack.minEnd, _AudioTrack.secEnd, _AudioTrack.frameEnd);

	if ((!_bFastCD)||(_bPlaySeekWait)) //リアルCD速度モードの場合。（_bPlaySeekWaitは、曲再生時のシーク時間ぶんだけウェイトするモード）
		_CDAccessCount = (Sint32)(7159090.0 * 1.614); //1.614秒(play_track()時と合わせて1.615秒)。ヴァリス１のOPで合わせた。超兄貴のタイトルロゴ乱れが解消(実機でも稀に乱れる)。ヘッドの位置によって(ゲームと場面によって)多少変わってくる。v2.29,v2.36,v2.37,v2.39,v2.41
	else //通常（シークもノーウェイト）の場合
		_CDAccessCount = (Sint32)(7159090.0 * 0.001); //0.001秒。遅延が出ない程度に多くウェイトしたほうが安定するようだ。0.002秒以上だと、超兄貴でやられたときにバグる(絵と音の若干の乱れ。実機も起こる)率が高い感じ。0.003秒以上だと、超兄貴のキャラ選択画面表示時にもたつく(１フレーム画面が乱れる)ことあり。v2.36,v2.37,v2.39,v2.41
	
	if (bRepeat) //リピート再生の場合は、PCエンジン側に結果を返さない。アルナムの牙のＯＰデモで必要。v1.47
	{
		_CDAccessCount = 0;
		_ReadCddaFirst = 0;
	}
	else
		_ReadCddaFirst = 1;//初回再生の合図

	t1 = timeGetTime();
	t2 = t1 + 10000;//10秒以上待った場合は、CDアクセスにハード的なトラブルが出た可能性が高いと想定し、Sleep(1)へ切り替えてOSの安定に備える。
	while (_bCDDASeeking)
		WINMAIN_SafetySleepZero(t1, t2); //シーク処理中の場合は待つ。シークが終わったことを確認してからここが呼ばれるので、ほぼ無いケースだが念のため。v2.29。安全にSleep(0)を行う。v2.42更新

	//すでにシークが済んでいる場合は、バッファへの読み込みを省略して、即再生する。
	//実機も同様の動きで、即再生しないとタイミングが合わないゲーム(ドラえもん等)あり。v2.29
	if ((_AudioTrack.bSeekDone)&&
		(_CDDASectorAddr == _PrevCDDASectorAddr)&&(_CDDASectorEndAddr == _PrevCDDASectorEndAddr))
	{
		//再生の際のウェイトを再設定。※シークが済んでいるのでウェイトも少なくする。実機でもすでにシークしてある場合は瞬時に再生される。
		//if ((!_bFastCD)||(!_bFastSeek)||(_bPlaySeekWait)||(_bPlaySeekWait2)) //リアルCD速度モードの場合。（_bPlaySeekWaitは、曲再生時のシーク時間ぶんだけウェイトするモード。_bPlaySeekWait2は連続シーク時だけウェイトするモード）。※現状はデフォルトでここのウェイトは必ず入れるようにした。v2.36
		_CDAccessCount = (Sint32)(7159090.0 * 0.001); //0.001秒。遅延が出ない程度に多くウェイトしたほうが安定するようだ。0.002秒以上だと、超兄貴でやられたときにバグる(絵と音の若干の乱れ。実機も起こる)率が高い感じ。0.003秒以上だと、超兄貴のキャラ選択画面表示時にもたつく(１フレーム画面が乱れる)ことあり。v2.36,v2.37,v2.39,v2.41
													  //         0.007秒以上だと、超兄貴で初回やられたときに１フレーム画面が乱れる(実機でも稀には乱れる)。0.010秒以上だと、ドラえもん(CD版)で最初のボスクリア時に画面乱れあり。
													  //         80クロックぐらい以上で、瞬時にCDアクセス処理が済んだ場合でもスナッチャー安定動作(75で稀にNG)。環境によっても違うかもしれないので今後も要調整。
													  //         50クロックぐらい以上で、悪魔城ドラキュラのサウンドテストでのフリーズを防止(40はNG)。小さい値だと曲選択ボタン連打でフリーズする。v1.28。v2.29更新
													  //         このウェイトがないと、スナッチャーのOP，風の伝説ザナドゥ２のOPで稀に止まることがあった。
		//_CDAccessCount = 0;//test用
		//再生処理
		_AudioTrack.bSeekDone = FALSE;
		_AudioTrack.bPaused = TRUE;
		_AudioTrack.bPlaying = TRUE; //v1.17更新。この段階で"再生中"とした。(リピート再生時のために)
		CDIF_PlayCdda(TRUE); //再生処理。ハード的には何もしないでcd_callback()へ飛ぶだけだが、_CDAccessCountでのウェイトを入れるためにコールバック形式での処理が必要。
		//この後cd_callbackにて通知される。この段階ではまだ読み込み完了の通知をPCE側にしていない状態。
		//Sleep(1000); //メインCPUスレッドを止めることで、CDアクセスが超高速なPC環境での動作を再現する。test用
		return;
	}

	//シークが済んでいないか、別の場所でシークされていた場合は、シーク処理をしてから再生する。v2.29
	_AudioTrack.bSeekDone = FALSE;
	_AudioTrack.bPaused = TRUE;
	_AudioTrack.bPlaying = TRUE; //v1.17更新。この段階で"再生中"とした。(リピート再生時のために)
	a = _CDDASectorEndAddr - _CDDASectorAddr;
	if (a > _CDDAReadBufferSizeSector2)
		a = _CDDAReadBufferSizeSector2;
	CDROM_SetInstallWavFileName(_PlayTrackNum, fileName);
	if ((fp = fopen(fileName, "rb")) != NULL)
	{	//wavファイルがあった場合
		fclose(fp);
		_bWavTrackInstalled = TRUE;
		CDIF_ReadCddaSectorHDD(_CDDAReadBuffer, _PlayTrackNum, (_CDDASectorAddr-((_DiscInfo.TOC[_PlayTrackNum].lbaH<<16)+(_DiscInfo.TOC[_PlayTrackNum].lbaM<<8)+_DiscInfo.TOC[_PlayTrackNum].lbaL))*2352, a, TRUE); //インストールしたファイルから読み込み。
	}
	else //通常の場合。CD-ROMドライブから読み込み
	{
		_bWavTrackInstalled = FALSE;
		CDIF_ReadCddaSector(_CDDAReadBuffer, _CDDASectorAddr, a, TRUE);//CDDAの先頭セクターからWAVデータを読み出す。
	}
	//読み出し成功の場合、cd_callbackにて通知される。この段階ではまだ読み込み完了していない。
}

//Kitao追加。ステートロードから再開用のplay_track。
static void
play_track2() //v2.05から_AudioTrack専用とした。
{
	Uint8	min;
	Uint8	sec;
	Uint8	frame;
	char	fileName[MAX_PATH+1];
	FILE*	fp;

//Kitaoテスト用
/*
//if (WINMAIN_GetBreakTrap())
{
	char s[100];
	sprintf(s, "Play2 TrackNo = %d", (int)_PlayTrackNum);
	int ok;
	ok = MessageBox(WINMAIN_GetHwnd(), s, "Test", MB_YESNO);
	if (ok != IDYES)
		WINMAIN_SetBreakTrap(FALSE);
}
*/
	//特定のゲームで必要な動作をおこなう
	gamePatch();

	_ReadCddaFirst = 0; //初回再生を終えているので0にセット

	//すぐにアクセスできるよう、曲の再生位置手前にシークする。シングルスレッド(メインスレッド)ですぐ行われる。
	CDROM_SetInstallWavFileName(_PlayTrackNum, fileName);
	if ((fp = fopen(fileName, "rb")) != NULL)
	{	//wavファイルがあった場合
		fclose(fp);
		_bWavTrackInstalled = TRUE;
		return; //音楽トラックがCDインストールしてある場合はシークせずすぐに帰る。
	}
	_bWavTrackInstalled = FALSE;
	min = _AudioTrack.minStart + _AudioTrack.subqMinTrack;
	sec = _AudioTrack.secStart + _AudioTrack.subqSecTrack;
	if (sec >= 60)
	{
		sec -= 60;
		++min;
	}
	frame = _AudioTrack.frameStart +  _AudioTrack.subqFrmTrack;
	if (frame >= 75)
	{
		frame -= 75;
		++sec;
	}
	CDIF_SeekCdda(min, sec, frame);
}

//Kitao更新。CD-DAをWAVで再生するようにしたためポーズもソフトウェアだけでおこなう。
static void
pause_track(
	BOOL		bPause)
{
	_AudioTrack.bPaused = bPause; //Kitao追加

	_Port[0] = 0xD8;
	_ReadByteCount = 0;
	_CheckCountAfterRead = 2;

	_bError = FALSE;
	_bCommandDone = TRUE;
}


//Kitao追加。CD-ROMアクセス速度を上げるため、インストール機能を付けた。v1.00
static void
read_sector()
{
	int		n;
	char	fileName[MAX_PATH+1];
	FILE*	fp;
	int		track;
	int		lastTrack = CDIF_GetLastTrack();

	//PRINTF("ReadByteCount = %X", _ReadByteCount); //Test用
	n = (_ReadByteCount-2048*32) / 2048;
	if (n > 32)
		n = 32;

	track = 1;
	while (((_DiscInfo.TOC[track+1].lbaH<<16)+(_DiscInfo.TOC[track+1].lbaM<<8)+_DiscInfo.TOC[track+1].lbaL <= _ReadSectorAddr)&&
			(track <= lastTrack))
		track++;
	CDROM_SetInstallFileName(track, fileName);
	if ((fp = fopen(fileName, "rb")) != NULL)
	{	//isoファイルが存在した場合
		fclose(fp);
		CDIF_ReadSectorHDD(_ReadBuffer2, track, (_ReadSectorAddr-((_DiscInfo.TOC[track].lbaH<<16)+(_DiscInfo.TOC[track].lbaM<<8)+_DiscInfo.TOC[track].lbaL))*2048, n, TRUE); //インストールしたファイルから読み込み。
	}
	else //通常の場合。CD-ROMドライブから読み込み
		CDIF_ReadSector(_ReadBuffer2, _ReadSectorAddr, n, TRUE); //次のセクターを別スレッドで先読みしておく
	_ReadSectorAddr += n;
}


/*-----------------------------------------------------------------------------
	[read_1801]
		CD-ROM からのデータを読み出す。
-----------------------------------------------------------------------------*/
static Uint8
read_1801()
{
	Uint8	ret;

	//PRINTF("ReadByteCount = %X", _ReadByteCount); //test
	if (_ReadByteCount > 0)
	{
		//Kitao更新。バッファを64Kバイトにし、先読み用のセカンダリバッファ(64Kバイト)を用意した。
		if (_ReadBufferIndex == 2048*32) //インデックスが2048*32になるたびに0に戻してセクターリードする。
		{
			if (!_bReadBuffer2Full) //セカンダリバッファの読み込みが未完了なら完了するまで待つ
			{
				_bResetCDAccessCount = TRUE; //CDIFのスレッドがループに陥ってしまわないために必要。
				while (!_bReadBuffer2Full)
					Sleep(1);
				_bResetCDAccessCount = FALSE;
			}
			memcpy(_ReadBuffer, _ReadBuffer2, 2048*32);
			_bReadBuffer2Full = FALSE;
			if (_ReadByteCount-2048*32 > 0)
				read_sector(); //次のセクターを別スレッドで先読みしておく
			_ReadBufferIndex = 0;
		}
		ret = _ReadBuffer[_ReadBufferIndex++];

		if (--_ReadByteCount == 0)
		{
			_Port[3] &= ~SIGNAL_ACK;
			_Port[3] |= SIGNAL_DONE; // data transfer done
			update_irq_state();
			
			_Port[0] = 0xD8;	// no more data left in buffer
			_ReadBufferIndex = 0;
			
			// 読み出し処理が終わった後に
			// 処理が正常に終了したかどうかを確認するための
			// 読み出しが２回行なわれる。
			_CheckCountAfterRead = 2;
		}
		else
			_Port[0] = 0xC8; //まだ読み込むデータがある

		return ret;
	}
	else	// バッファのデータが全て読み出された後に確認のための $1801 読み出しが２回行なわれる。 
	{
		if (_CheckCountAfterRead == 2)
		{
			--_CheckCountAfterRead;
			_Port[0] = 0xF8;
		
			if (_bError)
			{
				_bError = FALSE;
				return 1;
			}
		}
		else if (_CheckCountAfterRead == 1)
		{
			--_CheckCountAfterRead;
			_Port[0] &= ~SIGNAL_BUSY; //BUSYを解除
		}
	}

	return 0;
}


/*-----------------------------------------------------------------------------
	[read_1808]
		CD-ROM からのデータを読み出す。

	[多分]
		read_1801 との違いは、ＣＤ－ＲＯＭバッファ内の値を読み終えた後の
		２回の読み出しの値を返さない点。 
-----------------------------------------------------------------------------*/
static Uint8
read_1808()
{
	Uint8	ret;

	//PRINTF("ReadByteCount = %X", _ReadByteCount); //test
	if (_ReadByteCount > 0)
	{
		//Kitao更新。バッファを64Kバイトにし、先読みようのセカンダリバッファ(64Kバイト)を用意した。
		if (_ReadBufferIndex == 2048*32) //インデックスが2048*32になるたびに0に戻してセクターリードする。
		{
			if (!_bReadBuffer2Full) //セカンダリバッファの読み込みが未完了なら完了するまで待つ
			{
				_bResetCDAccessCount = TRUE; //CDIFのスレッドがループに陥ってしまわないために必要。
				while (!_bReadBuffer2Full)
					Sleep(1);
				_bResetCDAccessCount = FALSE;
			}
			memcpy(_ReadBuffer, _ReadBuffer2, 2048*32);
			_bReadBuffer2Full = FALSE;
			if (_ReadByteCount-2048*32 > 0)
				read_sector(); //次のセクターを別スレッドで先読みしておく
			_ReadBufferIndex = 0;
		}
		ret = _ReadBuffer[_ReadBufferIndex++];

		if (--_ReadByteCount == 0)
		{
			_Port[3] &= ~SIGNAL_ACK;
			_Port[3] |= SIGNAL_DONE; // data transfer done
			update_irq_state();
			
			_Port[0] = 0xD8;	// no more data left in buffer
			_ReadBufferIndex = 0;
			
			// 読み出し処理が終わった後に
			// 処理が正常に終了したかどうかを確認するための
			// 読み出しが２回行なわれる。
			_CheckCountAfterRead = 2;
		}
		else
			_Port[0] = 0xC8; //まだ読み込むデータがある

		return ret;
	}
	else
	{
		if (_CheckCountAfterRead > 0) //Kitao追加。_CheckCountAfterReadが0の場合は、ポート0を変更しない。雀偵物語３が動くようになった。エルディスもOK。v1.34
			_Port[0] = 0xD8; //すべてデータを読み出した
	}

	return 0;
}


//Test用
void
show_command_string(
	const char*		pCmdName,
	const Uint8*	pCmdString)
{
/*
	// This function only used for hardware-level logging
	//
	PRINTF("%s %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n",
			pCmdName,
			pCmdString[0],pCmdString[1],pCmdString[2],pCmdString[3],pCmdString[4],
			pCmdString[5],pCmdString[6],pCmdString[7],pCmdString[8],pCmdString[9]);
*/
}


/*-----------------------------------------------------------------------------
	[Init]
-----------------------------------------------------------------------------*/
Sint32 //v2.33更新。戻り値をCD-ROM(DVD,BD)ドライブ数とした。初期化エラーの場合-1を返す。ノードライブの場合0を返す。
CDROM_Init()
{
	Sint32	nCdromDevice;

	_bCdromInit = FALSE;

	CDROM_SetCdVolume(APP_GetCdVolume());//Kitao追加
	_bFadeOut = FALSE;
	_bFadeIn  = FALSE;
	_bFastCD = APP_GetDefaultFastCD(); //Kitao追加
	_bFastSeek = APP_GetDefaultFastSeek(); //Kitao追加
	_FadeOut1 = 0; //Kitao追加
	_FadeOut2 = 0; //Kitao追加
	memset((AudioTrack*)&_AudioTrack, 0, sizeof(_AudioTrack)); //CD-ROMドライブが使えない環境で、CDROM_AdvanceClock()が誤動作しないように。v1.04追加

	nCdromDevice = CDIF_Init(cd_callback);
	if (nCdromDevice == -1)
		return -1;

	if (check_cdrom2_disc(APP_GetCurrentCdDrive()))
	{
		_bCdromInit = TRUE;
		_bError = FALSE;
	}

	_ResetDelayCount = 0;
	ADPCM_SetNotificationFunction(adpcm_state_notification_callback_function);

	return nCdromDevice;
}


/*-----------------------------------------------------------------------------
	[Deinit]
-----------------------------------------------------------------------------*/
void
CDROM_Deinit()
{
	//Kitao追加。CDIFのスレッドで処理中(CDアクセス等)の場合、処理が終わるまで待つ。
	CDROM_Stop(FALSE); //Kitao追加。CDDAを止める。CDDA演奏中にファイルオープンしてHuカードのゲームを遊ぶときのために必要。
	_CDAccessCount = 0;
	_bResetCDAccessCount = TRUE; //CDIFのスレッドがループに陥ってしまわないために必要。
	CDIF_WaitDeviceBusy(); //CDIFのスレッドで処理中(CDアクセス等)の場合、処理が終わるまで待つ。
	_bResetCDAccessCount = FALSE;
	CDROM_Stop(FALSE); //タイミングによってCDDA演奏用変数がまた変更されている可能性があるので、CDIFスレッド終了後にもう一度STOP処理。

	INTCTRL_Cancel(INTCTRL_IRQ2);

	_bResetCDAccessCount = TRUE;
	CDIF_Deinit();
	_bResetCDAccessCount = FALSE;

	_bBRAMEnabled = FALSE;
	memset(_Port, 0, sizeof(_Port));

	_bReadBuffer2Full = FALSE; //Kitao追加
	_ReadSectorAddr = 0; //Kitao追加
	_ReadBufferIndex = 0;
	_ReadByteCount = 0;
	_bAdpcmDma = FALSE; //Kitao追加

	_Command = 0;
	_ArgsLeft = 0;
	_CmdArgBufferIndex = 0;

	_bCommandReset = TRUE;
	_bCommandReceived = FALSE;
	_CheckCountAfterRead = 0;
	memset((AudioTrack*)&_AudioTrack, 0, sizeof(_AudioTrack));
	_ClockCount = 0;
	_ClockCountSub = 0;//Kitao追加
	_FadeClockCount = 0;//Kitao追加
	_PlayTrackNum = 0;//Kitao追加
	_CddaIntroCount = 0;//Kitao追加

	_bDriveBusy = FALSE;
	_bError = FALSE;
	_bCDDASeeking = FALSE; //Kitao追加

	_bCdromInit = FALSE;
}


/*-----------------------------------------------------------------------------
	[Reset]
-----------------------------------------------------------------------------*/
BOOL
CDROM_Reset()
{
	CDROM_Deinit();
	return (CDROM_Init() != -1);
}


//Kitao追加。リセットせずにCD-ROMを入れ替える処理。TOC情報を更新する。
void
CDROM_NoResetCDChange(
	int dn)	//Kitao更新。dn=ドライブナンバー(0～9)
{
	char*	pCDTitle; //Kitao追加
	char	cdMessage[256]; //Kitao追加

	if (CDIF_SelectDevice(dn))
	{
		check_disc_toc();
		if (APP_GetCDGame())
		{
			pCDTitle = TOCDB_GetGameTitle();
			sprintf(cdMessage, "Game is stay. CD: %s", pCDTitle);
			if ((!APP_GetFullScreen())&&(APP_GetHideMessage()))
				PRINTF("%s - %s", APP_GetAppName(), cdMessage);
			else
				PRINTF("%s", cdMessage);
		}
	}
	else
	{
		if (APP_GetCDGame())
		{
			if ((!APP_GetFullScreen())&&(APP_GetHideMessage()))
				PRINTF("%s - Game is stay. CD: not ready.", APP_GetAppName());
			else
				PRINTF("Game is stay. CD: not ready.");
		}
	}
}


//Kitao追加
BOOL
CDROM_CheckPaused()
{
	return _AudioTrack.bPaused;
}


//Kitao追加
BOOL
CDROM_CheckPlaying()
{
	return _AudioTrack.bPlaying;
}


//Kitao追加
BOOL
CDROM_CheckInterrupt()
{
	return (_AudioTrack.bPlaying && _AudioTrack.bInterrupt);
}


BOOL
CDROM_IsCDROMEnabled()
{
	return _bCdromInit;
}


BOOL
CDROM_IsBRAMEnabled()
{
	return _bBRAMEnabled;
}


//Kitao追加
static void
execute_seek_sector()
{
	int		n;
	Sint32	t1, t2;
	char	fileName[MAX_PATH+1];
	FILE*	fp;
	int		track;
	int		lastTrack = CDIF_GetLastTrack();
	BOOL	bForceWait = FALSE;

	show_command_string("[SEEK SECTOR]", _CmdArgBuffer);

	CDROM_Stop(TRUE); //CDDA演奏中ならストップする。演奏中でない場合（_AudioTrack.bPlaying=FALSE）でもCDROM_MIXカウンターが０でない（CDDA読み込み最中の）こともあるため呼ぶ必要がある。
	//v1.21追加。フェード用の音量を初期化。レインボーアイランドの２面以降の開始時に必要。シーク時やプレイ時に入れると、ボス戦でやられたあとにCDDAが乱れる。
	_CurrentCdVolume = _InitialCdVolume;
	set_VOL();
	_bFadeOut = FALSE;
	_bFadeIn  = FALSE;
	_FadeCycle = 0;

	_Port[0] = 0xE8; //Kitao追加。v1.07。

	_ReadSectorAddr = (_CmdArgBuffer[1]<<16)|(_CmdArgBuffer[2]<<8)|_CmdArgBuffer[3]; //読み込み開始セクターをセット
	if (_CmdArgBuffer[4] == 0)
		_ReadByteCount = 2048 * 256; //カブキ一刀涼談で必要。v1.33
	else //通常
		_ReadByteCount = 2048 * _CmdArgBuffer[4]; //読み込み予定バイト数をセット

	_ReadBufferIndex = 2048*32; //初回にロードから開始するために2048*32にしておく

	//CDアクセスのウェイト機能を追加。ゲームによってはCDアクセス中にCPUパワーを使うものがあり、その場合ノンウェイトだと問題が起こる。
	if ((_bFastCD)&&(!_bAccessWait)) //高速モード(デフォルト)
	{
		if (_bSeekWait) //v0.61で追加。ヘッドシーク時間ぶんだけわずかにウェイトするモード
		{
			if (_bSeekWait3) //v2.34更新
				_CDAccessCount = (Sint32)(7159090.0 * 0.0150); //0.0150秒でダンジョンエクスプローラIIのOPデモがぴったり(口パクOK。※ウェイトが大きすぎると一瞬カクつくところも出てしまう)。短めのシークウェイト用。バスティールのビジュアルシーンOK。v2.17更新
			else if (_bSeekWait2) //v2.05追加
				_CDAccessCount = (Sint32)(7159090.0 * 0.0367); //0.0367秒でバスティールのビジュアルシーンOK(大きいと音ズレ＆稀に画面乱れ。小さいと音声途切れ＆音ズレ)。天使の詩２OK。短めのシークウェイト用。v2.34更新
			else //通常
				_CDAccessCount = (Sint32)(7159090.0 * 0.1667); //0.1667秒(約10フレームぶん)でファイティングストリートで勝負が決したときに(タイムアップ時で両者着地時が特に)ADPCMが発生されないことがある問題を解消。
		}
		else //通常(FastCD)の場合
			_CDAccessCount = 0; //通常は無難に安定＆高速なノーウェイトで。1でもあるとイース４のイベントシーン(OPの町へ歩いていくときのタイトルなど)で画面に１ラインのノイズが出たり、ラスタ割り込みを受け取れず画面が乱れる。※実機でも連続した部分のシークは瞬時に終わるからだろう。
	}
	else //実機並みのウェイトモード
		_CDAccessCount = (Sint32)(7159090.0 * 0.482); //0.482秒はダブルドラゴンIIで合わせた。ブランディッシュ，熱血物語もOK。

	//ぽっぷるメイルの場合ウェイトは0で。そうしないとビジュアルシーンで画面が崩れたりフリーズする可能性がある。
	if (_PopfulMail != 0)
		_CDAccessCount = 0;
	//ダブルドラゴンIIのスタート画面でRUNボタン押した直後のアクセスだけ実機並の遅さにする。スタートのADPCMを正常に最後まで再生。
	if (_bDoubleDragon)
	{
		if ((_ReadSectorAddr == 0x002150)||(_ReadSectorAddr == 0x00216C)||(_ReadSectorAddr == 0x002250))
			_CDAccessCount = (Sint32)(7159090.0 * 0.482); //0.482秒。小さいと画面が１フレーム乱れることあり。
	}
	//ダウンタウン熱血物語の本屋のお姉さんの「またのごらいてんを おまちしています」のADPCM音声を最後まで発声させる。
	if (_bNekketsu)
	{
		if (_ReadSectorAddr == 0x000B8E)
		{
				_CDAccessCount = (Sint32)(7159090.0 * 0.482); //0.482秒
				_bAccessWait = TRUE; //ここだけCDアクセスを実機並みの速度に
		}
		if (_ReadSectorAddr == 0x000EC4)
				_bAccessWait = FALSE; //元に戻す
	}
	//スーパーダライアスのスタート画面でRUNボタン押した直後のアクセスだけ実機並の遅さにする。スタートのADPCMを正常に最後まで再生。
	if (_bDarius)
	{
		if ((_ReadSectorAddr == 0x000E6D)||(_ReadSectorAddr == 0x000E85))
		{
			_CDAccessCount = (Sint32)(7159090.0 * 0.722); //0.722秒。このぐらいの"間"(イントロ終了後すぐ自機が動ける感じ)がちょうど良さそう。
			_bAccessWait = TRUE; //ここだけCDアクセスを実機並みの速度に
		}
		else
			_bAccessWait = FALSE; //元に戻す
	}
	//熱血レジェンドベースボーラーの音ズレを解消。v1.45
	if (_bBaseballer)
	{
		//スタート直後のビジュアルシーン
		if (_ReadSectorAddr == 0x004FC5) //多くの人々が失望し
		{		_CDAccessCount = (Sint32)(7159090.0 * 3.80); //3.80秒ウェイト
				bForceWait = TRUE; }//ここだけCDアクセスを実機並みの速度に
		if (_ReadSectorAddr == 0x005228) //委員長のセリフ
		{		_CDAccessCount = (Sint32)(7159090.0 * 1.40);
				bForceWait = TRUE; }
		if (_ReadSectorAddr == 0x005287) //暗黒おっさんのセリフ
		{		_CDAccessCount = (Sint32)(7159090.0 * 2.50);
				bForceWait = TRUE; }
		if (_ReadSectorAddr == 0x00533C) //暗黒おっさんのセリフ
		{		_CDAccessCount = (Sint32)(7159090.0 * 1.30);
				bForceWait = TRUE; }
		if (_ReadSectorAddr == 0x0053E8) //暗黒おっさんの娘のセリフ
		{		_CDAccessCount = (Sint32)(7159090.0 * 1.90);
				bForceWait = TRUE; }
		//ワイドヒル監督
		if (_ReadSectorAddr == 0x005672) //激しい戦いののち
		{		_CDAccessCount = (Sint32)(7159090.0 * 3.50);
				bForceWait = TRUE; }
		if (_ReadSectorAddr == 0x005777) //これこれあわてるな
		{		_CDAccessCount = (Sint32)(7159090.0 * 1.00);
				bForceWait = TRUE; }
		//大吾くん
		if (_ReadSectorAddr == 0x0058E0) //大吾わかったいくぜ
		{		_CDAccessCount = (Sint32)(7159090.0 * 1.30);
				bForceWait = TRUE; }
		//ジュリアーノ東森
		if (_ReadSectorAddr == 0x005C05) //スコーン
		{		_CDAccessCount = (Sint32)(7159090.0 * 1.80);
				bForceWait = TRUE; }
		//シェーラ
		if (_ReadSectorAddr == 0x005E12) //油断したのが
		{		_CDAccessCount = (Sint32)(7159090.0 * 1.80);
				bForceWait = TRUE; }
		//船
		if (_ReadSectorAddr == 0x005EB8) //あー！あれー！
		{		_CDAccessCount = (Sint32)(7159090.0 * 1.50);
				bForceWait = TRUE; }
		//チーフ・バッテンボー
		if (_ReadSectorAddr == 0x0062D2) //命がけの勝負アパッチ断らない
		{		_CDAccessCount = (Sint32)(7159090.0 * 0.80);
				bForceWait = TRUE; }
		if (_ReadSectorAddr == 0x006402) //負けた
		{		_CDAccessCount = (Sint32)(7159090.0 * 2.20);
				bForceWait = TRUE; }
		//アルティメット
		if (_ReadSectorAddr == 0x0099AE) //この面子で
		{		_CDAccessCount = (Sint32)(7159090.0 * 3.40);
				bForceWait = TRUE; }
		//アンデッド山本
		if (_ReadSectorAddr == 0x006D63) //ありがとう
		{		_CDAccessCount = (Sint32)(7159090.0 * 1.50);
				bForceWait = TRUE; }
		//ドラグーン
		if (_ReadSectorAddr == 0x00742D) //やらせるかー
		{		_CDAccessCount = (Sint32)(7159090.0 * 0.80);
				bForceWait = TRUE; }
		//ジュリアーノ東森
		if (_ReadSectorAddr == 0x00783C) //ようとうとう
		{		_CDAccessCount = (Sint32)(7159090.0 * 1.10);
				bForceWait = TRUE; }
		if (_ReadSectorAddr == 0x007929) //言うんじゃねー
		{		_CDAccessCount = (Sint32)(7159090.0 * 1.10);
				bForceWait = TRUE; }
		//アニーさん
		if (_ReadSectorAddr == 0x007C10) //とにかく今日はゆっくり
		{		_CDAccessCount = (Sint32)(7159090.0 * 3.70);
				bForceWait = TRUE; }
		if (_ReadSectorAddr == 0x007CFF) //男幸せになる
		{		_CDAccessCount = (Sint32)(7159090.0 * 0.60);
				bForceWait = TRUE; }
		if (_ReadSectorAddr == 0x007E0F) //正念場だ
		{		_CDAccessCount = (Sint32)(7159090.0 * 3.80);
				bForceWait = TRUE; }
		//アーロン
		if (_ReadSectorAddr == 0x00802A) //委員長！
		{		_CDAccessCount = (Sint32)(7159090.0 * 1.50);
				bForceWait = TRUE; }
		//アーロン続き
		if (_ReadSectorAddr == 0x0081D5) //勝ったな
		{		_CDAccessCount = (Sint32)(7159090.0 * 1.10);
				bForceWait = TRUE; }
		//ライゼル
		if (_ReadSectorAddr == 0x009BE5) //違うぞ！
		{		_CDAccessCount = (Sint32)(7159090.0 * 2.75);
				bForceWait = TRUE; }
		if ((_ReadSectorAddr >= 0x009CE5)&&
			(_ReadSectorAddr <= 0x009D05)) //笑わせるな
		{		_CDAccessCount = (Sint32)(7159090.0 * 3.80);
				bForceWait = TRUE; }
		//エンディング
		if (_ReadSectorAddr == 0x008D1D) //ほならみんな行くでー
		{		_CDAccessCount = (Sint32)(7159090.0 * 1.70);
				bForceWait = TRUE; }
		//スタッフロール
		if ((_ReadSectorAddr >= 0x0090FE)&&
			(_ReadSectorAddr <= 0x00931E))
		{		_CDAccessCount = (Sint32)(7159090.0 * 1.00); //実機と同じタイミングにできたかは未確認。最も印象深い感じになるように合わせた。
				bForceWait = TRUE; }
	}
	//AYAでウェイト。v2.25
	if (_bAya)
	{
		//病院での亜紀とのシーン
		if (_ReadSectorAddr == 0x002239) //タバコを吸う前
		{		_CDAccessCount = (Sint32)(7159090.0 * 7.00); //7秒ウェイト
				bForceWait = TRUE; }//ここだけCDアクセスを実機並みの速度に
		//手紙を見つけた後のシーン
		if (_ReadSectorAddr == 0x0020A3) //屋敷に入る前
		{		_CDAccessCount = (Sint32)(7159090.0 * 4.00); //4秒ウェイト
				bForceWait = TRUE; }
		//屋敷の中
		if (_ReadSectorAddr == 0x00204E) //ペンダントを振るところ
		{		_CDAccessCount = (Sint32)(7159090.0 * 3.00); //3秒ウェイト
				bForceWait = TRUE; }
	}
	//オルゴールでウェイト。v2.26
	if (_bOrgel)
	{
		//終盤、智名子とのシーン
		if (_ReadSectorAddr == 0x0054F9) //待ってください
		{		_CDAccessCount = (Sint32)(7159090.0 * 3.50); //3.5秒ウェイト
				bForceWait = TRUE; }//ここだけCDアクセスを実機並みの速度に
	}
	//パステルライムでウェイト。v2.40
	if (_bLime)
	{
		//先生が元に戻してくれた直後の会話シーン
		if (_ReadSectorAddr == 0x002186) //
		{		_CDAccessCount = (Sint32)(7159090.0 * 2.90); //2.90秒ウェイト。2.97以上入れると音乱れ。
				bForceWait = TRUE; }//ここだけCDアクセスを実機並みの速度に
	}
//Kitaoテスト用
//PRINTF("ReadSectorAddr = %X  ,  ReadByteCount = %X", (int)_ReadSectorAddr, _ReadByteCount);
/*
//if (WINMAIN_GetBreakTrap())
{
	char s[100];
	sprintf(s, "ReadSectorAddr = %x", (int)_ReadSectorAddr);
	int ok;
	ok = MessageBox(WINMAIN_GetHwnd(), s, "Test", MB_YESNO);
	if (ok != IDYES)
		WINMAIN_SetBreakTrap(FALSE);
}
*/

	_bReadSeekDone = FALSE; //cd_callbackにてPCE側にシーク完了が伝わるとTRUEになる
	_bReadBuffer2Full = FALSE;
	n = _ReadByteCount / 2048;
	if (n > 32)
		n = 32;
	t1 = timeGetTime();
	//v1.00。トラックデータをインストールしてある場合、インストールしたファイルから読み込みを行う。
	//       シャーロックホームズ１と２にて、かなり速い環境でないとうまく動かないことを解消。
	//       イース４もインストール(もしくは仮想ドライブ)することで、ADPCMの再生が安定する。
	track = 1;
	while (((_DiscInfo.TOC[track+1].lbaH<<16)+(_DiscInfo.TOC[track+1].lbaM<<8)+_DiscInfo.TOC[track+1].lbaL <= _ReadSectorAddr)&&
			(track <= lastTrack))
		track++;
	CDROM_SetInstallFileName(track, fileName);
	if ((fp = fopen(fileName, "rb")) != NULL)
	{	//isoファイルが存在した場合。v1.58からインストールされたHDDファイルの読み込みも別スレッドで行うようにした。こうしないと読み込みが長いときにADPCMに雑音が載ることがある。
		fclose(fp);
		CDIF_SeekDataHDD(_ReadBuffer2, track, (_ReadSectorAddr-((_DiscInfo.TOC[track].lbaH<<16)+(_DiscInfo.TOC[track].lbaM<<8)+_DiscInfo.TOC[track].lbaL))*2048, n, TRUE); //インストールしたファイルから読み込み。
	}
	else //通常の場合。CD-ROMドライブから読み込み
		CDIF_SeekData(_ReadBuffer2, _ReadSectorAddr, n, TRUE); //シークだけでなく、セカンダリバッファに先読みも行う。
	_ReadSectorAddr += n;
	//v0.83。CDドライブによってCDIF_SeekDataの時間が大きく違い、それもADPCM音声の音ずれの原因となる。なのでウェイトを入れてなるべくCDドライブ環境の誤差をなくすためここで調整。
	//		"ぽっぷるメイル"の音ずれ解消にも必要。
	if ((_bFastSeek)&&(_bFastCD)&&(!_bSeekWait)&&(!_bAccessWait)&&(!bForceWait)) //FastSeekモードのときはウェイトを全てカット
		_CDAccessCount = 0;
	else
	{
		if (!_bFastSeek)
		{
			//大きな読み込みの場合、ストリーム再生の場合がほとんどなので、実機のタイミングに近づけて音ずれを防止する。
			//ここでSleepすることによってオーディオスレッドだけが動き、ADPCMの再生タイミングを早めることができる。
			if (_ReadByteCount >= 0x8000)
			{
				t2 = 23 - (timeGetTime() - t1); //23ms。宇宙戦艦ヤマトとイース４で合わせた。大きいと音割れが起こる。24だと宇宙戦艦ヤマトでADPCMが途切れることがあった。v2.46更新
				if ((t2>=1)&&(t2<=23)) //タイマーカウンタのオーバーフローも考えて、この範囲のときだけSleepをおこなう。
					Sleep(t2);
			}
		}
	}
	//シーク成功の場合、cd_callbackにて通知される。この段階ではまだシーク(先読み)完了していない。
}


static void
execute_cd_playback_start_position()
{
	int		n;
	Uint8	tn;

	show_command_string("[PLAY AUDIO1]", _CmdArgBuffer);

	//再生中の場合、停止させる。コズミックファンタジーのOPで曲が始まってからRUN押した際に必要。_AudioTrack.bSeekDoneは初期化しない（ドラえもんで連続シーク時に初期化するとNG）。v2.32
	CDROM_Stop(FALSE);
	_AudioTrack.bPlaying  = FALSE;
	_AudioTrack.bPaused   = FALSE;

	/* 引数が全てゼロのときは停止させる */
	if ((_CmdArgBuffer[2] | _CmdArgBuffer[3] | _CmdArgBuffer[4]) == 0)
	{
		stop_track();
		_Port[3] |= SIGNAL_DONE;
		update_irq_state();
		return;
	}

	switch (_CmdArgBuffer[9] & 0xC0)
	{
		case 0x00:	// LBA指定モード
			lba2msf((_CmdArgBuffer[2]<<16)|(_CmdArgBuffer[3]<<8)|_CmdArgBuffer[4],
					(Uint8*)&_AudioTrack.minStart,
					(Uint8*)&_AudioTrack.secStart,
					(Uint8*)&_AudioTrack.frameStart);
			break;

		case 0x40:	// MSF指定モード
			_AudioTrack.minStart   = INT(_CmdArgBuffer[2]);
			_AudioTrack.secStart   = INT(_CmdArgBuffer[3]);
			_AudioTrack.frameStart = INT(_CmdArgBuffer[4]);
			break;

		case 0x80:	// トラック番号指定モード
			n = INT(_CmdArgBuffer[2]);
			_AudioTrack.minStart = _DiscInfo.TOC[n].min;
			_AudioTrack.secStart = _DiscInfo.TOC[n].sec;
			_AudioTrack.frameStart = _DiscInfo.TOC[n].frame;
			break;

		case 0xC0:	// ???
			//PRINTF("unknown mode");
			break;
	}
	_AudioTrack.searchMode = _CmdArgBuffer[1];

	//Kitao追加。サーチモードがオンのときのために、曲のエンドポジションを最終トラックの終わりに設定しておく。
	get_track_end_position(	get_last_track() + 1,
							(Uint8*)&_AudioTrack.minEnd,
							(Uint8*)&_AudioTrack.secEnd,
							(Uint8*)&_AudioTrack.frameEnd);

	//Kitao追加。サーチモードの値が設定されていればここで再生を開始。
	//PRINTF("PlayStartTrack = %X", _CmdArgBuffer[1]); //test用
	switch (_CmdArgBuffer[1])
	{
		case 0x00:	// Kitao更新。シーク＝「演奏開始(リピートなし)＋ポーズ」を行うようにした。ファージアスの邪皇帝のエンディングでスタッフロール時に音が鳴るようになった。スプリガンmkIIもこれを利用。v1.65
			tn = get_track_number_by_msf(_AudioTrack.minStart, _AudioTrack.secStart, _AudioTrack.frameStart) + 1;
			get_track_end_position(	tn,
									(Uint8*)&_AudioTrack.minEnd,
									(Uint8*)&_AudioTrack.secEnd,
									(Uint8*)&_AudioTrack.frameEnd); //１トラックのみを再生。リピートもしない。ファージアスの邪皇帝実機で確認済み。スプリガンmkIIはこのあと再生コマンドが行われてリピートする。
			_AudioTrack.playMode = 1;
			_AudioTrack.bRepeat = FALSE;
			_AudioTrack.bInterrupt = FALSE; //v2.29追加
			seek_track();
			break;

		case 0x01:	// Kitao更新。スタートポジション設定時は normal play ? ※エンドポジション設定時同様にリピートにすると内蔵CDプレイヤーで全曲終了後Ootakeごとフリーズ。
			_AudioTrack.playMode = 1;
			_AudioTrack.bRepeat = FALSE; //v1.65追加
			_AudioTrack.bInterrupt = FALSE; //v2.29追加
			play_track(FALSE);
			break;

		case 0x02:	// play, IRQ2 when finished ??
			_AudioTrack.playMode = 2;
			_AudioTrack.bRepeat = FALSE; //v1.65追加
			_AudioTrack.bInterrupt = TRUE;
			play_track(FALSE);
			break;

		case 0x03:	// play without repeat
			_AudioTrack.playMode = 3;
			_AudioTrack.bRepeat = FALSE; //v1.65追加
			_AudioTrack.bInterrupt = FALSE; //v2.29追加
			play_track(FALSE);
			break;
	}
}


static void
execute_cd_playback_end_position()
{
	show_command_string("[PLAY AUDIO2]", _CmdArgBuffer);

	switch (_CmdArgBuffer[9] & 0xC0)
	{
		case 0x00:	// LBA指定モード
			lba2msf((_CmdArgBuffer[2]<<16)|(_CmdArgBuffer[3]<<8)|_CmdArgBuffer[4],
					(Uint8*)&_AudioTrack.minEnd,
					(Uint8*)&_AudioTrack.secEnd,
					(Uint8*)&_AudioTrack.frameEnd);
			break;

		case 0x40:	// MSF指定モード
			_AudioTrack.minEnd   = INT(_CmdArgBuffer[2]);
			_AudioTrack.secEnd   = INT(_CmdArgBuffer[3]);
			_AudioTrack.frameEnd = INT(_CmdArgBuffer[4]);
			break;

		case 0x80:	// トラック番号指定モード
			get_track_end_position(	INT(_CmdArgBuffer[2]),
									(Uint8*)&_AudioTrack.minEnd,
									(Uint8*)&_AudioTrack.secEnd,
									(Uint8*)&_AudioTrack.frameEnd);
			break;

		case 0xC0:	// トラック終了位置?? 
			break;
	}

	//PRINTF("PlayEndTrack = %X", _CmdArgBuffer[1]); //test用
	switch (_CmdArgBuffer[1])
	{
		case 0x00:	// no operation ??
			_AudioTrack.playMode = 0;
			stop_track();
			_Port[0] = 0xD8;
			_ReadByteCount = 0;
			_CheckCountAfterRead = 2;
			_bCommandDone = TRUE;
			break;

		case 0x01:	// repeat play
			_AudioTrack.playMode = 1;
			_AudioTrack.bRepeat = TRUE;
			_AudioTrack.bInterrupt = FALSE; //v2.29追加
			play_track(FALSE);
			break;

		case 0x02:	// play, IRQ2 when finished ??
			_AudioTrack.playMode = 2;
			_AudioTrack.bRepeat = FALSE; //v1.65追加
			_AudioTrack.bInterrupt = TRUE;
			play_track(FALSE);
			break;

		case 0x03:	// play without repeat
			_AudioTrack.playMode = 3;
			_AudioTrack.bRepeat = FALSE; //v1.65追加
			_AudioTrack.bInterrupt = FALSE; //v2.29追加
			play_track(FALSE);
			break;
	}
}


static void
execute_pause_cd_playback()
{
	show_command_string("[PAUSE AUDIO]", _CmdArgBuffer);
	pause_track(TRUE);
}


static void
execute_read_subchannel_q()
{
	_ReadBuffer[2] = BCD(_AudioTrack.subqTrackNum);
	_ReadBuffer[3] = BCD(1); //Kitao追加。Index。内蔵のCDプレイヤーで必要。
	_ReadBuffer[4] = BCD(_AudioTrack.subqMinTrack);
	_ReadBuffer[5] = BCD(_AudioTrack.subqSecTrack);
	_ReadBuffer[6] = BCD(_AudioTrack.subqFrmTrack);
	_ReadBuffer[7] = BCD(_AudioTrack.subqMinTotal);
	_ReadBuffer[8] = BCD(_AudioTrack.subqSecTotal);
	_ReadBuffer[9] = BCD(_AudioTrack.subqFrmTotal);

	if (_AudioTrack.bPlaying && !_AudioTrack.bPaused) //Kitao更新
		_ReadBuffer[0] = 0;
	else if (_AudioTrack.bSeekDone) //Kitao更新
		_ReadBuffer[0] = 2;
	else
		_ReadBuffer[0] = 3;
/*
	PRINTF("[Q SUB CHANN] %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n",
			_ReadBuffer[0],_ReadBuffer[1],_ReadBuffer[2],_ReadBuffer[3],_ReadBuffer[4],
			_ReadBuffer[5],_ReadBuffer[6],_ReadBuffer[7],_ReadBuffer[8],_ReadBuffer[9]);
*/
	_ReadByteCount = 10;
	_Port[0] = 0xC8;
	_bCommandDone = TRUE;
}


static void
execute_get_dir_info()
{
	int		n; //Kitao追加

	switch (_CmdArgBuffer[1])
	{
		case 0:	// get first and last track number
			_ReadBuffer[0] = BCD(get_first_track());
			_ReadBuffer[1] = BCD(get_last_track());
			_ReadByteCount = 2;
			break;

		case 1:	// get total running time of disc  v0.78。Kitao更新。全曲再生時間を返すようにした。
			n = _DiscInfo.TOC[get_last_track()+1].min*60*75 + _DiscInfo.TOC[get_last_track()+1].sec*75 + _DiscInfo.TOC[get_last_track()+1].frame;
			_ReadBuffer[0] = BCD(n / 60 / 75);
			n -= (n / 60 / 75)*60*75;
			_ReadBuffer[1] = BCD(n / 75);
			_ReadBuffer[2] = BCD(n % 75);
			_ReadByteCount = 3;
			break;

		case 2:	// get track starting position and mode
		{
			//Kitao更新。_CmdArgBuffer[2]をINTに変換。ヴァリスIII,IVのオープニングデモの不具合を解消。
			n = INT(_CmdArgBuffer[2]);
			_ReadBuffer[0] = BCD(_DiscInfo.TOC[n].min);
			_ReadBuffer[1] = BCD(_DiscInfo.TOC[n].sec);
			_ReadBuffer[2] = BCD(_DiscInfo.TOC[n].frame);
			_ReadBuffer[3] = (_DiscInfo.TOC[n].isAudio == 1) ? 0 : 4;
			_ReadByteCount = 4;
			break;
		}
	}
	_Port[0] = 0xC8;
	_bCommandDone = TRUE;
}


static void
execute_command()
{
	//PRINTF("Execure Command = %X", _CmdArgBuffer[0]); //Test用

	switch (_CmdArgBuffer[0])
	{
		case 0x08:	// seek sector  Kitao更新。実機ではこの段階ではシークするだけのようだ。
			execute_seek_sector();
			break;

		case 0xD8:	// set audio playback start position
			execute_cd_playback_start_position();
			break;

		case 0xD9:	// set audio playback end position and start playing
			execute_cd_playback_end_position();
			break;

		case 0xDA:	// pause audio
			execute_pause_cd_playback();
			break;

		case 0xDD:	// read Q sub-channel
			execute_read_subchannel_q();
			break;

		case 0xDE:	// get dir info
			execute_get_dir_info();
			break;
	}
}


static void
receive_command(
	Uint8		data)
{
	if (data == 0x81)
	{
		// _ArgsLeft > 0 つまりコマンド引数を受付中のときは
		// リセットしない。
		if (_ArgsLeft == 0)
		{
			//PRINTF("$1801 <-- $81: cd reset?? $1800 = 0x00");
			_bCommandReset = TRUE;
			_bCommandReceived = FALSE;
			_bCommandDone = FALSE;
			_Port[0] = 0x00;

			//reset irq status
			_Port[3] = 0;
			update_irq_state();
			return;
		}
	}

	if (_bCommandReset)
	{
		_bCommandDone = FALSE;
		_Command = data;

		// コマンド系変数をリセットする。 
		_bCommandReset = FALSE;
		_bCommandReceived = FALSE;
		_CmdArgBufferIndex = 0;
		_ArgsLeft = 0;
		_ReadBufferIndex = 0;
		_ReadByteCount = 0;

		// コマンドを受け付ける。 
		switch (_Command)
		{
			case 0x00:	// TEST UNIT READY 
				_Port[0] = 0xD8;	// no more data needed
				_ArgsLeft = 0;
				_ReadByteCount = 0;
				_CheckCountAfterRead = 2;
				break;

			case 0x03:	// REQUEST SENSE
				_CmdArgBuffer[_CmdArgBufferIndex++] = 0x03;
				_ArgsLeft = 5; //v2.28追加
				break;

			case 0x08:	// read sector
				_CmdArgBuffer[_CmdArgBufferIndex++] = 0x08;
				_ArgsLeft = 5;
				break;

			case 0xD8:	// play audio (start position)
				_CmdArgBuffer[_CmdArgBufferIndex++] = 0xD8;
				_ArgsLeft = 9;
				break;

			case 0xD9:	// play audio (end position)
				_CmdArgBuffer[_CmdArgBufferIndex++] = 0xD9;
				_ArgsLeft = 9;
				break;

			case 0xDA:	// pause audio 
				_CmdArgBuffer[_CmdArgBufferIndex++] = 0xDA;
				_ArgsLeft = 9;
				_ReadByteCount = 0;
				_CheckCountAfterRead = 2;
				break;

			case 0xDD:
				_CmdArgBuffer[_CmdArgBufferIndex++] = 0xDD;
				_ArgsLeft = 9;
				_ReadByteCount = 10;
				break;

			case 0xDE:	// get CD directory info
				_CmdArgBuffer[_CmdArgBufferIndex++] = 0xDE;
				_ArgsLeft = 9;
				_ReadByteCount = 4;
				break;

			default:
				break;
		}
	}
	else
	{
		// 引数を受け付ける 
		_CmdArgBuffer[_CmdArgBufferIndex++] = data;
		
		if (--_ArgsLeft > 0)
		{
			_Port[0] = 0xD0;		// 0xD0: need more data
		}
		else
		{
			execute_command();
			_bCommandReceived = TRUE;
		}
	}
}


/*
	[フェードアウト]

	S 秒でフェードアウトするとき、
	クロックが C [Hz]で来るとすると、

		S * C [cycles]

	で完全に音量がゼロになればよい。

	現在の音量が V だとすると、

	S * C / V [cycles]

	ごとに V を１ずつ減じてゆけばよい。

	なお V を１ずつ減じずに、Dずつ減じる場合は

		S * C / (V / D) [cycles]

	ごとに V を減じる。
*/
static void
CDROM_FadeOut(
	Sint32	ms)
{
	if (ms == 0)
	{
		_CurrentCdVolume = 0;
		_VOL = 0.0;//Kitao追加
		_bFadeOut = FALSE;
		_bFadeIn  = FALSE;
		_FadeCycle = 0;
	}
	else if(_CurrentCdVolume > 0)
	{
		_FadeCycle = (Sint32)(((7159090.0 / ((double)_CurrentCdVolume / (double)_VolumeStep)) * (double)ms) / 1000.0);
		_bFadeOut	= TRUE;
		_bFadeIn	= FALSE;
	}
	else
	{
		_bFadeOut = FALSE;
		_bFadeIn  = FALSE;
		_FadeCycle = 0;
	}
}


static void
CDROM_FadeIn(
	Sint32	ms)
{
	if (ms == 0)
	{
		_CurrentCdVolume = _InitialCdVolume;
		set_VOL();//Kitao追加
		_bFadeOut = FALSE;
		_bFadeIn  = FALSE;
		_FadeCycle = 0;
	}
	else if (_InitialCdVolume - _CurrentCdVolume > 0)
	{
		_FadeCycle = (Sint32)(((7159090.0 / (((double)_InitialCdVolume - (double)_CurrentCdVolume) / (double)_VolumeStep)) * (double)ms) / 1000.0);
		_bFadeOut = FALSE;
		_bFadeIn  = TRUE;
	}
	else
	{
		_bFadeOut = FALSE;
		_bFadeIn  = FALSE;
		_FadeCycle = 0;
	}
}


void
CDROM_Write(
	Uint32		physAddr,
	Uint8		data)
{
	//PRINTF("CDROM_Write %X,%X", physAddr, data); //Kitaoテスト用
/*
if ((physAddr & 0xF) != 0xA)
{
	char s[100];
	sprintf(s,"CDROM_Write %X,%02X", physAddr, data);
	int ok;
	ok = MessageBox(WINMAIN_GetHwnd(), s, "Test", MB_YESNO); //Kitao Test
	if (ok != IDYES)
		WINMAIN_SetBreakTrap(FALSE);
}
*/
	switch (physAddr & 0xF)
	{
		case 0x0:		// $1800 write: resets the command input
			//PRINTF("CDROM_Write %X,%X", physAddr, data); //Kitaoテスト用
			//if (data == 0x81)
			{
				_Port[0] = 0xD0;	// = need more data
				_bCommandReset = TRUE;
				_bCommandReceived = FALSE;
				_bCommandDone = FALSE;
				_bAdpcmDma = FALSE;	//Kitao追加
				_bDriveBusy = FALSE;
				_ResetDelayCount = RESET_DELAY; //Kitao更新
				//PRINTF("$1800 <-- $81: command reset?? $1800 = 0xD0");
			
				/* reset irq status */
				_Port[3] = 0;
				update_irq_state();
			}
			//else
			//{
			//	PRINTF("CDROM_Write %X,%X", physAddr, data); //Kitaoテスト用
			//	_Port[0] = data;
			//}
			return;

		case 0x1:		// $1801
			_Port[1] = data;
			receive_command(data);
			return;

		case 0x2:		// $1802
			_Port[2] = data;
			update_irq_state();
			return;

//		case 0x3:		// read only
//			return;

		case 0x4:
			if (data & 2)
			{
				//Kitao追加。SELECT+RUNボタン時に音＆アクセスを止める必要がある。v1.59更新
				CDROM_Stop(TRUE);
				ADPCM_Reset();
				_bResetCDAccessCount = TRUE; //CDIFのスレッドがループに陥ってしまわないために必要。
				CDIF_WaitDeviceBusy(); //CDIFのスレッドで処理中(CDアクセス等)の場合、処理が終わるまで待つ。
				_bResetCDAccessCount = FALSE;
				
				// cd reset
				_bCommandReset = TRUE;
				_bCommandReceived = FALSE;
				_bCommandDone = FALSE;
				_bAdpcmDma = FALSE;	//Kitao追加
				_bDriveBusy = FALSE;
				_ResetDelayCount = RESET_DELAY; //Kitao更新
				stop_track();
				
				/* reset irq status */
				_Port[3] = 0;
				update_irq_state();
			}
			_Port[4] = data;
			return;

//		case 0x5:
//		case 0x6:
//			return;

		case 0x7:	// $1807: D7=1 enables backup ram 
			if (data & 0x80)
				_bBRAMEnabled = TRUE;
			return;
	
		case 0x8:
			ADPCM_SetAddrLo(data);
			return;

		case 0x9:
			ADPCM_SetAddrHi(data);
			return;

		case 0xA:
			//Kitao追加。シャーロックホームズの場合、動作タイミングがシビアなのでここでパッチ。ここでウェイトを入れると実機同様の音声が再現できる。
			if (_bSherlock)
				gCPU_ClockCount -= 551; //551＝シャーロックホームズ２で正常にADPCMが再生される値に合わせた。小さすぎると音飛び＆音が遅い。大きすぎると画面崩れ＆音が早い。
			ADPCM_WriteBuffer(data);
			return;

		case 0xB:	// ADPCM DMA  Kitao更新。DMA読み込みはCPUの動きと並列して行うようにした。
			if (data & 0x03) //Kitao追記。多くのゲームでbit1、餓狼伝説SPでbit0 が立っている。
			{
				_Port[0xC] |= 4;	//busy writing data
				_DmaClockCount = 0;
				if (_ReadByteCount == 0)
					_bAdpcmDmaStart = FALSE; //Kitao追加。ReadByteCountが0のときは、ReadByteCountがセットされるまで待つようにした。ロードス島戦記１が動くように。v1.31
				else
					_bAdpcmDmaStart = TRUE;
				_bAdpcmDma = TRUE;	//DMA読み込み開始
			}
			_Port[0xB] = data;
			return;

//		case 0xC:		// read-only
//			return;

		case 0xD:
			if (data & 0x80)
				ADPCM_Reset();

			//Kitao追加。bit1が立っていた場合、$180Aへの空読みの回数を設定し、そしてWriteAddrを設定する。v1.38
			if (data & 0x02)
			{
				if (data & 0x01)
					ADPCM_SetWriteAddr(0);//空書きしない。
				else
					ADPCM_SetWriteAddr(1);//１回空書きする。ロードス島戦記１。鏡の国のレジェンドで、デバッグ裏技の名前読み上げを再現。天外魔境ZIRIAのSCD版もこれで動くようになった。v1.38
			}
			
			if (data & 0x08)
				ADPCM_SetReadAddr();

			if (data & 0x10)
				ADPCM_SetLength();
			
			//Kitao更新。再生orストップは最後に判定するようにした。
			ADPCM_Repeat((data & 0x20) == 0x20);
			ADPCM_Play((data & 0x40) == 0x40); //Kitao更新。Bit6だけセットされた場合も再生するようにした。（スチームハーツ）
			
			_Port[0xD] = data;
			return;

		case 0xE:		// Set ADPCM playback rate
			ADPCM_SetFreq(data);
			return;

		case 0xF:
			/*
			{
				char s[100];
				sprintf(s,"CDROM_Write %X,%02X", physAddr, data);
				int ok;
				ok = MessageBox(WINMAIN_GetHwnd(), s, "Test", MB_YESNO); //Kitao Test
				if (ok != IDYES)
					WINMAIN_SetBreakTrap(FALSE);
			}
			*/
			switch (data & 0xF)
			{
				case 0:	//CD-DAとADPCMのフェードイン
					CDROM_FadeIn(100); //Kitao更新。実機のフェードイン聴こえ具合に合わせた。0だといきなり大音量でノイジーな場合がある。
					ADPCM_FadeIn(100); //Kitao更新
					break;

				case 1:	//Kitao追加。CD-DAのみフェードイン。イース３の宝箱をとった後で必要。
					CDROM_FadeIn(100);
					break;

				case 8: // Kitao追加。fade out CD (Short) & ADPCMフェードイン
					CDROM_FadeOut(1500); //Kitao更新。実機のフェードアウト聴こえ具合に合わせた
					ADPCM_FadeIn(100); //Kitao更新
					break;
				
				case 9: // fade out CD (Long)
					CDROM_FadeOut(5000); //Kitao更新。実機のフェードアウト聴こえ具合に合わせた
					break;

				case 0xA: // fade out ADPCM (Long)
					ADPCM_FadeOut(5000); //Kitao更新
					break;

				case 0xC: // Kitao追加。fade out CD (Short) & ADPCMフェードイン
					//銀河婦警伝説サファイアでADPCMが鳴るようになった。v0.83
					//ラストハルマゲドンもここが使われる。
					CDROM_FadeOut(1500);
					ADPCM_FadeIn(100);
					break; //Kitao追記。参考：0xCはスチームハーツでも使われている。CD-DAを鳴らせる鍵がここにある可能性も。

				case 0xD: // fade out CD (Short)
					CDROM_FadeOut(1500); //Kitao更新。実機のフェードアウト聴こえ具合に合わせた
					break;

				case 0xE: // fade out ADPCM (Short)
					ADPCM_FadeOut(1500); //Kitao更新
					break;
			}
			return;
	}
}


Uint8
CDROM_Read(
	Uint32	physAddr)
{
	Sint16	fadeOut; //Kitao追加
	Uint8	ret;

/*	if (physAddr != 0x1800)
	if (physAddr != 0x180C)
	if (physAddr != 0x180D)
	if (physAddr != 0x1803)
	if (physAddr != 0x1802)
	if (physAddr != 0x180A)
		PRINTF("CDROM_Read %X", physAddr); //Kitaoテスト用
*/
/*
if ((physAddr & 0xF) != 0x1)
if ((physAddr & 0xF) != 0x2)
if ((physAddr & 0xF) != 0xA)
if ((physAddr & 0xF) != 0xC)
//if ((physAddr & 0xF) != 0x8)
//if ((physAddr & 0xF) != 0x3)
//if ((physAddr & 0xF) != 0xD)
{
	char s[100];
	sprintf(s,"CDROM_Read %X", physAddr);
	int ok;
	ok = MessageBox(WINMAIN_GetHwnd(), s, "Test", MB_YESNO); //Kitao Test
	if (ok != IDYES)
		WINMAIN_SetBreakTrap(FALSE);
}
*/
	if ((physAddr & 0x18C0) == 0x18C0)
	{
		switch (physAddr & 0x18CF)
		{
			case 0x18C1: return 0xAA;
			case 0x18C2: return 0x55;
			case 0x18C3: return 0x00;
			case 0x18C5: return 0xAA;
			case 0x18C6: return 0x55;
			case 0x18C7: return 0x03;
		}
	}

	switch (physAddr & 0xF)
	{
		case 0x0:
			//PRINTF("Test%X P[0]=%02X, P[1]=%02X, P[2]=%02X, CReceiv=%d, CDone=%d, DBusy=%d", CPU_GetPC(),_Port[0],_Port[1],_Port[2],_bCommandReceived,_bCommandDone,_bDriveBusy); //Kitaoテスト用
			//PRINTF("Test$1800 P[0]=%02X, P[1]=%02X, P[2]=%02X, CReceiv=%d, CDone=%d, DBusy=%d", _Port[0],_Port[1],_Port[2],_bCommandReceived,_bCommandDone,_bDriveBusy); //Kitaoテスト用
			//PRINTF("Test$1800 CheckCountAfterRead=%d", _CheckCountAfterRead); //Kitaoテスト用
			if (_Port[2] & 0x80)
			{
				if (_CheckCountAfterRead == 0)
				{
					_bDriveBusy = FALSE;
					_Port[2] &= ~0xE0; //Kitao追加。ＧＳ美神でデモを見た後にスタートするのに必要。v1.45
					_Port[3] &= ~SIGNAL_DONE;
					update_irq_state();
				}
				return _Port[0] & ~SIGNAL_REQ;
			}
			else if (_bCommandReceived && !_bCommandDone)
			{
				if (_bDriveBusy) //ビジーを優先して処理するようにした。サイキックストームのタイトル画面で音の乱れが解消。v2.28
					return (_Port[0] & ~SIGNAL_REQ) | SIGNAL_IO; //普通にBUSYを返すとロードス島戦記のOPデモ後にフリーズ。SIGNAL_IO(0x08)を返すことでサイキックストームOK。
				else
					return _Port[0] & ~SIGNAL_REQ;
			}
			else if (_bDriveBusy)
			{
				return _Port[0] | SIGNAL_BUSY;
			}
			else if (_ResetDelayCount > 0)
			{
				--_ResetDelayCount;
				return _Port[0] & ~SIGNAL_REQ;
			}
			return _Port[0] | SIGNAL_REQ; //Ready

		case 0x1:
			if (_ReadByteCount > 0)
			{
				//Kitao追加。リード時のCPUウェイト。銀河婦警伝説サファイアのステージ開始時に必要。真怨霊戦記のタイトル画面が乱れないために必要。
				if ((_bReadWait)||(_bAccessWait)||(!_bFastCD)||(!_bFastSeek))
					gCPU_ClockCount -= _ReadWaitCount; //23。真怨霊戦記OK(19,23)。実機では46サイクル前後掛かると思われるが、シーク状況により誤差があるので、通常は少な目のウェイトにしておく。
													   //どうしてもウェイトが必要なゲームは、_ReadWaitCountを増やすか、_bAccesWait等をTRUEにして対処しよう。
				else //最低限のウェイトは入れておいたほうが良さそう。v1.59
					gCPU_ClockCount--; //-1。-2だとスプリガンの起動時ロゴ乱れ。-1以上で天外魔境II電源投入後デモの絹紹介終了時の下辺乱れが解消。-1以上でドラゴンスレイヤー英雄伝説の"CONTINE"選択後、真っ暗画面で止まることがあることを回避。大きいとスプリガンmkIIのメーカーロゴが揺れる。シャーロックホームズ２OK。v2.00,v2.40,v2.43
			}
			return read_1801();

		case 0x2: // read/write port (control port)
			return _Port[2];

		case 0x3:	// バックアップメモリを禁止する。 (read only)
					// status-read port
			_bBRAMEnabled = FALSE;
			/* switch left/right of digitized cd playback */
			ret = _Port[3] | SIGNAL_BRAM; //Kitao更新
			_Port[3] ^= SIGNAL_CDDAVOL;
			//PRINTF("Port[0x2] = %02X", _Port[0x2]);
			if (_Port[2] == 0)
				_Port[3] &= SIGNAL_CDDAVOL; //Kitao追加。Port[2]が0の場合、おそらくステータスはクリアされる。コブラ２の序盤墓場バトルシーンなどでフリーズを解消。v1.32
			update_irq_state();
			return ret;

		case 0x4:
			return _Port[4];

		case 0x5: //Kitao追加。CD-DAのボリュームメーター値（LowByte）
			if(_Port[3] & SIGNAL_CDDAVOL)
				fadeOut = (Sint16)_FadeOut1;
			else
				fadeOut = (Sint16)_FadeOut2;
			if (fadeOut < 0)
				fadeOut = -fadeOut;
			return ((Uint16)fadeOut & 0xFF);

		case 0x6: //Kitao追加。CD-DAのボリュームメーター値（HighByte）
			if(_Port[3] & SIGNAL_CDDAVOL)
				fadeOut = (Sint16)_FadeOut1;
			else
				fadeOut = (Sint16)_FadeOut2;
			if (fadeOut < 0)
				fadeOut = -fadeOut;
			return ((Uint16)fadeOut >> 8);

		case 0x7:
			// CD subchannel read
			//PRINTF("Port[7]Read!");//test
			return _Port[7];

		case 0x8:	// CD-ROM からセクタを読み出す。
			if (_ReadByteCount > 0)
			{
				//Kitao追加。リード時のCPUウェイト。銀河婦警伝説サファイアのステージ開始時に必要。真怨霊戦記のタイトル画面が乱れないために必要。
				if ((_bReadWait)||(_bAccessWait)||(!_bFastCD)||(!_bFastSeek))
					gCPU_ClockCount -= _ReadWaitCount; //23。真怨霊戦記OK(19,23)。実機では46サイクル前後掛かると思われるが、シーク状況により誤差があるので、通常は少な目のウェイトにしておく。
													   //どうしてもウェイトが必要なゲームは、_ReadWaitCountを増やすか、_bAccesWait等をTRUEにして対処しよう。
				else //最低限のウェイトは入れておいたほうが良さそう。v1.59
					gCPU_ClockCount--; //-1。-2だとスプリガンの起動時ロゴ乱れ。-1以上で天外魔境II電源投入後デモの絹紹介終了時の下辺乱れが解消。-1以上でドラゴンスレイヤー英雄伝説の"CONTINE"選択後、真っ暗画面で止まることがあることを回避。大きいとスプリガンmkIIのメーカーロゴが揺れる。シャーロックホームズ２OK。v2.00,v2.40,v2.43
			}
			return read_1808();

		case 0xA:
			return ADPCM_ReadBuffer();

		case 0xB:
			return _Port[0xB]; //Kitao更新。_Port[0xB]は、おそらく、ADPCMのDMA転送がすべて完了するとクリアされる。v1.07

		case 0xC:
			// D0: 現時点で ADPCM の再生が終了または停止されている場合は１ 
			// D2: CD --> DMA 転送中はゼロ？ (1: busy prepareing ADPCM data)
			// D3: ADPCM 再生中は１
			// D7: 前回の $180A の読み出し処理でＢＵＳＹの場合は、D7 = 1 となる。
			if (!ADPCM_IsPlaying())
			{
				_Port[0xC] = 1;
				_Port[0xC] &= ~8;
			}
			else
			{
				_Port[0xC] &= ~1;
				_Port[0xC] |= 8;
			}
			return _Port[0xC];

		case 0x9: //Kitao追記。0x9は何かに使われていそうな気も。
		case 0xD: //		   0xDのとき_Port[0xD]を返すとラストハルマゲドンで音がおかしくなる。
		case 0xE: //
		case 0xF: //
			return 0; //v1.62追加。存在するポートナンバーでは0を返し、それ以外では0xFFを返すようにした。
	}

	return 0xFF; //v1.62更新。I/O関連で、無いポートが指定されていたときは0xFFを返すようにした。
}


//Kitao追加。ぽっぷるメイル用の特別処理。将来的には特別処理なしでタイミングを合わせたい。
static void
popfulMailSub()
{
	if (_PlayTrackNum == 27) //オープニングデモの中間地点
	{
		if ((_CDDAReadByteCount < 8800000)&&(_CDDAReadByteCount > 7000000)) //オープニングデモの中間地点から数秒経ったところ
			VDC_SetAutoOverClock(0);//画面のちらつきを避けるため速度を元に戻す
		if (_CDDAReadByteCount < 7000000) //上の地点から数秒経ったところ
			VDC_SetAutoOverClock(200);//再び速度を速く
	}
	if (_PlayTrackNum == 29) //スタートデモの中間地点
	{
		if ((_CDDAReadByteCount < 3000000)&&(_CDDAReadByteCount > 1500000)) //スタートデモの中間地点から数秒経ったところ
			VDC_SetAutoOverClock(0);//画面のちらつきを避けるため速度を元に戻す
		if (_CDDAReadByteCount < 1500000) //上の地点から数秒経ったところ
			VDC_SetAutoOverClock(200);//再び速度を速く
	}
}

//Kitao更新。進めるクロック数を1クロック固定にした。戻り値を廃止した。
void
CDROM_AdvanceClock()
{
	//Kitao追加。CDアクセスウェイト処理
	if (_CDAccessCount > 0)
		--_CDAccessCount;

	//Kitao追加。実機に近いタイミングにするため、ADPCMと並列動作できる実装にした。
	if (_bAdpcmDma)
	{
		if (_bAdpcmDmaStart)
		{
			//PRINTF("ADPCM DMA = %d", _DmaClockCount);//Test用
			if ((++_DmaClockCount == 46)|| //7159090/150kb/1024=46.6。１バイトのDMA読み込みでかかるだいたいのクロック数（等倍速CD-ROM）。
				(_bFastAdpcmDma)) //46クロックだと遅くてタイミングが合わないゲーム用。聖戦士伝承で利用。v2.16
			{
				if (_ReadByteCount > 0)
				{
					ADPCM_WriteBuffer(read_1801());
					_Port[0xC] &= ~4; //DMA転送開始処理完了。１バイトの転送が開始された段階でフラグをクリアするようにした。ぽっぷるメイルの"つぶやき"のADPCM発声タイミングが合うようになった。v1.00
				}
				else
				{
					_bAdpcmDma = FALSE;
					_Port[0xB] = 0; //Kitao追加。餓狼伝説SPで必要
				}
				//CPUへの負荷(ADPCMメモリのアクセスがかち合うときだけ起こる？)。実機でも同様かは未確認。v2.11追加
				//ADPCM使用のビジュアルシーンで音がずれるゲームだけ個別処理。通常はノーウェイトにしておく。大きなウェイトを入れるとダブルドラゴン２や、お嬢様伝説ユナなど多くのソフトで乱れあり。
				if (_bYs4)
					gCPU_ClockCount -= 20; //20。イース４(特にフィーナとレアが初登場のシーン)の音声タイミングが最適に。21以上だとOP(ドギ＆ゴードン)で稀に画面乱れあった。v2.19更新
				//
				_DmaClockCount = 0;
			}
		}
	}

	//Kitao更新。秒単位ではなくフレーム(バイト)単位で曲の終了を判定するようにした。
	if (_AudioTrack.bPlaying && !_AudioTrack.bPaused)
	{
		if (_CddaIntroCount > 0) //CDDA再生直後なら。v1.23追加
		{
			if (--_CddaIntroCount == 0)
			{
				//バッファ秒のウェイト完了。曲の頭のバッファ秒が再生されたあとで、PCEのCPUへ伝える。これで実機に近い再生完了報告タイミングになったと思う。
				_Port[0] = 0xD8;
				_ReadByteCount = 0;
				_CheckCountAfterRead = 2;
				
				_bError = FALSE;
				_bCommandDone = TRUE;
			}
		}
		
		if (_CDDAReadByteCount == 0)
		{
			CDROM_Stop(TRUE); //ストップ処理
			if (_AudioTrack.bInterrupt)
			{
				_AudioTrack.bInterrupt = FALSE;
				if (_AudioTrack.playMode == 2)
				{
					_Port[3] |= SIGNAL_DONE;
					update_irq_state();
				}
			}
			if (_AudioTrack.bRepeat)
				play_track(TRUE);
		}
		
		if (_PopfulMail != 0) //ぽっぷるメイル用パッチ処理
			popfulMailSub();
	}
}


//Kitao更新。フェードアウト・フェードイン処理。APU_AdvanceClock()から呼ばれる。
void
CDROM_AdvanceFadeClock()
{
	if (_bFadeOut || _bFadeIn)
		if (++_FadeClockCount >= _FadeCycle)
		{
			_FadeClockCount = 0;
			if (_bFadeOut)
			{
				if (_CurrentCdVolume > 0)
				{
					_CurrentCdVolume -= _VolumeStep;
					set_VOL();//Kitao追加
					if (_CurrentCdVolume <= 0)
					{
						_CurrentCdVolume = 0;
						_VOL = 0.0;//Kitao追加
						_bFadeOut = FALSE;
					}
				}
			}
			else if (_bFadeIn)
			{
				if (_CurrentCdVolume < _InitialCdVolume)
				{
					_CurrentCdVolume += _VolumeStep;
					set_VOL();//Kitao追加
					if (_CurrentCdVolume >= _InitialCdVolume)
					{
						_CurrentCdVolume = _InitialCdVolume;
						set_VOL();//Kitao追加
						_bFadeIn = FALSE;
					}
				}
			}
		}
}


void
CDROM_SetCdVolume(
	Sint32		volume)
{
	Sint32	prevInitialCdVolume = _InitialCdVolume;

	_InitialCdVolume = volume;
	if ((!_bFadeOut)&&(!_bFadeIn)&&
		((!_AudioTrack.bPlaying)||(_CurrentCdVolume>0)|| //再生中でないか、現ボリュームが0より上なら。←音量をフェードして無音にしているソフトのために必要。
		 (prevInitialCdVolume==0))) //ボリュームを0に設定してあった場合は、上記の場合でも強制的にボリュームを設定する
	{
		_CurrentCdVolume = _InitialCdVolume;
		set_VOL();//Kitao追加
	}
	_VolumeStep      = _InitialCdVolume / 100;
	if (_VolumeStep == 0)
		_VolumeStep = 1;
}


//Kitao追加。isoのインストール(アクセス高速化のための取り込み)ファイル名をセットする
void
CDROM_SetInstallFileName(
	int		track,
	char*	pFileName)
{
	if (APP_GetInvalidateCdInstall())
		strcpy(pFileName,""); //CDインストールを強制無効にしている場合。v2.34追加
	else//通常
		sprintf(pFileName, "%sinstall\\%s\\Track%02d.iso", APP_GetAppPath(), TOCDB_GetGameTitle(), track);
}

//Kitao追加。wav(CD-DA)のインストール(アクセス高速化のための取り込み)ファイル名をセットする。v2.24追加
void
CDROM_SetInstallWavFileName(
	int		track,
	char*	pFileName)
{
	if (APP_GetInvalidateCdInstall())
		strcpy(pFileName,""); //CDインストールを強制無効にしている場合。v2.34追加
	else//通常
		sprintf(pFileName, "%sinstall\\%s\\Track%02d.wav", APP_GetAppPath(), TOCDB_GetGameTitle(), track);
}

//Kitao追加。cueのインストール(アクセス高速化のための取り込み)ファイル名をセットする。v2.24追加
void
CDROM_SetInstallCueFileName(
	char*	pFileName)
{
	sprintf(pFileName, "%sinstall\\%s\\%s.cue", APP_GetAppPath(), TOCDB_GetGameTitle(), TOCDB_GetGameTitle());
}

//Kitao追加。インストールに必要なHDD容量を計算する
void
CDROM_SetInstallRequiredHDD(
	BOOL	bFull) //フルインストール(音楽トラックもインストール)ならTRUEで呼ぶ。
{
	Sint32	r = 0;
	Sint32	s = 0;
	Sint32	sh = 0;
	Sint32	readSectorAddr;
	Sint32	endSectorAddr;
	int		track;
	int		lastTrack = CDIF_GetLastTrack();

	for (track=1; track<=lastTrack; track++)
	{
		if (_DiscInfo.TOC[track].isAudio == 0) //データトラックなら
		{
			readSectorAddr = (_DiscInfo.TOC[track].lbaH<<16)+(_DiscInfo.TOC[track].lbaM<<8)+_DiscInfo.TOC[track].lbaL;
			endSectorAddr = (_DiscInfo.TOC[track+1].lbaH<<16)+(_DiscInfo.TOC[track+1].lbaM<<8)+_DiscInfo.TOC[track+1].lbaL;
			if (_DiscInfo.TOC[track+1].isAudio == 1) //次のトラックがCDDAトラックなら
				endSectorAddr -= (_DiscInfo.TOC[track].min*75*60 + _DiscInfo.TOC[track].sec*75 + _DiscInfo.TOC[track].frame) - readSectorAddr; //プリギャップぶんを引く
			r += endSectorAddr - readSectorAddr;
		}
		else if (bFull) //フルインストールで音楽トラックなら。v2.24追加
		{
			sh += 44;//ヘッダ44byte
			readSectorAddr = (_DiscInfo.TOC[track].lbaH<<16)+(_DiscInfo.TOC[track].lbaM<<8)+_DiscInfo.TOC[track].lbaL;
			endSectorAddr = (_DiscInfo.TOC[track+1].lbaH<<16)+(_DiscInfo.TOC[track+1].lbaM<<8)+_DiscInfo.TOC[track+1].lbaL;
			if ((_DiscInfo.TOC[track+1].trackNum != 0)&&(_DiscInfo.TOC[track+1].isAudio == 0)) //次のトラックが存在するDATAトラックなら
				endSectorAddr -= (_DiscInfo.TOC[track].min*75*60 + _DiscInfo.TOC[track].sec*75 + _DiscInfo.TOC[track].frame) - readSectorAddr; //プリギャップぶんを引く
			s += endSectorAddr - readSectorAddr;
		}
	}

	_InstallRequiredHDD = ((r*2048 + sh + s*2352)/1024 + 1023) / 1024; //MB単位にする。
}

//Kitao追加。インストールに必要なHDD容量を返す
Sint32
CDROM_GetInstallRequiredHDD()
{
	return _InstallRequiredHDD;
}

//ゲームによってインストール時に特別処理が必要な場合に対処。v2.50
static void
checkCDInstallGame()
{
	char*	pGameTitle = TOCDB_GetGameTitle();

	if (strcmp(pGameTitle,"Star Parodia (J)") == 0) //スターパロジャー
	{
		if (!APP_GetCDSpeedDown()) //元々速度制限する設定の場合は処理しない
				CDIF_SetSpeed(4); //高速で読むとエラーが出やすいので、CD-ROMの読み込み速度をx4に制限する。※効果のないドライブもあり
	}
}

//ゲームによってインストール時に特別処理が必要な場合に対処。v2.50
static void
checkCDInstallGameEnd()
{
	char*	pGameTitle = TOCDB_GetGameTitle();

	if (strcmp(pGameTitle,"Star Parodia (J)") == 0) //スターパロジャー
	{
		if (!APP_GetCDSpeedDown()) //元々速度制限する設定の場合は処理しない
				CDIF_SetSpeed(0); //速度を元に戻す。
	}
}

//Kitao追加。インストール処理(アクセス高速化のための取り込み)。v1.00
BOOL
CDROM_CDInstall(
	HWND	hWnd,	//見出し表示のためにウィンドウのハンドルを設定
	BOOL	bFull)	//フルインストールの場合bFull=TRUEで呼ぶ
{
	unsigned char header[44] = {'R','I','F','F', 0,0,0,0, 'W','A','V','E', 'f','m','t',' ',
							    16,0,0,0, 1,0, 2,0, 0x44,0xAC,0,0, 0x10,0xB1,0x02,0, 4,0,16,0, //リニアPCM 16bitステレオ44100Hz
							    'd','a','t','a', 0,0,0,0};
	int		n;
	Sint32	readSectorAddr;
	Sint32	endSectorAddr;
	Uint8	readBuffer[2352*32];
	char	fileName[MAX_PATH+1];
	FILE*	fp;
	Sint32	track;
	Sint32	lastTrack = CDIF_GetLastTrack();
	char	buf[100];
	DWORD	size;
	Uint32	lba;
	Uint32	gap;
	Uint8	m,s,f;
	BOOL	bNextDataTrack; //v2.50追加

	//CDIFのスレッドで処理中(CDアクセス等)の場合、処理が終わるまで待つ。
	_bResetCDAccessCount = TRUE; //CDIFのスレッドがループに陥ってしまわないために必要。
	CDIF_WaitDeviceBusy();
	_bResetCDAccessCount = FALSE;

	checkCDInstallGame();//ゲームによってインストール時に特別処理が必要な場合に対処。v2.50

	for (track=1; track<=lastTrack; track++)
	{
		readSectorAddr = (_DiscInfo.TOC[track].lbaH<<16)+(_DiscInfo.TOC[track].lbaM<<8)+_DiscInfo.TOC[track].lbaL;
		endSectorAddr = (_DiscInfo.TOC[track+1].lbaH<<16)+(_DiscInfo.TOC[track+1].lbaM<<8)+_DiscInfo.TOC[track+1].lbaL;
		if (_DiscInfo.TOC[track].isAudio == 0) //データトラックなら
		{
			CDROM_SetInstallFileName(track, fileName);
			fp = fopen(fileName, "wb");
			if (fp != NULL)
			{
				if (_DiscInfo.TOC[track+1].isAudio == 1) //次のトラックがCDDAトラックなら
					endSectorAddr -= (_DiscInfo.TOC[track].min*75*60 + _DiscInfo.TOC[track].sec*75 + _DiscInfo.TOC[track].frame) - readSectorAddr; //プリギャップぶんを引く
				if (bFull) //フルインストール時
					sprintf(buf, "Ootake - Installing ISO Track %02d/%02d ( remainder %d ).", track,lastTrack, (int)(endSectorAddr-readSectorAddr)/100+1);
				else //データのみインストール時
					sprintf(buf, "Ootake - Installing ISO Track %02d ( remainder %d ).", track, (int)(endSectorAddr-readSectorAddr)/100+1);
				SetWindowText(hWnd, buf);
				UpdateWindow(hWnd);
				while (readSectorAddr < endSectorAddr)
				{
					if (CDINSTALL_GetResult() == -1) //中断された場合
					{
						//「途中までインストールしたファイル」を削除
						fclose(fp);
						remove(fileName);
						checkCDInstallGameEnd();//ゲームによってインストール時に特別処理が必要な場合に対処した設定を元に戻す。v2.50
						return FALSE;
					}
					n = endSectorAddr - readSectorAddr;
					if (n>32) n=32; //最大32セクターずつ読み込む
					memset(readBuffer, 0, 2048*32);
					_bInstallBusy = TRUE;
					CDIF_CDInstall(readBuffer, readSectorAddr, n, TRUE);
					readSectorAddr += n;
					if (bFull) //フルインストール時
						sprintf(buf, "Ootake - Installing ISO Track %02d/%02d ( remainder %d ).", track,lastTrack, (int)(endSectorAddr-readSectorAddr)/100);
					else //データのみインストール時
						sprintf(buf, "Ootake - Installing ISO Track %02d ( remainder %d ).", track, (int)(endSectorAddr-readSectorAddr)/100);
					SetWindowText(hWnd, buf);
					UpdateWindow(hWnd);
					while (_bInstallBusy) //セクターリード完了まで待つ
						Sleep(1);
					if (_bInstallSuccess == FALSE) //CD-ROMのハードエラーなどで読み込めなかった場合
					{
						if ((_DiscInfo.TOC[track+1].trackNum != 0)&&(_DiscInfo.TOC[track+1].isAudio == 0)) //次のトラックがDATAトラックなら（次トラックが音楽トラックor存在しない場合のエラーは無視）。v2.24
						{
							//「途中までインストールしたファイル」を削除
							fclose(fp);
							remove(fileName);
							MessageBox(hWnd, "CD-ROM Read Error. Please retry.    ", "Ootake", MB_OK);
							checkCDInstallGameEnd();
							return FALSE;
						}
					}
					fwrite(readBuffer, 2048*n, 1, fp);
				}
				fclose(fp);
			}
		}
		else if (bFull) //フルインストールで音楽トラックなら。v2.24追加
		{
			CDROM_SetInstallWavFileName(track, fileName);
			fp = fopen(fileName, "wb");
			if (fp != NULL)
			{
				if ((_DiscInfo.TOC[track+1].trackNum != 0)&&(_DiscInfo.TOC[track+1].isAudio == 0)) //次のトラックが存在するDATAトラックなら
				{
					endSectorAddr -= (_DiscInfo.TOC[track].min*75*60 + _DiscInfo.TOC[track].sec*75 + _DiscInfo.TOC[track].frame) - readSectorAddr; //プリギャップぶんを引く
					endSectorAddr -= 75; //終端ノイズ載りや、エラーでフリーズしないよう、１秒(75セクタ)手前で読み込みをやめる。スターパロジャーやイースシリーズ等のトラック１警告等で必要。v2.50
					bNextDataTrack = TRUE;
				}
				else
					bNextDataTrack = FALSE;
				sprintf(buf, "Ootake - Installing WAV Track %02d/%02d ( remainder %d ).", track,lastTrack, (int)(endSectorAddr-readSectorAddr)/100+1);
				SetWindowText(hWnd, buf);
				UpdateWindow(hWnd);
				size = 44 + (endSectorAddr-readSectorAddr)*2352 - 8; //ファイルサイズ(-8)を書き込み
				if (bNextDataTrack)
					size += 75*2352; //v2.51追加
				header[4] = byte(size);
				header[5] = byte(size >> 8);
				header[6] = byte(size >> 16);
				header[7] = byte(size >> 24);
				size -= 36; //波形データサイズを書き込み
				header[40] = byte(size);
				header[41] = byte(size >> 8);
				header[42] = byte(size >> 16);
				header[43] = byte(size >> 24);
				fwrite(header, 44, 1, fp); //WAVEヘッダーを出力
				while (readSectorAddr < endSectorAddr)
				{
					if (CDINSTALL_GetResult() == -1) //中断された場合
					{
						//「途中までインストールしたファイル」を削除
						fclose(fp);
						remove(fileName);
						checkCDInstallGameEnd();
						return FALSE;
					}
					n = endSectorAddr - readSectorAddr;
					if (n>32) n=32; //最大32セクターずつ読み込む
					memset(readBuffer, 0, 2352*32);
					_bInstallBusy = TRUE;
					CDIF_CDInstallWav(readBuffer, track, readSectorAddr, n, TRUE);
					readSectorAddr += n;
					sprintf(buf, "Ootake - Installing WAV Track %02d/%02d ( remainder %d ).", track,lastTrack, (int)(endSectorAddr-readSectorAddr)/100);
					SetWindowText(hWnd, buf);
					UpdateWindow(hWnd);
					while (_bInstallBusy) //セクターリード完了まで待つ
						Sleep(1);
					if (_bInstallSuccess == FALSE) //CD-ROMのハードエラーなどで読み込めなかった場合
					{
						if (_DiscInfo.TOC[track+1].isAudio != 0) //次のトラックがDATAトラック以外なら（次トラックがDATAトラックの場合のエラーは無視）
						{
							//「途中までインストールしたファイル」を削除
							fclose(fp);
							remove(fileName);
							MessageBox(hWnd, "CD-ROM Read Error. Please retry.    ", "Ootake", MB_OK);
							checkCDInstallGameEnd();
							return FALSE;
						}
					}
					fwrite(readBuffer, 2352*n, 1, fp);
				}
				if (bNextDataTrack)
				{	//未読み込み部分の１秒ぶんの無音を入れる
					memset(readBuffer, 0, 2352*25);
					for (n=1; n<=3; n++) //25x3=75frame
						fwrite(readBuffer, 2352*25, 1, fp);
				}
				fclose(fp);
			}
		}
	}

	//フルインストールの場合、cueファイルを作成する。v2.24追加
	if (bFull)
	{
		CDROM_SetInstallCueFileName(fileName);
		fp = fopen(fileName, "w");
		if (fp != NULL)
		{
			fprintf(fp, "REM ver%03d\n", APP_GetSoftVersion()); //v2.31追加。今後の不具合対策のため、インストール時のバージョンを保管。
			for (track=1; track<=lastTrack; track++)
			{
				lba = (_DiscInfo.TOC[track].lbaH<<16) + (_DiscInfo.TOC[track].lbaM<<8) + _DiscInfo.TOC[track].lbaL;
				gap = (_DiscInfo.TOC[track].min*75*60 + _DiscInfo.TOC[track].sec*75 + _DiscInfo.TOC[track].frame) - lba;
				if (_DiscInfo.TOC[track].isAudio == 0) //データトラックなら
				{
					fprintf(fp, "FILE \"Track%02d.iso\" BINARY\n", track);
					fprintf(fp, " TRACK %02d MODE1/2048\n", track);
					if (track >= 2)
						if (_DiscInfo.TOC[track-1].isAudio != 0) //ひとつ前のトラックが音楽トラックなら
					{
						m = (Uint8)(gap / 75 / 60);
						s = (Uint8)((gap - m * 75 * 60) / 75);
						f = (Uint8)(gap - (m * 75 * 60) - (s * 75));
						fprintf(fp, "  PREGAP %02d:%02d:%02d\n", m,s,f);
					}
					fprintf(fp, "  INDEX 01 00:00:00\n");
				}
				else //音楽トラックなら
				{
					fprintf(fp, "FILE \"Track%02d.wav\" WAVE\n", track);
					fprintf(fp, " TRACK %02d AUDIO\n", track);
					if (track >= 2)
						if (_DiscInfo.TOC[track-1].isAudio == 0) //ひとつ前のトラックがデータトラックなら
						{
							m = (Uint8)(gap / 75 / 60);
							s = (Uint8)((gap - m * 75 * 60) / 75);
							f = (Uint8)(gap - (m * 75 * 60) - (s * 75));
							fprintf(fp, "  PREGAP %02d:%02d:%02d\n", m,s,f);
						}
					fprintf(fp, "  INDEX 01 00:00:00\n");
				}
			}
			fclose(fp);
		}
		else
		{
			checkCDInstallGameEnd();
			return FALSE;
		}
	}

	checkCDInstallGameEnd();//ゲームによってインストール時に特別処理が必要な場合に対処した設定を元に戻す。v2.50
	return TRUE;
}

//Kitao追加。CDインストールしているかどうかを返す。v1.67
Sint32
CDROM_GetCDInstall()
{
	return _CDInstall;
}

//Kitao追加。CDアンインストール処理。v1.00
void
CDROM_CDUninstall()
{
	char	fileName[MAX_PATH+1];
	int		track;
	int		lastTrack = CDIF_GetLastTrack();
	char	trackChar[3];

	//CDIFのスレッドで処理中(CDアクセス等)の場合、処理が終わるまで待つ。
	_bResetCDAccessCount = TRUE; //CDIFのスレッドがループに陥ってしまわないために必要。
	CDIF_WaitDeviceBusy();
	_bResetCDAccessCount = FALSE;

	for (track=1; track<=lastTrack; track++)
	{
		sprintf(trackChar, "%02d", track);
		if (_DiscInfo.TOC[track].isAudio == 0) //データトラックなら
			CDROM_SetInstallFileName(track, fileName);
		else //音楽トラックなら
			CDROM_SetInstallWavFileName(track, fileName);
		remove(fileName); //削除
	}
	//cueファイルがあれば削除
	CDROM_SetInstallCueFileName(fileName);
	remove(fileName); //削除
}

//Kitao追加。インストールが最後まで完了されているかどうかをチェック。v1.00
//	戻り値…0=インストールされていない，1=データトラックがインストール済み，2=音楽トラックもフルインストール済み
Sint32
CDROM_CheckCDInstall()
{
	char	fileName[MAX_PATH+1];
	int		track;
	int		lastTrack = CDIF_GetLastTrack();
	char	trackChar[3];
	Sint32	ret = 2; //１トラックでも完了していなければ0にする。
	BOOL	bDataExist = FALSE;
	FILE*	fp;

	for (track=1; track<=lastTrack; track++)
	{
		sprintf(trackChar, "%02d", track);
		if (_DiscInfo.TOC[track].isAudio == 0) //データトラックなら
		{
			bDataExist = TRUE;
			CDROM_SetInstallFileName(track, fileName);
			if ((fp = fopen(fileName, "r")) != NULL) //ファイルがあれば
				fclose(fp);
			else //ファイルがなければ
			{
				ret = 0;
				break;
			}
		}
		else //音楽トラックなら
		{
			CDROM_SetInstallWavFileName(track, fileName);
			if ((fp = fopen(fileName, "r")) != NULL) //ファイルがあれば
				fclose(fp);
			else //ファイルがなければ
			{
				if (ret != 0)
					ret = 1;
			}
		}
	}

	if (!bDataExist) //データトラックor音楽トラックが1つも存在しなかった場合
		if (ret == 1)
			ret = 0;

	//フルインストールでcueファイルがあるかどうかをチェック
	if (ret == 2)
	{
		CDROM_SetInstallCueFileName(fileName);
		if ((fp = fopen(fileName, "r")) != NULL) //ファイルがあれば
		{
			CDIF_SetBadInstalled(fp); //正常にリッピングされたファイルかどうかを判断する。v2.31
			fclose(fp);
		}
		else //ファイルがなければ
			ret = 1;
	}

	return ret;
}


//Kitao追加。CDアクセスの高速化を行うかどうかを設定
void
CDROM_SetFastCD(
	BOOL fastCD)
{
	_bFastCD = fastCD;
}

//Kitao追加。CDアクセスの高速化を行うかどうかを返す
BOOL
CDROM_GetFastCD()
{
	return _bFastCD;
}


//Kitao追加。CDシークの高速化を行うかどうかを設定
void
CDROM_SetFastSeek(
	BOOL fastSeek)
{
	_bFastSeek = fastSeek;
}

//Kitao追加。CDシークの高速化を行うかどうかを返す
BOOL
CDROM_GetFastSeek()
{
	return _bFastSeek;
}


//Kitao追加。ボリュームミュート、ハーフなどをできるようにした。
void
CDROM_SetVolumeEffect(
	Uint32 volumeEffect)
{
	_CdVolumeEffect = volumeEffect; //※数値が大きいほどボリュームは小さくなる
	set_VOL();
}


//Kitao追加。CD-DA用のバッファの大きさをセットする。v1.00から512セクターに固定。
void
CDROM_SetCDDAReadBufferSize()
{
	_CDDAReadBufferSizeSector = 512; //CD-DA読み込みバッファの大きさ（単位はセクター数）。v1.00から512に固定。
	_CDDAReadBufferSizeSector2 = _CDDAReadBufferSizeSector / 2; //ハーフのサイズをここで計算しておく（高速化のため）

	_CDDAReadBufferSize = 2352 * _CDDAReadBufferSizeSector; //CD-DA読み込みバッファの大きさ（単位はバイト数）
	_CDDAReadBufferSize2 = _CDDAReadBufferSize / 2; //ハーフのサイズをここで計算しておく（高速化のため）

	ZeroMemory(_CDDAReadBuffer, sizeof(_CDDAReadBuffer)); //全バッファ領域をクリア
	_CDDAReadBufferIndex = 0; //すぐにCD-DAの追加読み込みがおこなわれるように、再生カーソル位置を先頭にする。
	_CDDAAjustCount = 0; //v2.32追加
}

//Kitao追加
Sint32
CDROM_GetCDDAReadBufferSize()
{
	return	_CDDAReadBufferSizeSector;
}


//Kitao追加。CDアクセスのウェイトを無効または有効にする。ステートセーブ＆ロード時にMainboard.cから使用。
void
CDROM_ResetCDAccessCount(
	BOOL	bReset) //無効にするならTRUE
{
	_bResetCDAccessCount = bReset;
}


//Kitao追加。LoadState後にCD-DAを鳴らす。※MAINBOARD_LoadState()から呼ばれる
void
CDROM_LoadPlayAudioTrack()
{
	if (!_bCdromInit)
		return;

	if (_AudioTrack.bPlaying)
		play_track2();//途中からCDDAを再生
}


//Kitao追加。CD-DAのテンポ微調整用のカウンタをクリアする。v2.32
void
CDROM_ClearCDDAAjustCount()
{
	_CDDAAjustCount = 0;
}


//Kitao追加。CD読み込み中かどうかを調べる。読み込み中(DMAは除く)の場合、残りバイト数を返す。
Sint32
CDROM_GetReadByteCount()
{
	if (_bAdpcmDma)
		return 0; //DMAで読み込み中は0を返す。
	else
		return _ReadByteCount;
}


//Kitao追加。ポートにデータをセット（デバッグ用）
void
CDROM_SetPort(
	Uint32		n,
	Uint8		data)
{
	_Port[n] = data;
}

//Kitao追加。ポートの内容を返す（デバッグ用）
Uint8
CDROM_GetPort(
	Uint32		n)
{
	return _Port[n];
}


//Kitao追加。CDインストールの状態を再確認する。CDフルインストール,アンインストール直後にApp.cppから使用。v2.35
void
CDROM_UpdateCDInstall()
{
	_CDInstall = CDROM_CheckCDInstall();
	if (_CDInstall == 2)
		_bWavTrackInstalled = TRUE;
	else
		_bWavTrackInstalled = FALSE;
}


// save variable
#define SAVE_V(V)	if (fwrite(&V, sizeof(V), 1, p) != 1)	return FALSE
#define LOAD_V(V)	if (fread(&V, sizeof(V), 1, p) != 1)	return FALSE
// save array
#define SAVE_A(A)	if (fwrite(A, sizeof(A), 1, p) != 1)	return FALSE
#define LOAD_A(A)	if (fread(A, sizeof(A), 1, p) != 1)		return FALSE
/*-----------------------------------------------------------------------------
	[SaveState]
		状態をファイルに保存します。 
-----------------------------------------------------------------------------*/
BOOL
CDROM_SaveState(
	FILE*		p)
{
	Sint32		CDDAReadBufferIndex = _CDDAReadBufferIndex; //v2.05追加。volatileだと型変換しないとfwrite,freadが使えないのでノーマル変数へ代入してセーブする。
	Sint32		CDDAReadByteCount = _CDDAReadByteCount;     //v2.04追加。同上。
	Sint32		CDDASectorAddr = _CDDASectorAddr;		    //v2.05追加。同上。
	Sint32		CDDASectorEndAddr = _CDDASectorEndAddr;		//v2.05追加。同上。
	BOOL		bReadBuffer2Full = _bReadBuffer2Full;		//v2.05追加。同上。
	BOOL		bCDDASeeking = _bCDDASeeking;				//v2.29追加。同上。
	BOOL		bReserve = FALSE; //Kitao追加。ゲーム毎のパッチ当てなどのための将来予約用

	if (p == NULL)
		return FALSE;

	SAVE_V(_bBRAMEnabled);
	SAVE_V(_ReadSectorAddr); //v0.61から追加
	SAVE_V(_ReadBufferIndex);
	SAVE_V(_ReadByteCount);
	SAVE_V(_Command);
	SAVE_V(_ArgsLeft);
	SAVE_V(_CmdArgBufferIndex);
	SAVE_V(_bCommandReset);
	SAVE_V(_bCommandDone);
	SAVE_V(_bDriveBusy);
	SAVE_V(_bError);
	SAVE_V(_ResetDelayCount);
	SAVE_V(_CheckCountAfterRead);
	fwrite((AudioTrack*)&_AudioTrack, sizeof(_AudioTrack), 1, p); //SAVE_V(AudioTrack);
	SAVE_V(_ClockCount);
	SAVE_V(_ClockCountSub); //Kitao追加。v0.78
	SAVE_V(_bCdromInit);
	SAVE_V(_PlayTrackNum); //Kitao追加。v0.64
	SAVE_V(_SubqMinTrack); //Kitao追加。v0.79
	SAVE_V(_SubqSecTrack); //Kitao追加。v0.79
	SAVE_V(_SubqFrmTrack); //Kitao追加。v0.79

	//Kitao追加。CDDA再生情報もセーブ
	if (APP_GetCDGame()) //v1.00から、CDゲームの場合だけセーブするようにした。
		SAVE_A(_CDDAReadBuffer); //v1.00からサイズが4倍に。
	SAVE_V(CDDAReadBufferIndex);
	SAVE_V(_CDDAReadBufferSize);
	SAVE_V(CDDAReadByteCount);
	SAVE_V(CDDASectorAddr);
	SAVE_V(CDDASectorEndAddr);

	SAVE_V(_CurrentCdVolume);//Kitao追加
	SAVE_V(_bFadeOut);//Kitao追加
	SAVE_V(_bFadeIn);//Kitao追加
	SAVE_V(_FadeCycle);//Kitao追加
	SAVE_V(_FadeClockCount);//Kitao追加
	SAVE_V(_bFastCD);//Kitao追加
	SAVE_V(_bFastSeek);//Kitao追加。v0.82
	SAVE_V(_CDAccessCount);//Kitao追加

	SAVE_A(_Port);
	SAVE_A(_ReadBuffer); //v0.80から64KBに
	SAVE_A(_ReadBuffer2); //v0.80追加
	SAVE_V(bReadBuffer2Full); //v0.80追加
	SAVE_A(_CmdArgBuffer);

	SAVE_V(_bAdpcmDma); //v0.64追加
	SAVE_V(_bAdpcmDmaStart); //v1.31追加
	SAVE_V(_DmaClockCount); //v0.64追加
	SAVE_V(_PopfulMail); //v0.64追加
	SAVE_V(_CddaIntroCount);//リザーブ領域をv1.23から使用
	
	SAVE_V(bCDDASeeking); //v2.29追加
	SAVE_V(_PrevCDDASectorAddr); //v2.29追加
	SAVE_V(_PrevCDDASectorEndAddr); //v2.29追加

	SAVE_V(bReserve);//Kitao追加。ゲーム毎のパッチ当てなどのための将来予約用
	SAVE_V(bReserve);//Kitao追加
	SAVE_V(bReserve);//Kitao追加
	SAVE_V(bReserve);//Kitao追加
	SAVE_V(bReserve);//Kitao追加
	SAVE_V(bReserve);//Kitao追加

	return TRUE;
}


/*-----------------------------------------------------------------------------
	[LoadState]
		状態をファイルから読み込みます。 
-----------------------------------------------------------------------------*/
BOOL
CDROM_LoadState(
	FILE*		p)
{
	Sint32		CDDAReadBufferIndex;	//v2.05追加。volatileだと型変換しないとfwrite,freadが使えないのでノーマル変数からロードする。
	Sint32		CDDAReadByteCount;		//v2.04追加。同上。
	Sint32		CDDASectorAddr;			//v2.05追加。同上。
	Sint32		CDDASectorEndAddr;		//v2.05追加。同上。
	BOOL		bReadBuffer2Full;		//v2.05追加。同上。
	BOOL		bCDDASeeking;			//v2.29追加。同上。
	BOOL		bReserve = FALSE; //Kitao追加。ゲーム毎のパッチ当てなどのための予約用
	Sint32		cddaBufferSize; //Kitao追加
	BOOL		bFastCD; //Kitao追加
	BOOL		bFastSeek; //Kitao追加

	if (p == NULL)
		return FALSE;

	LOAD_V(_bBRAMEnabled);
	if (MAINBOARD_GetStateVersion() >= 6) //Kitao追加。v0.61以降のセーブファイルなら
		LOAD_V(_ReadSectorAddr);
	LOAD_V(_ReadBufferIndex);
	LOAD_V(_ReadByteCount);
	LOAD_V(_Command);
	LOAD_V(_ArgsLeft);
	LOAD_V(_CmdArgBufferIndex);
	LOAD_V(_bCommandReset);
	LOAD_V(_bCommandDone);
	LOAD_V(_bDriveBusy);
	LOAD_V(_bError);
	LOAD_V(_ResetDelayCount);
	LOAD_V(_CheckCountAfterRead);
	fread((AudioTrack*)&_AudioTrack, sizeof(_AudioTrack), 1, p); //LOAD_V(AudioTrack);
	LOAD_V(_ClockCount);
	if (MAINBOARD_GetStateVersion() >= 12) //Kitao追加。v0.78以降のセーブファイルなら
	{
		LOAD_V(_ClockCountSub);
	}
	else
		_ClockCountSub = 0;
	LOAD_V(_bCdromInit);
	if (MAINBOARD_GetStateVersion() >= 8) //Kitao追加。v0.64以降のセーブファイルなら
	{
		LOAD_V(_PlayTrackNum);
	}
	else
		_PlayTrackNum = 0;
	if (MAINBOARD_GetStateVersion() >= 13) //Kitao追加。v0.79以降のセーブファイルなら
	{
		LOAD_V(_SubqMinTrack);
		LOAD_V(_SubqSecTrack);
		LOAD_V(_SubqFrmTrack);
	}

	//Kitao追加。CDDA再生情報もロード
	if (MAINBOARD_GetStateVersion() < 22) //Kitao追加。v1.00よりも前のセーブファイルなら
	{
		LOAD_A(_OldCDDAReadBuffer);
		//cddaBufferSizeが必ず異なるため、このあとバッファがクリアされる。
	}
	else
	{
		if (APP_GetCDGame()) //CDゲームの場合だけロード
		{
			LOAD_A(_CDDAReadBuffer);
		}
		else
			ZeroMemory(_CDDAReadBuffer, sizeof(_CDDAReadBuffer)); //全バッファ領域をクリア
	}
	LOAD_V(CDDAReadBufferIndex);
	_CDDAReadBufferIndex = CDDAReadBufferIndex; //v2.05追加
	_CDDAAjustCount = 0; //v2.32追加。環境違いで誤動作(小アジャストの環境で大きな値を読み込むと誤動作)することを防ぐため、アジャストカウンタは常にリセットで。
	LOAD_V(cddaBufferSize);
	if (cddaBufferSize != _CDDAReadBufferSize) //ステートセーブ時とバッファサイズが異なる場合は、バッファを破棄。
	{
		ZeroMemory(_CDDAReadBuffer, sizeof(_CDDAReadBuffer)); //全バッファ領域をクリア
		_CDDAReadBufferIndex = 0; //すぐにCD-DAの追加読み込みがおこなわれるように、再生カーソル位置を先頭にする。
	}

	LOAD_V(CDDAReadByteCount);
	_CDDAReadByteCount = CDDAReadByteCount; //v2.04追加
	LOAD_V(CDDASectorAddr);
	_CDDASectorAddr = CDDASectorAddr; //v2.05追加
	LOAD_V(CDDASectorEndAddr);
	_CDDASectorEndAddr = CDDASectorEndAddr; //v2.05追加

	LOAD_V(_CurrentCdVolume);//Kitao追加
	set_VOL(); //Kitao追加。v1.08
	LOAD_V(_bFadeOut);//Kitao追加
	LOAD_V(_bFadeIn);//Kitao追加
	LOAD_V(_FadeCycle);//Kitao追加
	LOAD_V(_FadeClockCount);//Kitao追加
	LOAD_V(bFastCD);//Kitao追加
	if ((_AutoFastCD == 0)&&(_AutoSlowCD == 0)) //ゲームごとの自動設定がされていないときだけ、設定を反映させる。
		_bFastCD = bFastCD;
	if (MAINBOARD_GetStateVersion() >= 15) //Kitao追加。v0.82以降のセーブファイルなら
	{
		LOAD_V(bFastSeek);
		if ((_AutoFastSeek == 0)&&(_AutoNormalSeek == 0)) //ゲームごとの自動設定がされていないときだけ、設定を反映させる。
			_bFastSeek = bFastSeek;
	}
	LOAD_V(_CDAccessCount);//Kitao追加

	LOAD_A(_Port);
	if (MAINBOARD_GetStateVersion() >= 14) //Kitao追加。v0.80以降のセーブファイルなら
	{
		LOAD_A(_ReadBuffer);
	}
	else if (MAINBOARD_GetStateVersion() >= 6) //Kitao追加。v0.61以降のセーブファイルなら
	{
		LOAD_A(_Old2ReadBuffer);
	}
	else //v0.60以前のセーブファイルなら
		LOAD_A(_OldReadBuffer);
	if (MAINBOARD_GetStateVersion() >= 14) //Kitao追加。v0.80以降のセーブファイルなら
	{
		LOAD_A(_ReadBuffer2);
		LOAD_V(bReadBuffer2Full);
		_bReadBuffer2Full = bReadBuffer2Full; //v2.05追加
	}
	LOAD_A(_CmdArgBuffer);

	if (MAINBOARD_GetStateVersion() >= 8) //Kitao追加。v0.64以降のセーブファイルなら
	{
		LOAD_V(_bAdpcmDma);
		if (MAINBOARD_GetStateVersion() >=29) //Kitao追加。v1.31以降のセーブファイルなら
		{
			LOAD_V(_bAdpcmDmaStart);
		}
		else
			_bAdpcmDmaStart = TRUE; //旧バージョンファイルだった場合TRUEで互換性が保てる。
		LOAD_V(_DmaClockCount);
		LOAD_V(_PopfulMail); //ダミー
	}
	else
		_bAdpcmDma = FALSE;
	LOAD_V(_CddaIntroCount);//Kitao追加。リザーブ領域をv1.23から使用。v1.22以下では0(FALSE)が書き込まれている。

	//Kitao追加。v2.29
	LOAD_V(bCDDASeeking); //v2.28以前の場合必ずFALSEになる
	_bCDDASeeking = bCDDASeeking;
	LOAD_V(_PrevCDDASectorAddr); //v2.28以前の場合必ず0になる
	LOAD_V(_PrevCDDASectorEndAddr); //v2.28以前の場合必ず0になる。

	LOAD_V(bReserve);//Kitao追加。ゲーム毎のパッチ当てなどのための将来予約用
	LOAD_V(bReserve);//Kitao追加
	LOAD_V(bReserve);//Kitao追加
	LOAD_V(bReserve);//Kitao追加
	LOAD_V(bReserve);//Kitao追加
	LOAD_V(bReserve);//Kitao追加

	//Kitao追加。ゲームごとの自動設定があればそれを優先する
	if (_bAutoDisconnectMultiTap)
		JOYPAD_ConnectMultiTap(FALSE);

	//Kitao追加。ゲーム開始時だけCDアクセスの速度を遅い(または速い)設定にしているゲームの場合、ステートロード時には「何か曲がかかった後の設定」にしておく。
	if (_AutoRetAccess == 1000)
		_bAccessWait = FALSE;
	if (_AutoRetSeekWait == 1000)
		_bSeekWait = FALSE;
	if (_AutoRetReadWait == 1000)
		_bReadWait = FALSE;
	if (_AutoNormalSeek == 1000)
		_bFastSeek = FALSE; //銀河婦警伝説サファイアで必要
	if (_AutoRetPlaySeekWait == 1000)
		_bPlaySeekWait = FALSE;

	if (_bGradiusII) //v1.21追加
	{
		INPUT_SetGradiusII(FALSE);
		_bGradiusII = FALSE;
	}

	return TRUE;
}

#undef SAVE_V
#undef SAVE_A
#undef LOAD_V
#undef LOAD_A
