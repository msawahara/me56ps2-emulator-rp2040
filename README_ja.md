# me56ps2-emulator-rp2040
このソフトウェアは、[me56ps2-emulator](https://github.com/msawahara/me56ps2-emulator)をWIZnet W5500-EVB-PICOで利用できるように移植したものです。

## セットアップ手順
[SETUP_ja.md](SETUP_ja.md)を参照してください。

## 設定
[CONFIGURATION_ja.md](CONFIGURATION_ja.md)を参照してください。

## 接続先の指定
モデムの接続時に、電話番号の代わりにIPアドレスを指定することができます。<br>
IPアドレスの区切り文字には ハイフン( `-` ) を使います。<br>
ポート番号を指定する場合は、IPアドレスの後ろに シャープ( `#` ) とポート番号を続けます。

例: 接続先IP = 198.51.100.234, ポート番号 = 8080の場合
```
198-51-100-234#8080
```
