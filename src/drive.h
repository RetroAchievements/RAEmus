#ifndef DRIVE_H_INCLUDED
#define DRIVE_H_INCLUDED

#include <stdio.h>
#include "file-op.h"

#include "initval.h"
/*	ドライブの数 NR_DRIVE は、initval.h で定義済 */
/*	イメージの数 NR_IMAGE は、initval.h で定義済 */




	/**** ドライブ制御ワーク ****/

typedef	struct{

  OSD_FILE *fp;			/* FILE ポインタ			*/

  char	read_only;		/* リードオンリーでファイルを開いたら真	*/
  char	over_image;		/* イメージ数が多過ぎる時に真		*/
  char	detect_broken_image;	/* 壊れたイメージが見つかったら真	*/
  char	empty;			/* ドライブに空を指定しているなら真	*/

  int	selected_image;		/* 選択しているイメージ番号 (0〜)	*/
  int	image_nr;		/* ファイル内のイメージ数   (1〜)	*/

  struct{			/* ファイル内の全イメージの情報		*/
    char	name[17];	/*	イメージ名			*/
    char	protect;	/*	プロテクト			*/
    char	type;		/*	ディスクタイプ			*/
    long	size;		/*	サイズ				*/
  }image[ MAX_NR_IMAGE ];

				/* ここから、選択中イメージのワーク	*/

  int	track;			/* 現在のトラック番号			*/
  int	sec_nr;			/* トラック内のセクタ数			*/
  int	sec;			/* 現在のセクタ番号			*/

  long	sec_pos;		/* セクタ  の現在位置			*/
  long	track_top;		/* トラックの先頭位置			*/
  long	disk_top;		/* ディスクの先頭位置			*/
  long	disk_end;		/* イメージの終端位置			*/

  char	protect;		/* ライトプロテクト			*/
  char	type;			/* ディスクタイプ			*/

				/* ファイル名				*/

  /* char	filename[ QUASI88_MAX_FILENAME ];*/

} PC88_DRIVE_T;



extern	PC88_DRIVE_T	drive[ NR_DRIVE ];




	/**** ディスク情報 オフセット ****/
#define	DISK_FILENAME	(0)		/* char x [17]	*/
#define	DISK_PROTECT	(26)		/* char		*/
#define	DISK_TYPE	(27)		/* char		*/
#define	DISK_SIZE	(28)		/* long		*/
#define	DISK_TRACK	(32)		/* long x [164]	*/

	/**** ID情報 オフセット ****/
#define	DISK_C		(0)		/* char		*/
#define	DISK_H		(1)		/* char		*/
#define	DISK_R		(2)		/* char		*/
#define	DISK_N		(3)		/* char		*/
#define	DISK_SEC_NR	(4)		/* short	*/
#define	DISK_DENSITY	(6)		/* char		*/
#define	DISK_DELETED	(7)		/* char		*/
#define	DISK_STATUS	(8)		/* char		*/
#define	DISK_RESERVED	(9)		/* char	x [5]	*/
#define	DISK_SEC_SZ	(14)		/* char	x [???]	*/

#define	SZ_DISK_ID	(16)		/* ID情報 16Byte*/


	/**** ディスク/ID情報 定数 ****/
#define DISK_PROTECT_TRUE	(0x10)
#define	DISK_PROTECT_FALSE	(0x00)

#define	DISK_TYPE_2D		(0x00)
#define	DISK_TYPE_2DD		(0x10)
#define	DISK_TYPE_2HD		(0x20)

#define	DISK_DENSITY_DOUBLE	(0x00)
#define	DISK_DENSITY_SINGLE	(0x40)

#define	DISK_DELETED_TRUE	(0x10)
#define	DISK_DELETED_FALSE	(0x00)



	/**** 関数 ****/


void	drive_init( void );
void	drive_reset( void );
int	disk_insert( int drv, const char *filename, int img, int readonly );
int	disk_change_image( int drv, int img );
void	disk_eject( int drv );
int	disk_insert_A_to_B( int src, int dst, int img );

void	drive_set_empty( int drv );
void	drive_unset_empty( int drv );
void	drive_change_empty( int drv );
int	drive_check_empty( int drv );

int	get_drive_ready( int drv );




#define	disk_same_file()		(drive[ 0 ].fp == drive[ 1 ].fp)
#define	disk_image_exist( drv )		(drive[ drv ].fp)
#define	disk_image_num( drv )		(drive[ drv ].image_nr)
#define	disk_image_selected( drv )	(drive[ drv ].selected_image)


#endif	/* DRIVE_H_INCLUDED */
