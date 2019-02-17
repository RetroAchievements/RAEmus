## QUASI88改 / QUASI88kai
---
QUASI88改は、福永省三氏の [QUASI88](http://www.eonet.ne.jp/~showtime/quasi88/) の改良版です。  
ソースやビルドシステムを必要以上に変えず、バグを修正し新機能を実装する方針です。

QUASI88kai is an improved version of Showzoh Fukunaga's [QUASI88](http://www.eonet.ne.jp/~showtime/quasi88/).
The aim is to fix bugs and implement new features with minimal changes to the source and build system.

主な更新内容 / Main changes:
* ビルドシステムの一般更新・修正（現在のコンパイラをサポートするように） / General updates to the build system (to support modern compilers)
* 全ファイルをUTF-8に変換 / All files converted to UTF-8
* Win32版に -double、-fullscreen を実装 / Support -double and -fullscreen in the Win32 version
* Win32版に起動オプションを有効にする / Support launch options in the Win32 version
* Win32版にテープのイメージファイルのドラッグ・アンド・ドロップを実装 / Support drag-and-drop of tape image files in the Win32 version
* フォーカスが外される時はキーの押下状態を解除 / Clear key press state when losing focus
* 偽メモリウェイト使用時の音出力を修正 / Fix audio output when using memory wait
* RetroAchievements対応版「RAQUASI88」を追加 / Add a RetroAchievements-compatible version "RAQUASI88"

この先の予定についてはイッシューページの enhancement タグを参照してください。

Please refer to issues with the "enhancement" tag for future plans.

最新バージョンは、 0.6.8　(2019/02/14 リリース) です。

The latest version is 0.6.8 (released 2019/02/14).

---

### 使い方 - Usage

QUASI88改を動かすにあたっては、以下のものが必要です。

The following items are required to run QUASI88kai.

- ROMイメージファイル / ROM image file(s)
- ディスクかテープのイメージファイル / Disk or tape images

詳しくは、 document/QUASI88.TXT と document/MANUAL.TXT を参照してください。

For more details, please refer to document/QUASI88.TXT and document/MANUAL.TXT (in Japanese).

---

### 著作権／免責 - Copyright/Disclaimer

QUASI88改（サウンド出力処理部を除く） はフリーソフトウェアであり、著作権は作者が保有しています。このソフトは無保証であり、このソフトを使用した事によるいかなる損害も作者は一切の責任を 負いません。ライセンスに関しては、QUASI88と同様に修正BSDライセンスに準じます。

QUASI88kai (excluding the sound processing portion) is free software, and copyright remains with the associated authors. This software is provided without any guarantee, and the authors do not take any responsibility for any damage incurred by its use. It is licensed under the Revised BSD license, as is QUASI88.

QUASI88改のサウンド出力処理部は、MAME および、XMAME のソースコードを使用しています。このソースは、各著作者が著作権を有します。ライセンスに関しては、 license/MAME.TXT (英語のみ)を参照下さい。

The sound processing portion of QUASI88kai uses source code from MAME and XMAME. The copyright to this source code belongs to its corresponding authors. Please refer to license/MAME.TXT for licensing information.

QUASI88改のサウンド出力処理部のうち、FM音源ジェネレータには fmgen のソースコードを使用しています。このソースは cisc氏が著作権を有します。ライセンスに関しては、 license/FMGEN.TXT を参照ください。

The sound processing portion of QUASI88kai also uses source code from the FM audio generator "fmgen". The copyright to this source code belongs to cisc. Please refer to license/FMGEN.TXT (in Japanese) for licensing information.

---
(c) 1998-2019 S.Fukunaga, R.Zumer
