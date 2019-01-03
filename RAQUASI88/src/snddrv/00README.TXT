
※ このディレクトリ以下には、MAME 0.112/XMAME 0.106 のソースが含まれています。
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

  QUASI88 のコンパイル時にサウンド出力を有効にしておくと、このディレクトリ
以下のファイルが組み込まれ、MAME/XMAME のサウンド出力機能 (サウンドドライバ)
が使用することができるようになります。

なお、この場合コンパイルしたバイナリは、MAME および XMAME のライセンスが適用
されますのでご注意ください。このディレクトリ以下のソースファイルの再配布等に
ついても同様です。


MAME のライセンスについては、このディレクトリにある

	license.txt

を参照してください。(XMAMEのライセンスもこれに準拠します)


(*) MAME は、Nicola Salmoria 氏 と The MAME Team が著作権を有する、マルチ
    アーケードマシンエミュレータです。
    XMAME は、XMAME project による MAME の UNIX移植版です。(現在、休止中)

    MAME についての、より多くの情報は以下のウェブサイトを御覧下さい。

	http://www.mamedev.org/


-------------------------------------------------------------------------------

●src ディレクトリ および src/sound ディレクトリのソースは、
  MAME 0.112 のソースを QUASI88 向けに改変したものです。


src/cpuintrf.h			ダミー
src/driver.c			未使用部分をコメントアウト。
src/driver.h			ダミー
src/mame.h			ダミー
src/mamecore.h			ダミー
src/osdepend.h			ダミー
src/profiler.h			ダミー
src/restrack.c			未使用部分をコメントアウト。
src/restrack.h			ダミー
src/sndintrf.c			BEEP, fmgen対応部を追加。
src/sndintrf.h			BEEP, fmgen対応部を追加。
src/sound.c			プロトタイプ変更など
src/sound.h			プロトタイプ変更
src/streams.c			MAME 0.106 相当に変更
src/streams.h			
src/timer.h			ダミー

src/sound/2203intf.c		タイマー、音量処理追加
src/sound/2203intf.h		タイマー、音量処理追加
src/sound/2608intf.c		タイマー、音量処理追加
src/sound/2608intf.h		タイマー、音量処理追加
src/sound/ay8910.c		音量処理追加
src/sound/ay8910.h		音量処理追加
src/sound/flt_rc.c		
src/sound/flt_rc.h		
src/sound/flt_vol.c		
src/sound/flt_vol.h		
src/sound/fm.c			修正
src/sound/fm.h			define修正
src/sound/samples.c		ファイル処理変更
src/sound/samples.h		
src/sound/wavwrite.c		修正
src/sound/wavwrite.h		
src/sound/ymdeltat.c		音量処理追加
src/sound/ymdeltat.h		


●src/unix ディレクトリ以下のソースは、
  XMAME 0.106 のソースを QUASI88向けに改変したものです。


src/unix/sound.c				オプション修正
src/unix/sysdep/begin_code.h			
src/unix/sysdep/end_code.h			
src/unix/sysdep/fifo.h				
src/unix/sysdep/misc.c				
src/unix/sysdep/misc.h				
src/unix/sysdep/plugin_manager.c		
src/unix/sysdep/plugin_manager.h		
src/unix/sysdep/plugin_manager_priv.h		
src/unix/sysdep/rc.c				独自オプション処理追加
src/unix/sysdep/rc.h				独自オプション処理追加
src/unix/sysdep/rc_priv.h			
src/unix/sysdep/sysdep_dsp.c			VERBOSE対応
src/unix/sysdep/sysdep_dsp.h			
src/unix/sysdep/sysdep_dsp_plugins.h		
src/unix/sysdep/sysdep_dsp_priv.h		
src/unix/sysdep/sysdep_mixer.c			VERBOSE対応
src/unix/sysdep/sysdep_mixer.h			
src/unix/sysdep/sysdep_mixer_plugins.h		
src/unix/sysdep/sysdep_mixer_priv.h		
src/unix/sysdep/sysdep_sound_stream.c		独自オプション処理追加
src/unix/sysdep/sysdep_sound_stream.h		
src/unix/sysdep/sysdep_sound_stream_priv.h	
src/unix/sysdep/dsp-drivers/alsa.c		
src/unix/sysdep/dsp-drivers/arts.c		
src/unix/sysdep/dsp-drivers/artssound.c		
src/unix/sysdep/dsp-drivers/coreaudio.c		
src/unix/sysdep/dsp-drivers/esound.c		VERBOSE対応
src/unix/sysdep/dsp-drivers/io-audio.c		
src/unix/sysdep/dsp-drivers/irix.c		
src/unix/sysdep/dsp-drivers/irix_al.c		
src/unix/sysdep/dsp-drivers/netbsd.c		
src/unix/sysdep/dsp-drivers/oss.c		VERBOSE対応
src/unix/sysdep/dsp-drivers/sdl.c		VERBOSE対応
src/unix/sysdep/dsp-drivers/solaris.c		
src/unix/sysdep/dsp-drivers/soundkit.m		
src/unix/sysdep/dsp-drivers/waveout.c		VERBOSE対応
src/unix/sysdep/mixer-drivers/alsa.c		
src/unix/sysdep/mixer-drivers/oss.c		
src/unix/sysdep/mixer-drivers/solaris.c		


-------------------------------------------------------------------------------
●MAME で必要なワークの宣言・定義用に、以下のファイルを作成しました。

	snddrv/quasi88/mame-quasi88.c
	snddrv/quasi88/mame-quasi88.h
	snddrv/quasi88/machine.c


●X11版 QUASI88 と MAME/XMAME とのインターフェイス部として、以下のファイルを
  作成しました。

	snddrv/quasi88/X11/audio.c
	snddrv/quasi88/X11/audio.h


●SDL版 QUASI88 と MAME とのインターフェイス部として、以下のファイルを
  作成しました。
  (snddrv/quasi88/SDL/sdl.c は、 src/unix/sysdep/dsp-drivers/sdl.c を
   ベースに作成しました)

	snddrv/quasi88/SDL/audio.c
	snddrv/quasi88/SDL/audio.h
	snddrv/quasi88/SDL/sdl.c


●MAME のサウンドドライバで BEEP 音を出力させるために、以下のファイルを
  新規に追加しました。

	snddrv/quasi88/beepintf.c
	snddrv/quasi88/beepintf.h
	snddrv/quasi88/beep.c
	snddrv/quasi88/beep.h


●MAME のサウンドドライバで fmgen のサウンドを出力させるために、以下のファイル
  を新規に追加しました。

	snddrv/quasi88/2203fmgen.cpp
	snddrv/quasi88/2203fmgen.h
	snddrv/quasi88/2608fmgen.cpp
	snddrv/quasi88/2608fmgen.h


-------------------------------------------------------------------------------
