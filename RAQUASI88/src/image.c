/************************************************************************/
/*									*/
/* ディスクイメージのヘッダ部の処理 (読み書き)				*/
/*									*/
/************************************************************************/

#include <stdio.h>
#include <string.h>

#include "quasi88.h"
#include "initval.h"
#include "drive.h"
#include "image.h"
#include "file-op.h"





/***********************************************************************
 * 指定されたオフセット位置から、ヘッダ情報 32バイトを読み出す。
 *	ヘッダ情報をもとに、このイメージの末尾１バイトをアクセスし、
 *	アクセス失敗したらイメージが壊れていると判断する。
 *	ファイルの現在位置は関数呼び出し前から変更されているので注意すること
 *
 *	引数	FILE *fp	ファイルポインタ
 *		long offset	イメージの先頭のファイル位置
 *		Uchar header[32] ヘッダ情報の格納先
 *
 *	返り値	D88_SUCCESS	正常終了
 *		D88_NO_IMAGE	これ以上イメージが無い
 *		D88_BAD_IMAGE	イメージ異常(ヘッダ破損、サイズ不足)
 *		D88_ERR_SEEK	シークエラー
 *		D88_ERR_READ	リードエラー
 *	header[32] には、イメージのヘッダの内容が入る。
 *	但し、返り値が D88_SUCCESS 以外の時は、header[]の内容は不定。
 ************************************************************************/

int	d88_read_header( OSD_FILE *fp, long offset, unsigned char header[32] )
{
  long	size;
  char	c;
  long	current;
  int	result = D88_SUCCESS;


  if( (current = osd_ftell( fp )) < 0 ) result = D88_ERR_SEEK;

  if( result == D88_SUCCESS ){

    if( osd_fseek( fp, offset,  SEEK_SET ) ) result = D88_NO_IMAGE;

    if( result == D88_SUCCESS ){
      size = osd_fread( header, sizeof(char), 32, fp );
      if     ( size == 0 ) result = D88_NO_IMAGE;
      else if( size < 32 ) result = D88_BAD_IMAGE;

      if( result == D88_SUCCESS ){

	size = READ_SIZE_IN_HEADER( header );

	if( osd_fseek( fp, offset+size-1, SEEK_SET ) ) result = D88_BAD_IMAGE;
	if( result == D88_SUCCESS ){
	  if( osd_fread( &c, sizeof(char), 1, fp )!=1 ) result = D88_BAD_IMAGE;
	}

      }
    }

    if( (osd_fseek( fp, current, SEEK_SET )) ) result = D88_ERR_SEEK;
  }

  return result;
}



/***********************************************************************
 * 指定されたイメージの最後に、ブランクディスクのイメージを追加する。
 *	追加されるのは、2D のディスク。2DD/2HD は未対応
 *
 *	引数	FILE *fp	ファイルポインタ			
 *		int  drv	ドライブにセットされているファイルの場合、
 *				そのドライブ番号。( 0 or 1 )
 *				別のファイルの場合は -1
 ************************************************************************/

int	d88_append_blank( OSD_FILE *fp, int drv )
{
  char *s, c[256];
  int	i, j, result = D88_SUCCESS;
  int	type = DISK_TYPE_2D;
  int	trk_nr, trk_size, img_size;
  long	current;

  switch( type ){
  case DISK_TYPE_2HD:
    trk_nr   = 164;
    trk_size = 0x2300;
    break;
  case DISK_TYPE_2DD:
    trk_nr   = 164;
    trk_size = 0x1600;
    break;
  case DISK_TYPE_2D:
  default:
    type     = DISK_TYPE_2D;
    trk_nr   = 84;
    trk_size = 0x1600;
    break;
  }
  img_size = (32) + (164*4) + (trk_nr*trk_size);


	/* 現在のファイル位置を覚えておく。(あとで、戻すため) */

  if( (current = osd_ftell( fp )) < 0 ) { result = D88_ERR_SEEK; }

	/* ファイル位置を、ファイルの末端に移動 */

  if( result == D88_SUCCESS ){
    if( (osd_fseek( fp, 0,  SEEK_END )) ){ result = D88_ERR_SEEK; }
  }

	/* ヘッダ部書き込み */

  if( result == D88_SUCCESS ){
    s = &c[0];
    for(i=0;i<17;i++) *s++ = 0;		/* filename */
    for(i=0;i<9;i++)  *s++ = 0;		/* reserved */
    *s++ = DISK_PROTECT_FALSE;		/* protect  */
    *s++ = type;			/* type     */
    *s++ = (img_size >>  0) & 0xff;	/* size     */
    *s++ = (img_size >>  8) & 0xff;
    *s++ = (img_size >> 16) & 0xff;
    *s++ = (img_size >> 24) & 0xff;
    if( osd_fwrite( c, sizeof(char), 32, fp )!=32 ){
      result = D88_ERR_WRITE;
    }
  }

	/* トラック部書き込み */

  if( result == D88_SUCCESS ){
    j = 0x2b0;				/* size of track data */
    for(i=0;i<164;i++){
      if( i<trk_nr ){
	c[0] = (j >>  0)&0xff;
	c[1] = (j >>  8)&0xff;
	c[2] = (j >> 16)&0xff;
	c[3] = (j >> 24)&0xff;
	j += trk_size;
      }else{
	c[0] = c[1] = c[2] = c[3] = 0;
      }
      if( osd_fwrite( c, sizeof(char), 4, fp )!=4 ) break;
    }
    if( i!=164 ){
      result = D88_ERR_WRITE;
    }
  }

	/* セクタ部書き込み */

  if( result == D88_SUCCESS ){
    for( i=0; i<256; i++ ) c[i] = 0;	/* sec data */
    for( i=0; i<trk_nr*trk_size; ){
      if( trk_nr*trk_size - i < 256 ) j = trk_nr*trk_size - i;
      else                            j = 256;
      if( osd_fwrite( c, sizeof(char), j, fp )!=(size_t)j ) break;
      i += j;
    }
    if( i!=trk_nr*trk_size ){
      result = D88_ERR_WRITE;
    }
  }

  osd_fflush( fp );


	/* ファイル位置を最初の位置に戻す */

  if( current >= 0 ){
    if( (osd_fseek( fp, current, SEEK_SET )) ){ result = D88_ERR_SEEK; }
  }

	/* 正常終了 かつ ドライブにセットされたファイル更新時はワークも更新 */

  if( result == D88_SUCCESS &&
      drv >= 0 ){

    int img, img_size;
    int type = DISK_TYPE_2D;

    switch( type ){
    case DISK_TYPE_2HD:
      img_size = 164 * 0x2300;
      break;
    case DISK_TYPE_2DD:
      img_size = 164 * 0x1600;
      break;
    case DISK_TYPE_2D:
    default:
      type     = DISK_TYPE_2D;
      img_size = 84 * 0x1600;
      break;
    }
    img_size += (32) + (164*4);


    for( i=0; i < 2; i++, drv ^= 1 ){
      if( !drive[ drv ].detect_broken_image ){
	if( drive[ drv ].image_nr + 1 <= MAX_NR_IMAGE ){
	  img = drive[ drv ].image_nr;
	  drive[drv].image[img].name[0] = '\0';
	  drive[drv].image[img].protect = DISK_PROTECT_FALSE;
	  drive[drv].image[img].type    = type;
	  drive[drv].image[img].size    = img_size;
	  drive[ drv ].image_nr ++;
	}else{
	  drive[ drv ].over_image = TRUE;
	}
      }
      if( drive[0].fp != drive[1].fp ) break;
    }

  }

  return result;
}



/*----------------------------------------------------------------------
 * ディスクの先頭から指定されたイメージを探し出し、そのオフセット位置を返す
 *	ファイルの現在位置は関数呼び出し前から変更されているので注意すること
 *
 *	引数	FILE *fp	ファイルポインタ
 *		ing  img	探すイメージの番号 0〜
 *		long *img_top	見つかった場合、そのファイル位置をセット
 *
 *	返り値	D88_SUCCESS	正常終了
 *----------------------------------------------------------------------*/

static	int	d88_search_image( OSD_FILE *fp, int img, long *image_top )
{
  unsigned char c[32];
  long offset = 0;
  int	num = 0, result;

  if( img >= MAX_NR_IMAGE ) return D88_MANY_IMAGE;

  while( (result = d88_read_header( fp, offset, c ) ) == D88_SUCCESS ){

    if( img == num ){
      *image_top = offset;
      return result;
    }

    num ++;
    offset += READ_SIZE_IN_HEADER( c );

    if( offset < 0 ){
      return D88_MANY_IMAGE;
    }

  }

  return result;
}


/***********************************************************************
 * 指定されたイメージ番号の、プロテクト情報を書き換える。
 *
 *	引数	FILE *fp	ファイルポインタ			
 *		int  drv	ドライブにセットされているファイルの場合、
 *				そのドライブ番号。( 0 or 1 )
 *				別のファイルの場合は -1
 *		int  img	イメージ番号
 *		char *protect   プロテクト情報
 ************************************************************************/
int	d88_write_protect( OSD_FILE *fp, int drv, int img, const char *protect)
{
  long	current;
  long	offset = 0;
  int	i, result;


	/* 現在のファイル位置を覚えておく。(あとで、戻すため) */

  if( (current = osd_ftell( fp )) < 0 ) { return D88_ERR_SEEK; }

	/* img 番目のイメージを検索  */

  result = d88_search_image( fp, img, &offset );

	/* img 番目のイメージが見つかったらプロテクト情報を書き換えて終了 */

  if( result == D88_SUCCESS ){

    if( osd_fseek( fp, offset + DISK_PROTECT, SEEK_SET )==0 ){
      if( osd_fwrite( protect, sizeof(char), 1, fp )==1 ){
	   osd_fflush( fp );
	   result = D88_SUCCESS;
      }
      else result = D88_ERR_WRITE;
    }else  result = D88_ERR_SEEK;

  }

	/* ファイル位置をもとの位置に戻す */

  if( (osd_fseek( fp, current, SEEK_SET )) ){ return D88_ERR_SEEK; }


	/* 正常終了 かつ ドライブにセットされたファイル更新時はワークも更新 */

  if( result == D88_SUCCESS &&
      drv >= 0 ){

    for( i=0; i < 2; i++, drv ^= 1 ){

      drive[ drv ].image[ img ].protect = *protect;

      if( drive[ drv ].selected_image == img ){
	drive[ drv ].protect = drive[ drv ].image[ img ].protect;

	if( drive[ drv ].read_only ) drive[ drv ].protect = DISK_PROTECT_TRUE;
      }

      if( drive[0].fp != drive[1].fp ) break;
    }

  }

  return result;
}



/***********************************************************************
 * 指定されたイメージ番号の、イメージ名を書き換える。
 *
 *	引数	FILE *fp	ファイルポインタ			
 *		int  drv	ドライブにセットされているファイルの場合、
 *				そのドライブ番号。( 0 or 1 )
 *				別のファイルの場合は -1
 *		int  img	イメージ番号
 *		char *name      イメージ名(先頭の16文字まで)
 ************************************************************************/
int	d88_write_name( OSD_FILE *fp, int drv, int img, const char *name )
{
  long	current;
  long	offset = 0;
  int	i, result;
  char	c[17];

  strncpy( c, name, 16 );
  c[16] = '\0';


	/* 現在のファイル位置を覚えておく。(あとで、戻すため) */

  if( (current = osd_ftell( fp )) < 0 ) { return D88_ERR_SEEK; }

	/* img 番目のイメージを検索  */

  result = d88_search_image( fp, img, &offset );

	/* img 番目のイメージが見つかったらイメージ名を書き換えて終了 */

  if( result == D88_SUCCESS ){

    if( osd_fseek( fp, offset + DISK_FILENAME, SEEK_SET )==0 ){
      if( osd_fwrite( c, sizeof(char), 17, fp )==17 ){
	   osd_fflush( fp );
	   result = D88_SUCCESS;
      }
      else result = D88_ERR_WRITE;
    }else  result = D88_ERR_SEEK;

  }

	/* ファイル位置をもとの位置に戻す */

  if( (osd_fseek( fp, current, SEEK_SET )) ){ return D88_ERR_SEEK; }


	/* 正常終了 かつ ドライブにセットされたファイル更新時はワークも更新 */

  if( result == D88_SUCCESS &&
      drv >= 0 ){

    for( i=0; i < 2; i++, drv ^= 1 ){
      strncpy( drive[ drv ].image[ img ].name, name, 17 );

      if( drive[0].fp != drive[1].fp ) break;
    }

  }

  return result;
}



/***********************************************************************
 * 指定されたイメージ番号の、イメージをアンフォーマット状態にする。
 *
 *	引数	FILE *fp	ファイルポインタ			
 *		int  drv	ドライブにセットされているファイルの場合、
 *				そのドライブ番号。( 0 or 1 )
 *				別のファイルの場合は -1
 *		int  img	イメージ番号
 ************************************************************************/
int	d88_write_unformat( OSD_FILE *fp, int drv, int img )
{
  unsigned char c[256];
  long	current;
  long	offset = 0;
  int	result;
  long	st, sz, len;


	/* 現在のファイル位置を覚えておく。(あとで、戻すため) */

  if( (current = osd_ftell( fp )) < 0 ) { return D88_ERR_SEEK; }

	/* img 番目のイメージを検索  */

  result = d88_search_image( fp, img, &offset );

	/* img 番目のイメージが見つかったらアンフォーマットして終了 */

  if( result == D88_SUCCESS ){

    if( osd_fseek( fp, offset, SEEK_SET )==0 ){		/* ヘッダから       */
      if( osd_fread( c, sizeof(char), 32, fp )==32 ){	/* 全体のサイズ取得 */
	sz = READ_SIZE_IN_HEADER( c );			/* (はみ出し判定用) */

	if( osd_fseek( fp, offset + DISK_TRACK, SEEK_SET )==0 ){
	  if( osd_fread( c, sizeof(char), 4, fp )==4 ){

	    st =((long)c[0]+((long)c[1]<<8)+((long)c[2]<<16)+((long)c[3]<<24));

	    memset( c, 0, 256 );		  /* 0トラック目の先頭から   */
	    if( DISK_TRACK+4 <= st && st < sz ){  /* イメージ末尾まで0ライト */

	      if( osd_fseek( fp, offset+st, SEEK_SET )==0 ){

		while( st < sz ){
		  if( sz - st < 256 ) len = sz - st;
		  else                len = 256;
		  if( osd_fwrite( c, sizeof(char), len, fp ) != (size_t)len ){
		    result = D88_ERR_WRITE;
		    break;
		  }
		  st += len;
		}
		osd_fflush( fp );

  	      } else result = D88_ERR_SEEK;

	    } else   result = D88_BAD_IMAGE;

	  } else result = D88_ERR_READ;
	} else   result = D88_ERR_SEEK;

      } else   result = D88_ERR_READ;
    } else     result = D88_ERR_SEEK;

  }

	/* ファイル位置をもとの位置に戻す */

  if( (osd_fseek( fp, current, SEEK_SET )) ){ return D88_ERR_SEEK; }


  return result;
}





/***********************************************************************
 * 指定されたイメージ番号の、イメージをフォーマットする。
 *	2D の、N88-BASIC データディスクとしてフォーマットを行なう。
 *
 *	引数	FILE *fp	ファイルポインタ			
 *		int  drv	ドライブにセットされているファイルの場合、
 *				そのドライブ番号。( 0 or 1 )
 *				別のファイルの場合は -1
 *		int  img	イメージ番号
 ************************************************************************/


#define	GENERATE_SECTOR_DATA()					\
	do{							\
	  if( trk==37 && sec==13 ){				\
	    i = 1;						\
	  }else if( trk==37 && (sec==14||sec==15||sec==16) ){	\
	    i = 2;						\
	  }else{						\
	    i = 0;						\
	  }							\
	  d[i][ DISK_C ] = trk >> 1;				\
	  d[i][ DISK_H ] = trk & 1;				\
	  d[i][ DISK_R ] = sec;					\
	  d[i][ DISK_N ] = 1;					\
	  d[i][ DISK_SEC_NR +0 ] = 16 >> 0;			\
	  d[i][ DISK_SEC_NR +1 ] = 16 >> 8;			\
	  d[i][ DISK_DENSITY ] = DISK_DENSITY_DOUBLE;		\
	  d[i][ DISK_DELETED ] = DISK_DELETED_FALSE;		\
	  d[i][ DISK_STATUS ] = 0;				\
	  d[i][ DISK_SEC_SZ +0 ] = ((256) >> 0) & 0xff;		\
	  d[i][ DISK_SEC_SZ +1 ] = ((256) >> 8) & 0xff;		\
	}while(0)


int	d88_write_format( OSD_FILE *fp, int drv, int img )
{
  unsigned char c[32];
  unsigned char d[3][ 16 + 256 ];
  long	current;
  long	offset, st, sz;
  int	i, result, trk, sec;

	/* セクタ情報を生成 */

  for( i=0; i<16;  i++ ) d[ 0 ][ i ] = 0x00;	/* 下記以外の全セクタ */
  for( i=0; i<256; i++ ) d[ 0 ][ 16 +i ] = 0xff;

  for( i=0; i<16;  i++ ) d[ 1 ][ i ] = 0x00;	/* 37トラック セクタ13 */
  for( i=0; i<256; i++ ) d[ 1 ][ 16 +i ] = 0x00;

  for( i=0; i<16;  i++ ) d[ 2 ][ i ] = 0x00;	/* 37トラック セクタ14〜16 */
  for( i=0; i<160; i++ ) d[ 2 ][ 16 +i ] = 0xff;
  for(    ; i<256; i++ ) d[ 2 ][ 16 +i ] = 0x00;
  d[ 2 ][ 16 +37*2    ] = 0xfe;
  d[ 2 ][ 16 +37*2 +1 ] = 0xfe;


	/* 現在のファイル位置を覚えておく。(あとで、戻すため) */

  if( (current = osd_ftell( fp )) < 0 ) { return D88_ERR_SEEK; }

	/* img 番目のイメージを検索  */

  result = d88_search_image( fp, img, &offset );

	/* img 番目のイメージが見つかったらフォーマットして終了 */

  if( result == D88_SUCCESS ){

    if( osd_fseek( fp, offset, SEEK_SET )==0 ){		/* ヘッダから       */
      if( osd_fread( c, sizeof(char), 32, fp )==32 ){	/* 全体のサイズ取得 */
	sz = READ_SIZE_IN_HEADER( c );			/* (はみ出し判定用) */

	c[ 0 ] = DISK_TYPE_2D;				/* 2Dイメージに設定 */
	if( osd_fseek( fp, offset + DISK_TYPE, SEEK_SET )==0 ){
	  if( osd_fwrite( c, sizeof(char), 1, fp )==1 ){

	    for( trk=0; trk<80; trk++ ){		/* 80トラック書換え */

	      if( osd_fseek( fp, offset+(DISK_TRACK+trk*4), SEEK_SET )==0 ){
		if( osd_fread( c, sizeof(char), 4, fp )==4 ){

		  st =  (long)c[0]     +((long)c[1]<<8)
		       +((long)c[2]<<16)+((long)c[3]<<24);

		  if( osd_fseek( fp, offset+st, SEEK_SET )==0 ){

		    if( DISK_TRACK+80*4 <= st && st+(16+256)*16 <= sz ){

		      for( sec=1; sec<=16; sec++ ){	/* 16セクタ書換え */
			GENERATE_SECTOR_DATA();
			if( osd_fwrite( d[i], sizeof(char), 16+256, fp )
			    				 != 16+256 ){
			  result = D88_ERR_WRITE;
			  break;
			}
		      }

		    }

		  }else result = D88_BAD_IMAGE;

		}else result = D88_ERR_READ;
	      }else   result = D88_ERR_SEEK;

	      if( result != D88_SUCCESS ){
		break;
	      }
	    }
	    osd_fflush( fp );

	  }else result = D88_ERR_WRITE;
	}else   result = D88_ERR_SEEK;

      }else result = D88_ERR_READ;
    }else   result = D88_ERR_SEEK;

  }

	/* ファイル位置をもとの位置に戻す */

  if( (osd_fseek( fp, current, SEEK_SET )) ){ return D88_ERR_SEEK; }


	/* 正常終了 かつ ドライブにセットされたファイル更新時はワークも更新 */

  if( result == D88_SUCCESS &&
      drv >= 0 ){

    for( i=0; i<2; i++, drv ^= 1 ){
      drive[ drv ].image[ img ].type = DISK_TYPE_2D;

      if( drive[0].fp != drive[1].fp ) break;
    }

  }

  return result;
}
