#include "hidapi\hidapi.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>

// ����
// windows�ł́gWireless Gamepad�h�ƔF������܂��B
// �����v�̓r�b�g�t���O�ŁA4���B�����v�̈�ԏォ��10�i���� 1, 2, 4, 8 �ƑΉ����Ă��܂��B
// �Q�lURL: https://github.com/dekuNukem/Nintendo_Switch_Reverse_Engineering

#define JOYCON_L_PRODUCT_ID 0x2006 //8198
#define JOYCON_R_PRODUCT_ID 0x2007 //8199
#define VENDOR_ID  0x057E  // Nintendo Co., Ltd //0d1406

namespace
{
    //===============================
    // �T�u�R�}���h
    //===============================
    enum SUBCOMMAND
    {
        SUBCMD_LIGHT_SET = 0x30,        //�v���C���[���C�g�̐ݒ�
        SUBCMD_LIGHT_GET = 0x31,        //�v���C���[���C�g�̎擾
        SUBCMD_LIGHT_HOME = 0x38,       //HOME���C�g�̐ݒ�
        SUBCMD_IMU = 0x40,              //6���Z���T�[�̗L����
        SUBCMD_IMU_SENSI_SET = 0x41,    //6���Z���T�[�̊��x�ݒ�
        SUBCMD_IMU_REGI_WRITE = 0x42,   //6���Z���T�[�̃��W�X�^�[����
        SUBCMD_IMU_REGI_READ = 0x43,    //6���Z���T�[�̃��W�X�^�[�Ǎ�
        SUBCMD_VIB = 0x48,              //�U���̗L����
        SUBCMD_MAX
    };

    //===============================
    // �{�^��(���E���ꂼ��)(buff[1])
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
    // ����{�^�� (���E����{�^��)(buff[2])
    //===============================
    enum JOYCON_BUTTON_SP
    {
        MINUS = 0x01,   //1
        PLUS = 0x02,   //2
        STICK_L = 0x04,   //4
        STICK_R = 0x08,   //8
        HOME = 0x10,  //16
        PHOTO = 0x20,  //32
        LR = 0x40,   //64
        ZLZR = 0x80,  //128
    };

    //===============================
    //�X�e�B�b�N
    //===============================
    enum JOYCON_STICK
    {

    };
}

//====================================
// Joycon�ɏo��
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
// ���C���֐�
//====================================
int main()
{
    int globalCount = 0;
    // �ڑ�����Ă���HID�f�o�C�X�̘A�����X�g���擾�B
    hid_device_info* device = hid_enumerate(0, 0);

    while (device)
    {
        if (device->product_id == JOYCON_L_PRODUCT_ID || device->product_id == JOYCON_R_PRODUCT_ID)
        {
            // �v���_�N�gID�����w�肵�āAHID device��open����B��������ƁA����hid�f�o�C�X�̏�񂪍ڂ���hid_device���A���Ă���B
            hid_device* dev = hid_open(device->vendor_id, device->product_id, device->serial_number);
            // ���J���Ă���f�o�C�X�̃v���_�N�g���̎擾�B
            printf("\nproduct_id: %ls", device->product_string);

            uint8_t data[0x01];

            // 0x03�Ԃ̃T�u�R�}���h�ɁA0x01�𑗐M���܂��B
            data[0] = 0x01; //Report ID
            SendSubcommand(dev, 0x30, data, 1, &globalCount);
            data[0] = 0x40; //command
            SendSubcommand(dev, 0x40, data, 1, &globalCount);
            //hid_set_nonblocking(dev, 1);

            // read input report
            uint8_t buff[0x40]; memset(buff, 0x40, size_t(0x40));
            // �ǂݍ��ރT�C�Y���w��B
            size_t size = sizeof(buff) / sizeof(int);
            // buff �� input report ������B
            int ret = hid_read(dev, buff, size);
            printf("\ninput report id: %d\n", *buff);
            // �{�^���̉������݂��r�b�g�t���O�ŕ\������Ă���B
            printf("input report id: %d\n", buff[5]);

            //���W�X�^�[���
            uint8_t regiData[0x02];

            //�Ώۃf�o�C�X�̏�Ԃ��擾��������
            while (true)
            {
                //�O���r�Ώ�
                //hid_device* devNow = hid_open(device->vendor_id, device->product_id, device->serial_number);

                // input report ���󂯂Ƃ�B
                int ret = hid_read(dev, buff, size);
                
                memset(regiData, 0, size_t(0x02));
                SendSubcommand(dev, SUBCMD_IMU_REGI_READ, regiData, 2, &globalCount);

                // input report �� id �� 0x3F �̂��̂ɍi��B
                if (*buff != 0x3F)
                {
                    printf("\ninput report id: %d\n", *buff);
                    continue;
                }

                // input report �� id�@��\���B
                printf("\ninput report id: %d\n", *buff);
                // �{�^���̃r�b�g�t�B�[���h��\���B
                printf("button byte 1: %d\n", buff[1]);
                printf("button byte 2: %d\n", buff[2]);
                // �X�e�B�b�N�̏�Ԃ�\���B
                printf("stick  byte 3: %d\n", buff[3]);

                //printf("byte 4: %d\n", buff[4]);
                //printf("byte 5: %d\n", buff[5]);
                //printf("byte 6: %d\n", buff[6]);
                //printf("byte 7: %d\n", buff[7]);
                //printf("byte 8: %d\n", buff[8]);
                //printf("byte 9: %d\n", buff[9]);
                //printf("byte 10: %d\n", buff[10]);
                //printf("byte 11: %d\n", buff[11]);
                //printf("byte 12: %d\n", buff[12]);
                //printf("byte 13: %d\n", buff[13]);
                //printf("byte 14: %d\n", buff[14]);
                //printf("byte 15: %d\n", buff[15]);
                //printf("byte 16: %d\n", buff[16]);

                printf("register 0: %d\n", regiData[0]);
                printf("register 1: %d\n", regiData[1]);
                
                //// �W���C���f�[�^�̎擾
                //int16_t gyro_x = (buff[19] << 8) | buff[18];
                //int16_t gyro_y = (buff[21] << 8) | buff[20];
                //int16_t gyro_z = (buff[23] << 8) | buff[22];

                //// �W���C���f�[�^���o��
                //printf("Gyro X:%d, Gyro Y: %d, Gyro Z: %d",gyro_x, gyro_y, gyro_z);


                data[0] = buff[3];

                SendSubcommand(dev, 0x30, data, 1, &globalCount);
            }
        }
        // ���̃f�o�C�X�ցB�@�@
        device = device->next;
    }
    hid_free_enumeration(device);
}