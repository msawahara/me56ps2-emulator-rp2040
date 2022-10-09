# 設定
多くの場合は、DHCPによる自動設定で動作するため、設定を変更しなくとも利用できます。<br>
しかし、サーバとして利用するため静的IPアドレスを使用する場合や、ポート番号の変更を行う場合は、自分で設定を行なうこともできます。<br>

設定は `config.h` ファイルに記述します。

## DHCP
```c++
    // DHCPでアドレスを設定する場合: true
    // 静的IPアドレスを設定する場合: false
    constexpr bool use_dhcp = true;
```
DHCPを有効にする場合、 `use_dhcp` に `true` を指定します。<br>
DHCPを無効化し静的IPアドレスを使用する場合には、 `use_dhcp` に `false` を指定します。

## 静的IPアドレス
```c++
    // 静的IPアドレス
    namespace static_ip {
        const IPAddress ip_addr(192, 168, 1, 2);
        const IPAddress dns_server(192, 168, 1, 1);
        const IPAddress gateway(192, 168, 1, 1);
        const IPAddress subnet_mask(255, 255, 255, 0);
    }
```
静的IPアドレスを指定する場合は、以下の4項目を設定する必要があります。
- `ip_addr` : デバイスが使用するIPアドレス
- `dns_server` : DNSサーバのIPアドレス
- `gateway` : ゲートウェイのIPアドレス
- `subnet_mask` : サブネットマスク (IPアドレス形式)

IPアドレスは、IPv4アドレスのみ利用できます。<br>
`config.h` 中に記載する際は、丸括弧の中にカンマ区切りで記載してください。

## 対戦に使用するポート番号
```c++
    // 対戦用待ち受けポート
    constexpr uint16_t listen_port = 10023;

    // 対戦接続時デフォルトポート
    constexpr uint16_t default_port = 10023;
```
`listen_port` は、待ち受けに使用するポート番号です。サーバとして使用する場合には、必要に応じて変更してください。<br>
`default_port` は、接続先指定時にポート番号を省略した場合に使用する接続先ポート番号です。

## デバッグ用ログ出力
```c++
    // ログ出力を有効にする
    constexpr bool enable_log = false;

    // ログ出力待ち受けポート (デバッグ用)
    constexpr uint16_t log_listen_port = 23;
```
デバッグ用ログ出力の設定は、初期状態では無効化されています。<br>
有効化すると、ログ出力時にUARTの送信待ちが生じ、ゲームの動作に影響が出る場合があります。

ログ出力を有効にするには、 `enable_log` を `true` に設定します。
すると、以下の二通りの方法でログが出力されます。
- UART (TX = 1 pin, RX = 2 pin, 115200bps, 8bits, no parity, 1 stop bit)
- TCP 23番ポート (`log_listen_port` で指定したポート番号)

## MACアドレス
```c++
    // MACアドレス
    uint8_t mac_addr[6] = {0x02, 0x20, 0x40, 0x00, 0x00, 0x00};

    // Board Unique ID を元に MAC アドレスの下位 3-octets を自動生成する
    constexpr bool use_board_unique_id = true;
```
MACアドレスは、 `02-20-40` から始まるロカールMACアドレスを使用しています。<br>
また、複数のデバイスを利用した際にMACアドレスが重複しないよう、Board Unique IDを元に下位 3-octets のアドレスを生成しています。

## 非アクティブ時の通信間隔
```c++
    // 非アクティブ時の通信間隔
    constexpr int report_interval_ms = 40;
```
USB通信で送信すべきデータが無い場合に、モデムのステータス情報を報告する間隔の設定です。<br>
通常は変更する必要はありません。

___注意: この設定は、latency timerとは異なります。___

オリジナルのME56PS2では、USBの送信バッファ中のデータが1パケット分 (64 bytesからヘッダ分の2 bytesを除いた62 bytes) に満たない場合に、latency timerがタイムアウトするまでパケットの送信を待機するように設計されていました。このlatency timerの初期値は40msであり、通信が遅延する原因となります。

me56ps2-emulator-rp2040の実装では、送信バッファ中のデータが1パケット分に満たない場合でも、バッファ中のデータを直ちに送信します。この動作は、USBバスの転送効率の低下を引き起こしますが、より低遅延な通信が実現できます。

つまり、me56ps2-emulator-rp2040にはlatency timerの設定は存在しません。<br>
`report_interval_ms` はlatency timerとは異なり、データ送信を遅延を生じるものではなく、送信すべきデータが無い場合の通信間隔の設定です。
