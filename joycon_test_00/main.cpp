#define JOYCON_L_PRODUCT_ID 8198
#define JOYCON_R_PRODUCT_ID 8199
#include "hidapi/hidapi.h"

#include <stdio.h>

void main(void)
{
	// 接続されているHIDデバイスの連結リストを取得。
	hid_device_info *device = hid_enumerate(0, 0);

	while (device)
	{
		// プロダクトID等を指定して、HID deviceをopenする。そうすると、そのhidデバイスの情報が載ったhid_deviceが帰ってくる。
		hid_device *dev = hid_open(device->vendor_id, device->product_id, device->serial_number);
		// 今開いているデバイスのプロダクト名の取得。
		printf("\nproduct_id: %ls", device->product_string);
		// 次のデバイスへ。　　
		device = device->next;
	}

	int a;
	scanf("%d", &a);

	hid_free_enumeration(device);
}