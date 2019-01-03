### ***★QUASI88とは?***
---
&nbsp;&nbsp;QUASI88 は、UNIX + X Window System ないし UNIX + SDL の環境で動作する PC-8801 エミュレータです。<br />
現在 FreeBSD、Linux、IRIX、AIX、Solalis で動作報告があります。
MacOS X + X11  や MS-Windows + Cygwin + XFree86  といった環境でも動作するそうです。

その他にも、SDLライブラリがサポートしている環境、 すなわち Windows  や、 Macintosh  などでも動作すると思われます。
（SDLについての詳しくは、 http://www.libsdl.org/ を参照ください。）
<br />

- QUASI88 は、[fMSX](http://www.komkon.org/fms/)  のソースコードを参考にして、作成されました。
- QUASI88 のサウンドドライバは、[MAME](http://mamedev.org/)・ xmame から移植しました。
- QUASI88 のFM音源ジェネレータには、[fmgen](http://retropc.net/cisc/m88/) も組み込まれています。
<br />
<br />

▼ 最新バージョンは、 0.6.4  (2013/3/29 リリース) です。
<br />
<br />

### ***★使い方***
---
&nbsp;&nbsp;QUASI88を動かすにあたっては、以下のものが必要なです

- 処理速度の速いマシン        

    &nbsp;&nbsp;&nbsp;&nbsp;といっても、最近の Windows マシンレベルなら十分と思われます。<br />
    &nbsp;&nbsp;&nbsp;&nbsp;Pentium III 1GHz あたりもあれば、十分かと。

- [ROMイメージファイル](https://www.eonet.ne.jp/~showtime/quasi88/memo/rom.html)

- ディスクイメージファイル  

    &nbsp;&nbsp;&nbsp;&nbsp;現状、QUASI88 の配布物には、イメージ吸いだしソフトは同梱されていません。QUASI88 は [ぶるー氏作の P88SR.EXE](http://www1.plala.or.jp/aoto/pc88emu.htm) のイメージで作動可能ですので、自力でイメージを入手してください。<br />
    &nbsp;&nbsp;&nbsp;&nbsp;（幸い、インターネット上にはいろんなイメージ吸いだしソフトがあります。 これらツール類は、AKATTA氏の 88Break http://www.cug.net/~akatta/ が非常に充実しており、お勧めです。） 
<br />

詳しい使い方については、 [こちらのページ](https://www.eonet.ne.jp/~showtime/quasi88/howto/index.html) を参照してください。
<br />
<br />

---

#### 著作権／免責

<font size="2">
&nbsp;QUASI88 （サウンド出力処理部を除く） はフリーソフトウェアであり、著作権は作者が保有しています。このソフトは無保証であり、このソフトを使用した事によるいかなる損害も作者は一切の責任を 負いません。ライセンスに関しては、修正BSDライセンスに準ずるものとします。

&nbsp;QUASI88 のサウンド出力処理部は、MAME および、XMAME のソースコードを使用しています。このソースは、各著作者が著作権を有します。ライセンスに関しては、MAME および、XMAME のドキュメントを参照下さい。

&nbsp;QUASI88 のサウンド出力処理部のうち、FM音源ジェネレータには fmgen のソースコードを使用しています。このソースは cisc氏が著作権を有します。ライセンスに関しては、fmgen のドキュメントを参照ください。
</font>

---
(c) 1998-2018 S.Fukunaga, R.Zumer