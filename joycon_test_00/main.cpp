#define JOYCON_L_PRODUCT_ID 8198
#define JOYCON_R_PRODUCT_ID 8199
#include "hidapi\hidapi.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>

// 説明
// windowsでは“Wireless Gamepad”と認識されます。
// ランプはビットフラグで、4桁。ランプの一番上から10進数で 1, 2, 4, 8 と対応しています。

namespace
{
    //===============================
    // ボタン
    //===============================
    enum JOYCON_BUTTON
    {
        Y = 0x01,   //1
        X = 0x02,   //2
        B = 0x04,   //4
        A = 0x08,   //8
        SR = 0x10,  //16
        SL = 0x20,  //32
        //R = 0x40,   //64
        //ZR = 0x80,  //128
    };

    //===============================
    // 特殊ボタン (左右合一ボタン)
    //===============================
    enum JOYCON_BUTTON_SP
    {
        MINUS = 0x01,   //1
        PLUS = 0x02,   //2
        STICK_L = 0x04,   //4
        STICK_R = 0x08,   //8
        HOME = 0x10,  //16
        PHOTO = 0x20,  //32
        R = 0x40,   //64
        ZR = 0x80,  //128
    };

    //===============================
    //スティック
    //===============================
    enum JOYCON_STICK
    {

    };
}

//====================================
// Joyconに出力
//====================================
void SendSubcommand(hid_device* dev, uint8_t command, uint8_t data[], int len, int* globalCount)
{
    uint8_t buf[0x40]; memset(buf, 0x0, size_t(0x40));

    buf[0] = 1; // 0x10 for rumble only
    buf[1] = *globalCount; // Increment by 1 for each packet sent. It loops in 0x0 - 0xF range.

    if (*globalCount == 0xf0) {
        *globalCount = 0x00;
    }
    else {
        *globalCount++;
    }

    buf[10] = command;
    memcpy(buf + 11, data, len);

    hid_write(dev, buf, 0x40);
}

//====================================
// メイン関数
//====================================
int main()
{
    int globalCount = 0;
    // 接続されているHIDデバイスの連結リストを取得。
    hid_device_info* device = hid_enumerate(0, 0);

    while (device)
    {
        if (device->product_id == JOYCON_L_PRODUCT_ID || device->product_id == JOYCON_R_PRODUCT_ID)
        {
            // プロダクトID等を指定して、HID deviceをopenする。そうすると、そのhidデバイスの情報が載ったhid_deviceが帰ってくる。
            hid_device* dev = hid_open(device->vendor_id, device->product_id, device->serial_number);
            // 今開いているデバイスのプロダクト名の取得。
            printf("\nproduct_id: %ls", device->product_string);

            uint8_t data[0x01];

            data[0] = 0x01;
            // 0x03番のサブコマンドに、0x01を送信します。
            SendSubcommand(dev, 0x30, data, 1, &globalCount);

            // read input report
            uint8_t buff[0x40]; memset(buff, 0x40, size_t(0x40));
            // 読み込むサイズを指定。
            size_t size = 49;
            // buff に input report が入る。
            int ret = hid_read(dev, buff, size);
            printf("\ninput report id: %d\n", *buff);
            // ボタンの押し込みがビットフラグで表現されている。
            printf("input report id: %d\n", buff[5]);

            //対象デバイスの状態を取得し続ける
            while (true)
            {
                // input report を受けとる。
                int ret = hid_read(dev, buff, size);
                // input report の id が 0x3F のものに絞る。
                if (*buff != 0x3F)
                {
                    continue;
                }

                // input report の id　を表示。
                printf("\ninput report id: %d\n", *buff);
                // ボタンのビットフィールドを表示。
                printf("button byte 1: %d\n", buff[1]);
                printf("button byte 2: %d\n", buff[2]);
                // スティックの状態を表示。
                printf("stick  byte 3: %d\n", buff[3]);

                data[0] = buff[3];

                SendSubcommand(dev, 0x30, data, 1, &globalCount);
            }
        }
        // 次のデバイスへ。　　
        device = device->next;
    }
    hid_free_enumeration(device);
}