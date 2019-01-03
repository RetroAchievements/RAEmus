#ifndef STATUS_H_INCLUDED
#define STATUS_H_INCLUDED


extern	int	status_imagename;		/* イメージ名表示有無 */


typedef struct {
    byte	*pixmap;	/* ステータスのイメージ用バッファ */
    int		w;		/* イメージ表示サイズ 幅 0〜	  */
    int		h;		/*                    高          */
} T_STATUS_INFO;

extern	T_STATUS_INFO	status_info[3];


#define	STATUS_INFO_TIME	(55 * 3)	/* 標準の表示時間 約3秒 */
#define	STATUS_WARN_TIME	(55 * 10)	/* 警告の表示時間 約10秒 */


void	status_init(void);
void	status_setup(int show);
void	status_update(void);

void	status_message_default(int pos, const char *msg);
void	status_message(int pos, int frames, const char *msg);


#endif	/* STATUS_H_INCLUDED */
