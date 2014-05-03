/******************************************************************************
	[AudioOut.h]
		オーディオインタフェイスを定義します．

	Copyright (C) 2004 Ki

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
******************************************************************************/
#ifndef AUDIO_OUT_H_INCLUDED
#define AUDIO_OUT_H_INCLUDED

#include "TypeDefs.h"

#define AOUT_BUFFERRATE 4	//Kitao追加。バッファをいくつかに分けて持つ。細かく分けるほどバッファを小さくしても（音の遅延を改善しても）音質を保てる。３と４とで大きな違いを確認。最大63。※逆に大きくしすぎても処理が追いつかなくて駄目。４でBuffer2048が落としどころ？


BOOL
AOUT_Init(
	Sint32		soundType,	//Kitao追加
	Uint32		bufSize,	// in samples 
	Uint32		sampleRate,
	void		(*pCallBack)(int ch, Sint16* pBuf, Sint32 nSamples)); //Kitao更新。ch(チャンネルナンバー)を追加

void
AOUT_Play(
	BOOL		bPlay);

void
AOUT_Deinit();

//Kitao追加
void
AOUT_SetPlayStart();

//Kitao追加
void
AOUT_SetFpOutputWAV(
	FILE*	fp,
	Sint32	mode);

//Kitao追加
FILE*
AOUT_GetFpOutputWAV();

//Kitao追加
void
AOUT_SetOutputWavFileSize(
	DWORD	size);

//Kitao追加
DWORD
AOUT_GetOutputWavFileSize();

//Kitao追加
Sint32
AOUT_GetOutputWavWaitFinish();

//Kitao追加
Sint32
AOUT_GetPlay();


#endif // AUDIO_OUT_H_INCLUDED
