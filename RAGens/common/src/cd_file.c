#include <stdio.h>
#include <windows.h>
#include "cd_sys.h"
#include "cd_file.h"
#include "lc89510.h"
#include "cdda_mp3.h"
#include "star_68k.h"
#include "rom.h"
#include "mem_S68K.h"

struct _file_track Tracks[100];

char cp_buf[2560];
char Track_Played;

int FILE_Init(void)
{
	MP3_Init();
	Unload_ISO();

	return 0;
}


void FILE_End(void)
{
	Unload_ISO();
}


int Load_ISO(char *buf, char *iso_name)
{
	HANDLE File_Size;
	int i, j, num_track, Cur_LBA;
	FILE *tmp_file;
	char tmp_name[1024], tmp_ext[10];
	char exts[20][16] = {
		"%02d.mp3", " %02d.mp3", "-%02d.mp3", "_%02d.mp3", " - %02d.mp3",
		"%d.mp3", " %d.mp3", "-%d.mp3", "_%d.mp3", " - %d.mp3",
		"%02d.wav", " %02d.wav", "-%02d.wav", "_%02d.wav", " - %02d.wav",
		"%d.wav", " %d.wav", "-%d.wav", "_%d.wav", " - %2d.wav"};
	
	Unload_ISO();

	if (Detect_Format(iso_name) == SEGACD_IMAGE + 1) Tracks[0].Type = TYPE_BIN;
	else if (Detect_Format(iso_name) == SEGACD_IMAGE) Tracks[0].Type = TYPE_ISO;
	else return -2;

	File_Size = CreateFile(iso_name, GENERIC_READ, FILE_SHARE_READ, 
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	Tracks[0].Lenght = GetFileSize(File_Size, NULL);

	if (Tracks[0].Type == TYPE_ISO) Tracks[0].Lenght >>= 11;	// size in sectors
	else Tracks[0].Lenght /= 2352;								// size in sectors

	CloseHandle(File_Size);

	fopen_s(&Tracks[0].F, iso_name, "rb");

	if (Tracks[0].F == NULL)
	{
		Tracks[0].Type = 0;
		Tracks[0].Lenght = 0;
		return -1;
	}

	if (Tracks[0].Type == TYPE_ISO) fseek(Tracks[0].F, 0x100, SEEK_SET);
	else fseek(Tracks[0].F, 0x110, SEEK_SET);

	fread(buf, 1, 0x200, Tracks[0].F);
	fseek(Tracks[0].F, 0, SEEK_SET);

	SCD.TOC.First_Track = 1;

	SCD.TOC.Tracks[0].Num = 1;
	SCD.TOC.Tracks[0].Type = 1;				// DATA

	SCD.TOC.Tracks[0].MSF.M = 0;
	SCD.TOC.Tracks[0].MSF.S = 2;
	SCD.TOC.Tracks[0].MSF.F = 0;

#ifdef DEBUG_CD
	fprintf(debug_SCD_file, "\nTrack 0 - %02d:%02d:%02d ", SCD.TOC.Tracks[0].MSF.M, SCD.TOC.Tracks[0].MSF.S, SCD.TOC.Tracks[0].MSF.F);
	if (SCD.TOC.Tracks[0].Type) fprintf(debug_SCD_file, "DATA\n");
	else fprintf(debug_SCD_file, "AUDIO\n");
#endif

	Cur_LBA = Tracks[0].Lenght;				// Size in sectors

	strcpy_s(tmp_name, 1024, iso_name);

	for(num_track = 2, i = 0; i < 100; i++)
	{
		for(j = 0; j < 20; j++)
		{
			tmp_name[strlen(iso_name) - 4] = 0;
			wsprintf(tmp_ext, exts[j], i);
			strcat_s(tmp_name, 1024, tmp_ext);

			fopen_s(&tmp_file, tmp_name, "rb");

			if (tmp_file)
			{
				float fs;

				File_Size = CreateFile(tmp_name, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

				fs = (float) GetFileSize(File_Size, NULL);				// used to calculate lenght

				Tracks[num_track - SCD.TOC.First_Track].F = tmp_file;
				
				SCD.TOC.Tracks[num_track - SCD.TOC.First_Track].Num = num_track;
				SCD.TOC.Tracks[num_track - SCD.TOC.First_Track].Type = 0;			// AUDIO

				LBA_to_MSF(Cur_LBA, &(SCD.TOC.Tracks[num_track - SCD.TOC.First_Track].MSF));

#ifdef DEBUG_CD
				fprintf(debug_SCD_file, "Track %i - %02d:%02d:%02d ", num_track - SCD.TOC.First_Track, SCD.TOC.Tracks[num_track - SCD.TOC.First_Track].MSF.M, SCD.TOC.Tracks[num_track - SCD.TOC.First_Track].MSF.S, SCD.TOC.Tracks[num_track - SCD.TOC.First_Track].MSF.F);
				if (SCD.TOC.Tracks[num_track - SCD.TOC.First_Track].Type) fprintf(debug_SCD_file, "DATA\n");
				else fprintf(debug_SCD_file, "AUDIO\n");
#endif
		
				if (j < 10)
				{
					// MP3 File
					Tracks[num_track - SCD.TOC.First_Track].Type = TYPE_MP3;
					fs /= (float) (MP3_Get_Bitrate(Tracks[num_track - 1].F) >> 3);
					fs *= 75;
					Tracks[num_track - SCD.TOC.First_Track].Lenght = (int) fs;
					Cur_LBA += Tracks[num_track - SCD.TOC.First_Track].Lenght;
				}
				else
				{
					// WAV File
					Tracks[num_track - SCD.TOC.First_Track].Type = TYPE_WAV;
					Tracks[num_track - SCD.TOC.First_Track].Lenght = 1000;
					Cur_LBA += Tracks[num_track - SCD.TOC.First_Track].Lenght;
				}

				j = 1000;
				num_track++;
			}
		}
	}
			
/*	
//	Faking some audios tracks if no present

	if (num_track == 2)
	{
		for(; num_track < 95; num_track++)
		{
			SCD.TOC.Tracks[num_track - SCD.TOC.First_Track].Num = num_track;
			SCD.TOC.Tracks[num_track - SCD.TOC.First_Track].Type = 0;			// AUDIO

			LBA_to_MSF(Cur_LBA, &(SCD.TOC.Tracks[num_track - SCD.TOC.First_Track].MSF));

			Cur_LBA += 100;
		}
	}
*/	
	SCD.TOC.Last_Track = num_track - 1;

	SCD.TOC.Tracks[num_track - SCD.TOC.First_Track].Num = num_track;
	SCD.TOC.Tracks[num_track - SCD.TOC.First_Track].Type = 0;

	LBA_to_MSF(Cur_LBA, &(SCD.TOC.Tracks[num_track - SCD.TOC.First_Track].MSF));

#ifdef DEBUG_CD
	fprintf(debug_SCD_file, "End CD - %02d:%02d:%02d\n\n", SCD.TOC.Tracks[num_track - SCD.TOC.First_Track].MSF.M, SCD.TOC.Tracks[num_track - SCD.TOC.First_Track].MSF.S, SCD.TOC.Tracks[num_track - SCD.TOC.First_Track].MSF.F);
#endif

	return 0;
}


void Unload_ISO(void)
{
	int i;
	
	Track_Played = 99;

	for(i = 0; i < 100; i++)
	{
		if (Tracks[i].F) fclose(Tracks[i].F);
		Tracks[i].F = NULL;
		Tracks[i].Lenght = 0;
		Tracks[i].Type = 0;
	}
}


int FILE_Read_One_LBA_CDC(void)
{
	int where_read;
	
	if (CDD.Control & 0x0100)					// DATA
	{
		if (Tracks[0].F == NULL) return -1;

		if (SCD.Cur_LBA < 0) where_read = 0;
		else if (SCD.Cur_LBA >= Tracks[0].Lenght) where_read = Tracks[0].Lenght - 1;
		else where_read = SCD.Cur_LBA;

		if (Tracks[0].Type == TYPE_ISO) where_read <<= 11;
		else where_read = (where_read * 2352 + 16);

//		memset(cp_buf, 0, 2048);

		fseek(Tracks[0].F, where_read, SEEK_SET);
		fread(cp_buf, 1, 2048, Tracks[0].F);

#ifdef DEBUG_CD
		fprintf(debug_SCD_file, "\n\nRead file CDC 1 data sector :\n");
#endif
	}
	else										// AUDIO
	{
		int rate, channel;
		
		if (Tracks[SCD.Cur_Track - SCD.TOC.First_Track].Type == TYPE_MP3)
		{
			MP3_Update(cp_buf, &rate, &channel, 0);
			Write_CD_Audio((short *) cp_buf, rate, channel, 588);
		}

#ifdef DEBUG_CD
		fprintf(debug_SCD_file, "\n\nRead file CDC 1 audio sector :\n");
#endif
	}

	// Update CDC stuff

	CDC_Update_Header();

	if (CDD.Control & 0x0100)			// DATA track
	{
		if (CDC.CTRL.B.B0 & 0x80)		// DECEN = decoding enable
		{
			if (CDC.CTRL.B.B0 & 0x04)	// WRRQ : this bit enable write to buffer
			{
				// CAUTION : lookahead bit not implemented

				SCD.Cur_LBA++;

				CDC.WA.N = (CDC.WA.N + 2352) & 0x7FFF;		// add one sector to WA
				CDC.PT.N = (CDC.PT.N + 2352) & 0x7FFF;

				memcpy(&CDC.Buffer[CDC.PT.N + 4], cp_buf, 2048);
				memcpy(&CDC.Buffer[CDC.PT.N], &CDC.HEAD, 4);

#ifdef DEBUG_CD
				fprintf(debug_SCD_file, "\nRead -> WA = %d  Buffer[%d] =\n", CDC.WA.N, CDC.PT.N & 0x3FFF);
				fprintf(debug_SCD_file, "Header 1 = %.2X %.2X %.2X %.2X\n", CDC.HEAD.B.B0, CDC.HEAD.B.B1, CDC.HEAD.B.B2, CDC.HEAD.B.B3);
//				fwrite(Buf_Read, 1, 2048, debug_SCD_file);
//				fprintf(debug_SCD_file, "\nCDC buffer =\n");
//				fwrite(&CDC.Buffer[CDC.PT.N], 1, 2052, debug_SCD_file);
				fprintf(debug_SCD_file, "Header 2 = %.2X %.2X %.2X %.2X --- %.2X %.2X\n\n", CDC.Buffer[(CDC.PT.N + 0) & 0x3FFF], CDC.Buffer[(CDC.PT.N + 1) & 0x3FFF], CDC.Buffer[(CDC.PT.N + 2) & 0x3FFF], CDC.Buffer[(CDC.PT.N + 3) & 0x3FFF], CDC.Buffer[(CDC.PT.N + 4) & 0x3FFF], CDC.Buffer[(CDC.PT.N + 5) & 0x3FFF]);
#endif
			}

			CDC.STAT.B.B0 = 0x80;

			if (CDC.CTRL.B.B0 & 0x10)		// determine form bit form sub header ?
			{
				CDC.STAT.B.B2 = CDC.CTRL.B.B1 & 0x08;
			}
			else
			{
				CDC.STAT.B.B2 = CDC.CTRL.B.B1 & 0x0C;
			}

			if (CDC.CTRL.B.B0 & 0x02) CDC.STAT.B.B3 = 0x20;	// ECC done
			else CDC.STAT.B.B3 = 0x00;	// ECC not done

			if (CDC.IFCTRL & 0x20)
			{
				if (Int_Mask_S68K & 0x20) sub68k_interrupt(5, -1);

#ifdef DEBUG_CD
				fprintf(debug_SCD_file, "CDC - DEC interrupt\n");
#endif

				CDC.IFSTAT &= ~0x20;		// DEC interrupt happen
				CDC_Decode_Reg_Read = 0;	// Reset read after DEC int
			}
		}
	}
	else
	{
		SCD.Cur_LBA++;

		CDC.WA.N = (CDC.WA.N + 2352) & 0x7FFF;		// add one sector to WA
		CDC.PT.N = (CDC.PT.N + 2352) & 0x7FFF;

		if (CDC.CTRL.B.B0 & 0x80)		// DECEN = decoding enable
		{
			if (CDC.CTRL.B.B0 & 0x04)	// WRRQ : this bit enable write to buffer
			{
				// CAUTION : lookahead bit not implemented

				memcpy(&CDC.Buffer[CDC.PT.N], cp_buf, 2352);
			}

			CDC.STAT.B.B0 = 0x80;

			if (CDC.CTRL.B.B0 & 0x10)		// determine form bit form sub header ?
			{
				CDC.STAT.B.B2 = CDC.CTRL.B.B1 & 0x08;
			}
			else
			{
				CDC.STAT.B.B2 = CDC.CTRL.B.B1 & 0x0C;
			}

			if (CDC.CTRL.B.B0 & 0x02) CDC.STAT.B.B3 = 0x20;	// ECC done
			else CDC.STAT.B.B3 = 0x00;	// ECC not done

			if (CDC.IFCTRL & 0x20)
			{
				if (Int_Mask_S68K & 0x20) sub68k_interrupt(5, -1);

#ifdef DEBUG_CD
				fprintf(debug_SCD_file, "CDC - DEC interrupt\n");
#endif

				CDC.IFSTAT &= ~0x20;		// DEC interrupt happen
				CDC_Decode_Reg_Read = 0;	// Reset read after DEC int
			}
		}
	}

	return 0;
}


int FILE_Play_CD_LBA(void)
{
	int Track_LBA_Pos;

#ifdef DEBUG_CD
	fprintf(debug_SCD_file, "Play FILE Comp\n");
#endif

	if (Tracks[SCD.Cur_Track - SCD.TOC.First_Track].F == NULL)
	{
		return 1;
	}

	Track_LBA_Pos = SCD.Cur_LBA - Track_to_LBA(SCD.Cur_Track);
	if (Track_LBA_Pos < 0) Track_LBA_Pos = 0;

	if (Tracks[SCD.Cur_Track - SCD.TOC.First_Track].Type == TYPE_MP3)
	{
		MP3_Play(SCD.Cur_Track - SCD.TOC.First_Track, Track_LBA_Pos);
	}
	else if (Tracks[SCD.Cur_Track - SCD.TOC.First_Track].Type == TYPE_WAV)
	{
		return 2;
	}
	else
	{
		return 3;
	}

	return 0;
}