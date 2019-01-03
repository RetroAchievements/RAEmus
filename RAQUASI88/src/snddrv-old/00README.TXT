
※ このディレクトリ以下には、MAME 0.71/XMAME 0.71.1 のソースが含まれています。
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

  QUASI88 のコンパイル時にサウンド出力を有効にしておくと、このディレクトリ
以下のファイルが組み込まれ、MAME/XMAME のサウンド出力機能 (サウンドドライバ)
が使用することができるようになります。

なお、この場合コンパイルしたバイナリは、MAME および XMAME のライセンスが適用
されますのでご注意ください。このディレクトリ以下のソースファイルの再配布等に
ついても同様です。


MAME のライセンスについては、このディレクトリにある

	mame.txt

を参照してください。(XMAMEのライセンスもこれに準拠します)


(*) MAME は、Nicola Salmoria 氏 と The MAME Team が著作権を有する、マルチ
    アーケードマシンエミュレータです。
    XMAME は、XMAME project による MAME の UNIX移植版です。

    MAME、XMAME についての、より多くの情報は以下のページを御覧下さい。

	MAME  website ==>> http://www.mamedev.org/
	XMAME website ==>> http://x.mame.net/      (リンク切れ)


-------------------------------------------------------------------------------
xmame 0.71.1 のサウンドドライバを組み込みにあたり、以下のファイルを
使用しました。これらのうちいくつかは、QUASI88 向けに改変されています。

なお、左がもともとのファイルのあった場所で、右が QUASI88 配布物における
ファイルの場所です。

xmame-0.71.1/src/sndintrf.c				snddrv/src/sndintrf.c
xmame-0.71.1/src/sndintrf.h				snddrv/src/sndintrf.h
xmame-0.71.1/src/sound/2203intf.c			snddrv/src/sound/2203intf.c
xmame-0.71.1/src/sound/2203intf.h			snddrv/src/sound/2203intf.h
xmame-0.71.1/src/sound/2608intf.c			snddrv/src/sound/2608intf.c
xmame-0.71.1/src/sound/2608intf.h			snddrv/src/sound/2608intf.h
xmame-0.71.1/src/sound/ay8910.c				snddrv/src/sound/ay8910.c
xmame-0.71.1/src/sound/ay8910.h				snddrv/src/sound/ay8910.h
xmame-0.71.1/src/sound/filter.c				snddrv/src/sound/filter.c
xmame-0.71.1/src/sound/filter.h				snddrv/src/sound/filter.h
xmame-0.71.1/src/sound/fm.c				snddrv/src/sound/fm.c
xmame-0.71.1/src/sound/fm.h				snddrv/src/sound/fm.h
xmame-0.71.1/src/sound/mixer.c				snddrv/src/sound/mixer.c
xmame-0.71.1/src/sound/mixer.h				snddrv/src/sound/mixer.h
xmame-0.71.1/src/sound/streams.c			snddrv/src/sound/streams.c
xmame-0.71.1/src/sound/streams.h			snddrv/src/sound/streams.h
xmame-0.71.1/src/sound/ymdeltat.c			snddrv/src/sound/ymdeltat.c
xmame-0.71.1/src/sound/ymdeltat.h			snddrv/src/sound/ymdeltat.h
xmame-0.71.1/src/unix/sound.c				snddrv/src/unix/sound.c
xmame-0.71.1/src/unix/sysdep/COPYING.LIB		snddrv/src/unix/sysdep/COPYING.LIB
xmame-0.71.1/src/unix/sysdep/begin_code.h		snddrv/src/unix/sysdep/begin_code.h
xmame-0.71.1/src/unix/sysdep/end_code.h			snddrv/src/unix/sysdep/end_code.h
xmame-0.71.1/src/unix/sysdep/fifo.h			snddrv/src/unix/sysdep/fifo.h
xmame-0.71.1/src/unix/sysdep/misc.c			snddrv/src/unix/sysdep/misc.c
xmame-0.71.1/src/unix/sysdep/misc.h			snddrv/src/unix/sysdep/misc.h
xmame-0.71.1/src/unix/sysdep/plugin_manager.c		snddrv/src/unix/sysdep/plugin_manager.c
xmame-0.71.1/src/unix/sysdep/plugin_manager.h		snddrv/src/unix/sysdep/plugin_manager.h
xmame-0.71.1/src/unix/sysdep/plugin_manager_priv.h	snddrv/src/unix/sysdep/plugin_manager_priv.h
xmame-0.71.1/src/unix/sysdep/rc.c			snddrv/src/unix/sysdep/rc.c
xmame-0.71.1/src/unix/sysdep/rc.h			snddrv/src/unix/sysdep/rc.h
xmame-0.71.1/src/unix/sysdep/rc_priv.h			snddrv/src/unix/sysdep/rc_priv.h
xmame-0.71.1/src/unix/sysdep/sound_stream.c		snddrv/src/unix/sysdep/sound_stream.c
xmame-0.71.1/src/unix/sysdep/sound_stream.h		snddrv/src/unix/sysdep/sound_stream.h
xmame-0.71.1/src/unix/sysdep/sound_stream_priv.h	snddrv/src/unix/sysdep/sound_stream_priv.h
xmame-0.71.1/src/unix/sysdep/sysdep_dsp.c		snddrv/src/unix/sysdep/sysdep_dsp.c
xmame-0.71.1/src/unix/sysdep/sysdep_dsp.h		snddrv/src/unix/sysdep/sysdep_dsp.h
xmame-0.71.1/src/unix/sysdep/sysdep_dsp_plugins.h	snddrv/src/unix/sysdep/sysdep_dsp_plugins.h
xmame-0.71.1/src/unix/sysdep/sysdep_dsp_priv.h		snddrv/src/unix/sysdep/sysdep_dsp_priv.h
xmame-0.71.1/src/unix/sysdep/sysdep_mixer.c		snddrv/src/unix/sysdep/sysdep_mixer.c
xmame-0.71.1/src/unix/sysdep/sysdep_mixer.h		snddrv/src/unix/sysdep/sysdep_mixer.h
xmame-0.71.1/src/unix/sysdep/sysdep_mixer_plugins.h	snddrv/src/unix/sysdep/sysdep_mixer_plugins.h
xmame-0.71.1/src/unix/sysdep/sysdep_mixer_priv.h	snddrv/src/unix/sysdep/sysdep_mixer_priv.h
xmame-0.71.1/src/unix/sysdep/dsp-drivers/alsa.c		snddrv/src/unix/sysdep/dsp-drivers/alsa.c
xmame-0.71.1/src/unix/sysdep/dsp-drivers/arts.c		snddrv/src/unix/sysdep/dsp-drivers/arts.c
xmame-0.71.1/src/unix/sysdep/dsp-drivers/artssound.c	snddrv/src/unix/sysdep/dsp-drivers/artssound.c
xmame-0.71.1/src/unix/sysdep/dsp-drivers/coreaudio.c	snddrv/src/unix/sysdep/dsp-drivers/coreaudio.c
xmame-0.71.1/src/unix/sysdep/dsp-drivers/esound.c	snddrv/src/unix/sysdep/dsp-drivers/esound.c
xmame-0.71.1/src/unix/sysdep/dsp-drivers/io-audio.c	snddrv/src/unix/sysdep/dsp-drivers/io-audio.c
xmame-0.71.1/src/unix/sysdep/dsp-drivers/irix.c		snddrv/src/unix/sysdep/dsp-drivers/irix.c
xmame-0.71.1/src/unix/sysdep/dsp-drivers/irix_al.c	snddrv/src/unix/sysdep/dsp-drivers/irix_al.c
xmame-0.71.1/src/unix/sysdep/dsp-drivers/netbsd.c	snddrv/src/unix/sysdep/dsp-drivers/netbsd.c
xmame-0.71.1/src/unix/sysdep/dsp-drivers/oss.c		snddrv/src/unix/sysdep/dsp-drivers/oss.c
xmame-0.71.1/src/unix/sysdep/dsp-drivers/sdl.c		snddrv/src/unix/sysdep/dsp-drivers/sdl.c, snddrv/quasi88-SDL/sdl.c
xmame-0.71.1/src/unix/sysdep/dsp-drivers/solaris.c	snddrv/src/unix/sysdep/dsp-drivers/solaris.c
xmame-0.71.1/src/unix/sysdep/dsp-drivers/soundkit.m	snddrv/src/unix/sysdep/dsp-drivers/soundkit.m
xmame-0.71.1/src/unix/sysdep/dsp-drivers/waveout.c	snddrv/src/unix/sysdep/dsp-drivers/waveout.c
xmame-0.71.1/src/unix/sysdep/mixer-drivers/oss.c	snddrv/src/unix/sysdep/mixer-drivers/oss.c
xmame-0.71.1/src/unix/sysdep/mixer-drivers/solaris.c	snddrv/src/unix/sysdep/mixer-drivers/solaris.c



MAME で必要なワークの宣言・定義用に、以下のファイルを作成しました。

	snddrv/quasi88/mame-quasi88.c
	snddrv/quasi88/mame-quasi88.h


コンパイル用のダミーファイルとして、以下のファイルを作成ししました。

	snddrv/src/devices.h
	snddrv/src/osd_cpu.h
	snddrv/src/state.h
	snddrv/src/unix/driver.h
	snddrv/src/unix/xmame.h


-------------------------------------------------------------------------------
X11版 QUASI88 と MAME/XMAME とのインターフェイス部として、以下のファイルを
作成しました。

	snddrv/quasi88/X11/audio.c
	snddrv/quasi88/X11/audio.h


SDL版 QUASI88 と MAME とのインターフェイス部として、以下のファイルを
作成しました。
(snddrv/quasi88/SDL/sdl.c は、 src/unix/sysdep/dsp-drivers/sdl.c を
 ベースに作成しました)

	snddrv/quasi88/SDL/audio.c
	snddrv/quasi88/SDL/audio.h
	snddrv/quasi88/SDL/sdl.c


-------------------------------------------------------------------------------
MAME/XMAME のサウンドドライバで BEEP 音を出力させるために、以下のファイルを
新規に追加しました。

	snddrv/quasi88/beepintf.c
	snddrv/quasi88/beep.c
	snddrv/quasi88/beep.h
