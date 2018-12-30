#ifndef SNAPSHOT_H_INCLUDED
#define SNAPSHOT_H_INCLUDED

#include <stdio.h>
#include "file-op.h"


#if 0	/* → file-op.h */
extern char file_snap[QUASI88_MAX_FILENAME];	/* スナップショットベース部 */
#endif



enum {
  SNAPSHOT_FMT_BMP,
  SNAPSHOT_FMT_PPM,
  SNAPSHOT_FMT_RAW
};
extern	int	snapshot_format;	/* スナップショットフォーマット	*/


					/* スナップショットコマンド	*/
#define	SNAPSHOT_CMD_SIZE	(1024)
extern	char	snapshot_cmd[ SNAPSHOT_CMD_SIZE ];
extern	char	snapshot_cmd_do;	/* コマンド実行の有無		*/

extern	char	snapshot_cmd_enable;	/* コマンド実行の可否		*/


void		filename_init_snap(int synchronize);
void		filename_set_snap_base(const char *filename);
const char	*filename_get_snap_base(void);

void	screen_snapshot_init(void);
void	screen_snapshot_exit(void);

int	screen_snapshot_save(void);




void		filename_init_wav(int synchronize);
void		filename_set_wav_base(const char *filename);
const char	*filename_get_wav_base(void);

int	waveout_save_start(void);
void	waveout_save_stop(void);

#endif	/* SNAPSHOT_H_INCLUDED */
