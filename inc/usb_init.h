#ifndef __USB_INIT_H__
#define __USB_INIT_H__

#include "CH57x_common.h"
#include "CH57x_usbdev.h"

#define DevEP0SIZE 0x40 // 0x40
#define HID_GET_REPORT 0x01
#define HID_GET_IDLE 0x02
#define HID_GET_PROTOCOL 0x03
#define HID_SET_REPORT 0x09
#define HID_SET_IDLE 0x0A
#define HID_SET_PROTOCOL 0x0B

void ENDP0_SETUP();
void Get_Descriptors();
void Clear_Feature();
void Get_Device_Descriptor();
void Get_Config_Descriptor();
void Get_Strings();
void Get_Device_Qualifier_Descriptor();
void ENDP0_OUT();
void ENDP0_IN();
void ENDP1_OUT();
void ENDP1_IN();

typedef struct _USB_STRING_DESCRIPTOR
{
    UINT8 bLength;         //描述符大小
    UINT8 bDescriptorType; //字符串描述符类型，固定为0x03
    UINT16 bString[];      //字符串的首地址
} USB_STRING_DESCR, *PUSB_STRING_DESCR;

typedef struct __PACKED _USB_LANGUAGE_DESCRIPTOR
{
    UINT8 bLength;         //长度0x04
    UINT8 bDescriptorType; //语言描述符，固定0x03
    UINT16 LanguageID;     //语言ID
} USB_LANGUAGE_DESCR, *PUSB_LANGUAGE_DESCR;

USB_DEV_DESCR Rudder_Device_Descriptor = {
    0x12,
    0x01,
    0x0110,
    0x00, //填写00h，由后面的InterfaceDescriptor决定
    0x00,
    0x00,
    0x40,   // 64K
    0x0001, //厂商编号
    0x0001, //产品编号
    0x0001, //设备出厂编号
    0x01,   //描述厂商字符串的索引
    0x02,   //描述产品字符串的索引
    0x00,   //描述设备序列号字符串的索引 0x00表示无
    0x01,   //可能的配置数量
};

UINT8 Rudder_Config_Descriptor_All[] = {
    // Config Descriptor
    0x09,
    0x02,
    0x29, //配置所返回的所有数量的大小(1接口，2端点，1HID)
    0x00,
    0x01, //此配置所支持的接口数量
    0x01, // Set_Configuration命令需要的参数值
    0x00, //描述该配置的字符串的索引值，无描述符
    0x80, //供电模式，总线供电
    0x32, //最大电流，以2mA为单位，这里选择100mA

    // Interface Descriptor
    0x09,
    0x04,
    0x00, //接口编号（Interface 0）
    0x00, //备用的接口编号，无
    0x01, //该接口所用的端口数量
    0x03, //接口类型
    0x00, //接口子类型
    0x00, //接口遵循的协议
    0x00, //描述该接口的字符串索引

    // HID Descriptor
    0x09,
    0x21,
    0x11,
    0x01,
    0x00,
    0x01,
    0x22,
    0x17,
    0x00,

    // Endpoint 1 IN
    0x07,
    0x05,
    0x81, // USB设备的端点地址．Bit7，方向，对于控制端点可以忽略，1/0:IN/OUT．Bit6-4，保留．BIt3-0：端点号．
    0x03, // 端点属性．Bit7-2，保留（同步有定义）．BIt1-0：00控制，01同步，02批量，03中断．
    0x40,
    0x00, //本端点接收或发送的最大信息包大小 64K
    0x32, //主机查询端点的时间间隔

    // Endpoint 1 OUT
    0x07,
    0x05,
    0x01, // USB设备的端点地址．Bit7，方向，对于控制端点可以忽略，1/0:IN/OUT．Bit6-4，保留．BIt3-0：端点号．
    0x03, // 端点属性．Bit7-2，保留（同步有定义）．BIt1-0：00控制，01同步，02批量，03中断．
    0x40,
    0x00, //本端点接收或发送的最大信息包大小 64K
    0x32  //主机查询端点的时间间隔
};

UINT8 Rudder_Device_Qualifier_Descriptor[] = {
    0x0A,
    0x06,
    0x00,
    0x02,
    0x00,
    0x00,
    0x00,
    0x40,
    0x01,
    0x00,
};

// String Index: 0x00
UINT8 Rudder_Language_Descriptor[] = {
    0x04,
    0x03,
    0x09, // English
    0x04,
};

// String Index: 0x01
UINT8 Rudder_String_Manufacturer[] = {
    26, 0x03, 'F', 0, 'e', 0, 'l', 0, 'i', 0, 'x', 0, 'I', 0, 'n', 0, 'd', 0, 's', 0, 't', 0, 'r', 0, 'y', 0};

// String Index: 0x02
UINT8 Rudder_String_Product[] = {
    14, 0x03, 'R', 0, 'u', 0, 'd', 0, 'd', 0, 'e', 0, 'r', 0};

UINT8 Rudder_Report_Descriptor[] = {
    0x05,
    0x01,
    0x09,
    0x04,
    0xA1,
    0x01,
    0x09,
    0x33,
    0x09,
    0x34,
    0x16,
    0x4C,
    0xFF,
    0x26,
    0xB4,
    0x00,
    0x75,
    0x10,
    0x95,
    0x02,
    0x81,
    0x02,
    0xC0,
};

#endif