#include <IPAddress.h>

namespace config {
    // DHCPでアドレスを設定する場合: true
    // 静的IPアドレスを設定する場合: false
    constexpr bool use_dhcp = true;

    // 静的IPアドレス
    namespace static_ip {
        const IPAddress ip_addr(192, 168, 1, 2);
        const IPAddress dns_server(192, 168, 1, 1);
        const IPAddress gateway(192, 168, 1, 1);
        const IPAddress subnet_mask(255, 255, 255, 0);
    }

    // 対戦用待ち受けポート
    constexpr uint16_t listen_port = 10023;

    // 対戦接続時デフォルトポート
    constexpr uint16_t default_port = 10023;

    // ログ出力を有効にする
    constexpr bool enable_log = true;

    // ログ出力待ち受けポート (デバッグ用)
    constexpr uint16_t log_listen_port = 23;

    // MACアドレス
    uint8_t mac_addr[6] = {0x02, 0x20, 0x40, 0x00, 0x00, 0x00};

    // Board Unique ID を元に MAC アドレスの下位 3-octets を自動生成する
    constexpr bool use_board_unique_id = true;

    // 非アクティブ時の通信間隔
    constexpr int report_interval_ms = 40;
}
