# セットアップ手順

## Arduino IDEのインストールと設定
- 最新のArduino IDEをダウンロードし、インストールする
  - https://www.arduino.cc/en/software
- Arduino IDEを起動する
- メニューから「File」→「Preferences」を開く
- 「Additional boards manager URLs」に以下のURLを追加する
  - https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json
- OKを押し、Preferencesを閉じる
- メニューから「Tools」→「Boards」→「Boards Manager」を開く
- Boards Managerの検索欄に「RP2040」と入力する
- 「Raspberry Pi Pico/RP2040 by Earle F. Philhower, III」を選び、Installボタンを押す
- インストールが完了したら、Arduino IDEを終了する

## ソースコードの取得
- ソースコードが保存さているレポジトリを開く
  - https://github.com/msawahara/me56ps2-emulator-rp2040
- 緑色の「Code」ボタンを押して、「Download ZIP」を選び、ソースコードをダウンロードする
- ダウンロードした「me56ps2-emulator-rp2040-master.zip」を展開する
- フォルダ名から最後の「-master」を消し、「me56ps2-emulator-rp2040」へ名前を変更する

## 書き込み
- ダウンロードしたフォルダ内のme56ps2-emulator-rp2040.inoを開く
- W5500-EVB-Picoを、BOOTSELボタンを押しながらPCへ接続する
  - 接続をしたらBOOTSELボタンは離す
- 「Select Board」→「Select other board and port」を開く
- BOARDSの選択
  - BOARDSの検索欄に「W5500」と入力する
  - 「WIZnet W5500-EVB-Pico」を選ぶ
- PORTSの選択
  - 「Show all ports」にチェックを入れる
  - 「UF2 Board UF2 Devices」を選ぶ
- OKを押す
- メニューから「Tools」→「USB Stack」→「No USB」を選ぶ (表示が無い場合はこの手順を飛ばしてください)
- メニューから「Sketch」→「Upload」を選び、書き込みを開始する
- 「Done Uploading.」「Wrote xxxxxx bytes to X:/NEW.UF2」などと表示されたら、書き込み完了
