#include "usb_init.h"
#include "CH57x_usbdev.h"

UINT8 *pDescr;      //待发送的Descriptor的首地址
UINT8 len;          // 所准备的长度
UINT8 DevConfig;    //当前的配置ID
UINT8 SetupReqCode; //记录SETUP令牌中DATA0的请求码
UINT8 SetupReqType;
UINT16 SetupReqLen; //记录SETUP令牌中DATA0的请求长度
UINT8 errflag;      //错误码
UINT8 flag;

void USB_DevTransProcess(void)
{
    UINT8 len;
    UINT8 intflag, errflag;

    intflag = R8_USB_INT_FG;
    if (intflag & RB_UIF_TRANSFER)
    {
        switch (R8_USB_INT_ST & (MASK_UIS_TOKEN | MASK_UIS_ENDP))
        {
            // Endpoint 0：SETUP Transaction
        case UIS_TOKEN_SETUP:
            ENDP0_SETUP();
            break;
            // Endpoint 0：OUT Transaction
        case UIS_TOKEN_OUT:
            ENDP0_OUT();
            break;
            // Endpoint 0：IN Transaction
        case UIS_TOKEN_IN:
            ENDP0_IN();
            break;
            // Endpoint 1：OUT Transaction
        case UIS_TOKEN_OUT | 1:
            ENDP1_OUT();
            break;
            // Endpoint 1：IN Transaction
        case UIS_TOKEN_IN | 1:
            ENDP1_IN();
            break;
        default:
            UART1_SendByte(0xEE);
            break;
        }
        // Clear Interrupt Flag
        R8_USB_INT_FG = RB_UIF_TRANSFER;
    }
    else if (intflag & RB_UIF_BUS_RST)
    {
        R8_USB_DEV_AD = 0;
        R8_UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
        //写1清零标志
        R8_USB_INT_FG = RB_UIF_BUS_RST;
    }
    else if (intflag & RB_UIF_SUSPEND)
    {
        if (R8_USB_MIS_ST & RB_UMS_SUSPEND)
        {
        } // Suspend
        else
        {
        } // Awake
        R8_USB_INT_FG = RB_UIF_SUSPEND;
    }
    else
    {
        R8_USB_INT_FG = intflag;
    }
}

void ENDP0_SETUP()
{
    R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_NAK;
    if (R8_USB_RX_LEN == sizeof(USB_SETUP_REQ))
    {
        SetupReqLen = pSetupReqPak->wLength;
        SetupReqCode = pSetupReqPak->bRequest;
        SetupReqType = pSetupReqPak->bRequestType;
        if ((pSetupReqPak->bRequestType & USB_REQ_TYP_MASK) == USB_REQ_TYP_STANDARD)
        {
            //标准请求
            switch (pSetupReqPak->bRequest)
            {
            case USB_GET_DESCRIPTOR:
                Get_Descriptors();
                R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_ACK;
                break;
            case USB_SET_ADDRESS:
                SetupReqLen = pSetupReqPak->wValue & 0x7F;
                R8_UEP0_T_LEN = 0;
                R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_ACK;
                break;
            case USB_GET_CONFIGURATION:
                pEP0_DataBuf[0] = DevConfig;
                R8_UEP0_T_LEN = 1;
                R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_ACK;
                break;
            case USB_SET_CONFIGURATION:
                DevConfig = pSetupReqPak->wValue & 0xFF; //因为一个字节，所以保留低位
                R8_UEP0_T_LEN = 0;
                R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_ACK;
                break;
            case USB_CLEAR_FEATURE:
                break;
            case USB_GET_INTERFACE:
                pEP0_DataBuf[0] = 0x00; // 编号为0的Interface
                if (SetupReqLen > 1)
                    SetupReqLen = 1;
                break;
            case USB_GET_STATUS:
                if (pSetupReqPak->bRequestType == 0x82)
                {
                    pEP0_DataBuf[0] = 0x00;
                    pEP0_DataBuf[1] = 0x00;
                    SetupReqLen = 2;
                }
                break;
            default:
                errflag = 0xF3;
                break;
            }
        }
        else
        {
            //非标准请求
            switch (SetupReqCode)
            {
            case HID_SET_IDLE:
                R8_UEP0_T_LEN = 0;
                R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_STALL;
                break;
            default:
                errflag = 0xF6;
                break;
            }
        }
    }
    else
    {
        errflag = 0xF1;
    }
}

void Get_Descriptors()
{
    switch (pSetupReqPak->wValue >> 8)
    {
    case USB_DESCR_TYP_DEVICE:
        pDescr = (UINT8 *)&Rudder_Device_Descriptor;
        len = Rudder_Device_Descriptor.bLength;
        if (SetupReqLen > len)
            SetupReqLen = len;
        len = (SetupReqLen >= DevEP0SIZE) ? DevEP0SIZE : SetupReqLen;
        break;
    case USB_DESCR_TYP_CONFIG:
        pDescr = Rudder_Config_Descriptor_All;
        len = Rudder_Config_Descriptor_All[2];
        if (SetupReqLen > len)
            SetupReqLen = len;
        len = (SetupReqLen >= DevEP0SIZE) ? DevEP0SIZE : SetupReqLen;
        break;
    case USB_DESCR_TYP_STRING:
        switch ((pSetupReqPak->wValue) & 0xff)
        {
        case 0:
            pDescr = Rudder_Language_Descriptor;
            len = Rudder_Language_Descriptor[0];
            break;
        case 1:
            pDescr = Rudder_String_Manufacturer;
            len = Rudder_String_Manufacturer[0];
            break;
        case 2:
            pDescr = Rudder_String_Product;
            len = Rudder_String_Product[0];
            break;
        default:
            errflag = 0xF5;
            break;
        }
        if (SetupReqLen > len)
            SetupReqLen = len;
        len = (SetupReqLen >= DevEP0SIZE) ? DevEP0SIZE : SetupReqLen;
        break;
    case USB_DESCR_TYP_REPORT:
        pDescr = Rudder_Report_Descriptor;
        len = sizeof(Rudder_Report_Descriptor);
        if (SetupReqLen > len)
            SetupReqLen = len;
        len = (SetupReqLen >= DevEP0SIZE) ? DevEP0SIZE : SetupReqLen;
        break;
    case USB_DESCR_TYP_PHYSIC:
        // No Physics Descriptor
        break;
    default:
        errflag = 0xF4;
        break;
    }
    // Fill Data
    if (len > 0)
    {
        memcpy(pEP0_RAM_Addr, pDescr, len);
        SetupReqLen -= len;
        pDescr += len;
    }
    R8_UEP0_T_LEN = len;
}

void ENDP0_OUT()
{
}
void ENDP0_IN()
{
    UINT8 len = 0;
    switch (SetupReqCode)
    {
    case USB_GET_DESCRIPTOR:
        len = SetupReqLen >= DevEP0SIZE ? DevEP0SIZE : SetupReqLen;
        if (len > 0)
        {
            memcpy(pEP0_DataBuf, pDescr, len);
            SetupReqLen -= len;
            pDescr += len;
        }
        R8_UEP0_T_LEN = len;
        R8_UEP0_CTRL = RB_UEP_R_TOG | (R8_UEP0_CTRL & 0x40) ^ 0x40 | UEP_R_RES_ACK | UEP_T_RES_ACK;
        // R8_UEP0_CTRL ^= RB_UEP_T_TOG;
        break;
    case USB_SET_ADDRESS:
        R8_USB_DEV_AD = (R8_USB_DEV_AD & RB_UDA_GP_BIT) | SetupReqLen;
        break;
    case USB_SET_CONFIGURATION:
        break;
    case HID_SET_IDLE:
        break;
    default:
        errflag = 0xF6;
        R8_UEP0_T_LEN = 0; // 状态阶段完成中断或者是强制上传0长度数据包结束控制传输
        R8_UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
        break;
    }
}
void ENDP1_OUT()
{
    
}
void ENDP1_IN()
{
}