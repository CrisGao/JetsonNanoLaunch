/*************************************************************************************************
Copyright   : 2019  INMOTION Tech. Co., Ltd.
File name   : im_packet_send_receive_method.h
Description : 本文件作为 im_protocol 的补充，对 im_protocol 中数据包的接收及发送进行了二次封装，
              旨在提供严格遵循协议中数据格式的规范化的数据收发接口
Author      : 龙乐坪(Lorin)
Version     : V1.1.4
Date        : 2019.04.17
History     : V1.0.0   2018.10.09  第一个版本
              V1.0.1   2018.10.10  扩展了 IM_DataPacketProcess 的功能，可获取已接收的数据包数量，
                                   以及已处理完成的数据包数量，并返回处理结果(错误值)
              V1.0.2   2018.10.12  增加了格式0 在格式1的基础上省略设备号
              V1.1.0   2018.10.15  简化了数据包发送接口形式，由按数据格式及是否使用标签，是否加密
                                   这些选项分别封装改为统一按数据格式的组别封装，提供标准组和专用
                                   组的两组接口
                                   数据接收部分增加了按组别分别接收的接口
                                   文件由 im_std_data_format 更名为 im_packet_send_receive_method
              V1.1.1   2018.10.16  修复了IM_StandardGroupGetReceivedPacket 及
                                   IM_SpecialGroupGetReceivedPacket 中一处数据类型错误可能导致函数
                                   返回值异常的Bug
              V1.1.2   2018.11.12  修复了 IM_StandardGroupPackDataToTransmitter 中发送格式 0,1 时
                                   通道及属性被去除的BUG
              V1.1.3   2019.04.12  因 im_protocol 升级，IM_PackDataToTransmitter 的返回类型及定义
                                   发生了更改，故同步更改了 IM_StandardGroupPackDataToTransmitter 
                                   及 IM_SpecialGroupPackDataToTransmitter 的返回值类型（uint16_t 
                                   改为 int32_t）及定义
              V1.1.4   2019.04.17  因 im_protocol 升级，同步的
                                   IM_StandardGroupGetReceivedPacket,
                                   IM_SpecialGroupGetReceivedPacket,
                                   IM_StandardGroupPacketReceivedCallback,
                                   IM_SpecialGroupPacketReceivedCallback,
                                   四个函数增加了对数据包接收完成时间的支持
*************************************************************************************************/

#include <string.h>
#include "im_protocol.h"

//要求返回数据包是否已被处理 处理完成返回 1 否则返回 0
uint8_t (*IM_StandardGroupPacketReceivedCallback)
(
    int8_t handle, uint32_t tab, 
    uint8_t source, uint8_t target, uint8_t channel, uint8_t property, 
    void *para, uint8_t length, uint32_t received_time
) = NULL;
uint8_t (*IM_SpecialGroupPacketReceivedCallback)
(
    int8_t handle, uint32_t tab, 
    uint8_t source, uint8_t target, uint8_t cmd, 
    void *para, uint8_t length, uint32_t received_time
) = NULL;

//返回0 无误 -1句柄所指向的收发器不存在 -2收发器忙 -3
int8_t IM_DataPacketProcess(int8_t handle, uint8_t *received_packet_num, uint8_t *processed_packet_num)
{
    int8_t res = 0;
    uint8_t received_packets = 0;
    uint8_t processed_packets = 0;
    uint32_t tab;
    uint8_t format;
    uint8_t length;
    uint8_t *data;
    uint32_t received_time;
    
    for(;;)
    {
        res = IM_GetReceivedPacket(handle, &tab, &format, &data, &length, &received_time, 0);
        if(res <= 0)
        {
            break;
        }
        received_packets++;
        
        switch(format)
        {
            case 0:
                if(IM_StandardGroupPacketReceivedCallback != NULL && length >= 2)
                {
                    if(IM_StandardGroupPacketReceivedCallback(handle, tab, 0x00, 0xFF, data[0], data[1], data + 2, length - 2, received_time))
                    {
                        processed_packets++;
                    }
                }
                break;
            case 1:
                if(IM_StandardGroupPacketReceivedCallback != NULL && length >= 3)
                {
                    if(IM_StandardGroupPacketReceivedCallback(handle, tab, 0x00, data[0], data[1], data[2], data + 3, length - 3, received_time))
                    {
                        processed_packets++;
                    }
                }
                break;
            case 2:
                if(IM_StandardGroupPacketReceivedCallback != NULL && length >= 4)
                {
                    if(IM_StandardGroupPacketReceivedCallback(handle, tab, data[0], data[1], data[2], data[3], data + 4, length - 4, received_time))
                    {
                        processed_packets++;
                    }
                }
                break;
            case 3:
                if(IM_SpecialGroupPacketReceivedCallback != NULL && length >= 1)
                {
                    if(IM_SpecialGroupPacketReceivedCallback(handle, tab, 0x00, 0xFF, data[0], data + 1, length - 1, received_time))
                    {
                        processed_packets++;
                    }
                }
                break;
            case 4:
                if(IM_SpecialGroupPacketReceivedCallback != NULL && length >= 2)
                {
                    if(IM_SpecialGroupPacketReceivedCallback(handle, tab, 0x00, data[0], data[1], data + 2, length - 2, received_time))
                    {
                        processed_packets++;
                    }
                }
                break;
            case 5:
                if(IM_SpecialGroupPacketReceivedCallback != NULL && length >= 3)
                {
                    if(IM_SpecialGroupPacketReceivedCallback(handle, tab, data[0], data[1], data[2], data + 3, length - 3, received_time))
                    {
                        processed_packets++;
                    }
                }
                break;
            default:
                break;
        }
        
        IM_FreeReceivedPacket(handle);
    }
    
    if(received_packet_num != NULL)
    {
        *received_packet_num = received_packets;
    }
    if(processed_packet_num != NULL)
    {
        *processed_packet_num = processed_packets;
    }
    return res;
}

int8_t IM_DataPacketProcess2(int8_t handle, uint8_t *received_packet_num, uint8_t *processed_packet_num)
{
    int8_t res = 0;
    uint8_t received_packets = 0;
    uint8_t processed_packets = 0;
    uint32_t tab;
    uint8_t format;
    uint8_t length;
    uint8_t *p_data;
    uint8_t data[256];
    uint32_t received_time;
    
    for(;;)
    {
        res = IM_GetReceivedPacket(handle, &tab, &format, &p_data, &length, &received_time, 0);
        if(res <= 0)
        {
            break;
        }
        memcpy(data, p_data, length);
        
        IM_FreeReceivedPacket(handle);
        
        received_packets++;
        
        switch(format)
        {
            case 0:
                if(IM_StandardGroupPacketReceivedCallback != NULL && length >= 2)
                {
                    if(IM_StandardGroupPacketReceivedCallback(handle, tab, 0x00, 0xFF, data[0], data[1], data + 2, length - 2, received_time))
                    {
                        processed_packets++;
                    }
                }
                break;
            case 1:
                if(IM_StandardGroupPacketReceivedCallback != NULL && length >= 3)
                {
                    if(IM_StandardGroupPacketReceivedCallback(handle, tab, 0x00, data[0], data[1], data[2], data + 3, length - 3, received_time))
                    {
                        processed_packets++;
                    }
                }
                break;
            case 2:
                if(IM_StandardGroupPacketReceivedCallback != NULL && length >= 4)
                {
                    if(IM_StandardGroupPacketReceivedCallback(handle, tab, data[0], data[1], data[2], data[3], data + 4, length - 4, received_time))
                    {
                        processed_packets++;
                    }
                }
                break;
            case 3:
                if(IM_SpecialGroupPacketReceivedCallback != NULL && length >= 1)
                {
                    if(IM_SpecialGroupPacketReceivedCallback(handle, tab, 0x00, 0xFF, data[0], data + 1, length - 1, received_time))
                    {
                        processed_packets++;
                    }
                }
                break;
            case 4:
                if(IM_SpecialGroupPacketReceivedCallback != NULL && length >= 2)
                {
                    if(IM_SpecialGroupPacketReceivedCallback(handle, tab, 0x00, data[0], data[1], data + 2, length - 2, received_time))
                    {
                        processed_packets++;
                    }
                }
                break;
            case 5:
                if(IM_SpecialGroupPacketReceivedCallback != NULL && length >= 3)
                {
                    if(IM_SpecialGroupPacketReceivedCallback(handle, tab, data[0], data[1], data[2], data + 3, length - 3, received_time))
                    {
                        processed_packets++;
                    }
                }
                break;
            default:
                break;
        }
    }
    
    if(received_packet_num != NULL)
    {
        *received_packet_num = received_packets;
    }
    if(processed_packet_num != NULL)
    {
        *processed_packet_num = processed_packets;
    }
    return res;
}

//从接收器队队首提取一个标准组数据包
//队列为空返回0  成功返回 1，队首数据包格式非标准组返回 2，数据格式为标准组但数据段不符合规范返回 3
//返回负数表示错误
int8_t IM_StandardGroupGetReceivedPacket
(
    int8_t handle, uint32_t *tab, 
    uint8_t *source, uint8_t *target, uint8_t *channel, uint8_t *property, 
    void **para, uint8_t *length, uint32_t *received_time
)
{
    int8_t res = 0;
    uint32_t temp_tab;
    uint8_t temp_format;
    uint8_t temp_length;
    uint32_t temp_received_time;
    uint8_t *temp_data;
    
    res = IM_GetReceivedPacket(handle, &temp_tab, &temp_format, &temp_data, &temp_length, &temp_received_time, 1);//允许重复访问
    if(res <= 0)
    {
        return res;
    }
    switch(temp_format)
    {
        case 0:
            if(temp_length >= 2)
            {
                if(tab != NULL) *tab = temp_tab;
                if(source != NULL) *source = 0x00;
                if(target != NULL) *target = 0xFF;
                if(channel != NULL) *channel = temp_data[0];
                if(property != NULL) *property = temp_data[1];
                
                if(length != NULL) *length = temp_length - 2;
                if(para != NULL) *para = temp_data + 2;
                if(received_time != NULL) *received_time = temp_received_time;
                
                return 1;
            }
            break;
        case 1:
            if(temp_length >= 3)
            {
                if(tab != NULL) *tab = temp_tab;
                if(source != NULL) *source = 0x00;
                if(target != NULL) *target = temp_data[0];
                if(channel != NULL) *channel = temp_data[1];
                if(property != NULL) *property = temp_data[2];
                
                if(length != NULL) *length = temp_length - 3;
                if(para != NULL) *para = temp_data + 3;
                if(received_time != NULL) *received_time = temp_received_time;
                
                return 1;
            }
            break;
        case 2:
            if(temp_length >= 4)
            {
                if(tab != NULL) *tab = temp_tab;
                if(source != NULL) *source = temp_data[0];
                if(target != NULL) *target = temp_data[1];
                if(channel != NULL) *channel = temp_data[2];
                if(property != NULL) *property = temp_data[3];
                
                if(length != NULL) *length = temp_length - 4;
                if(para != NULL) *para = temp_data + 4;
                if(received_time != NULL) *received_time = temp_received_time;
                
                return 1;
            }
            break;
        default:
            return 2;
    }
    return 3;
}

//从接收器队队首提取一个标准组数据包
//队列为空返回0  成功返回 1，队首数据包格式非专用组返回 2，数据格式为专用组但数据段不符合规范返回 3
//返回负数表示错误
int8_t IM_SpecialGroupGetReceivedPacket
(
    int8_t handle, uint32_t *tab, 
    uint8_t *source, uint8_t *target, uint8_t *cmd, 
    void **para, uint8_t *length, uint32_t *received_time
)
{
    int8_t res = 0;
    uint32_t temp_tab;
    uint8_t temp_format;
    uint8_t temp_length;
    uint32_t temp_received_time;
    uint8_t *temp_data;
    
    res = IM_GetReceivedPacket(handle, &temp_tab, &temp_format, &temp_data, &temp_length, &temp_received_time, 1);
    if(res <= 0)
    {
        return res;
    }
    switch(temp_format)
    {
        case 3:
            if(temp_length >= 1)
            {
                if(tab != NULL) *tab = temp_tab;
                if(source != NULL) *source = 0x00;
                if(target != NULL) *target = 0xFF;
                if(cmd != NULL) *cmd = temp_data[0];
                
                if(length != NULL) *length = temp_length - 1;
                if(para != NULL) *para = temp_data + 1;
                if(received_time != NULL) *received_time = temp_received_time;
                
                return 1;
            }
            break;
        case 4:
            if(temp_length >= 2)
            {
                if(tab != NULL) *tab = temp_tab;
                if(source != NULL) *source = 0x00;
                if(target != NULL) *target = temp_data[0];
                if(cmd != NULL) *cmd = temp_data[1];
                
                if(length != NULL) *length = temp_length - 2;
                if(para != NULL) *para = temp_data + 2;
                if(received_time != NULL) *received_time = temp_received_time;
                
                return 1;
            }
            break;
        case 5:
            if(temp_length >= 3)
            {
                if(tab != NULL) *tab = temp_tab;
                if(source != NULL) *source = temp_data[0];
                if(target != NULL) *target = temp_data[1];
                if(cmd != NULL) *cmd = temp_data[2];
                
                if(length != NULL) *length = temp_length - 3;
                if(para != NULL) *para = temp_data + 3;
                if(received_time != NULL) *received_time = temp_received_time;
                
                return 1;
            }
            break;
        default:
            return 2;
    }
    return 3;
}

/*************************************************************************************************
Function    : IM_StdGroupPackDataToTransmitter
Description : 按照数据格式x进行封装,并输出到指定的发送器缓存队列
Calls       : IM_PackDataToTransmitter
Called By   : 用户指令
Input       : handle 指定的队列句柄
              key    随机密钥索引  低8位有效，超过255表示不对数据加密
              tab    包标签        低28位有效 超过0x0FFFFFFF 表示不带标签

              source  来源设备
              target  目标设备
              channel 目标通道
              property 目标属性

              para   参数起始地址
              len    参数长度
              check_type  数据帧校验方式
Output      : 
Return      : 1-? 数据包实际长度(包含帧头帧尾校验等)
              0   不可能出现
              -1  句柄所对应的收发器不存在
              -2  当前发送器正在被访问
              -3  发送器队列满
              -4  剩余连续缓冲区不足
Others      : 
*************************************************************************************************/
int32_t IM_StandardGroupPackDataToTransmitter
(
    int8_t handle, uint16_t key, uint32_t tab,
    uint8_t source, uint8_t target, uint8_t channel, uint8_t property,  
    void *para, uint8_t len, CHECK_TYPE check_type
)
{
    if(source == 0x00)
    {
        if(target == 0xFF)
        {
            uint8_t temp[2]; temp[0] = channel; temp[1] = property;
            return IM_PackDataToTransmitter(handle, key, tab, 0, temp, sizeof(temp), (uint8_t *)para, len, check_type);
        }
        else
        {
            uint8_t temp[3]; temp[0] = target; temp[1] = channel; temp[2] = property;
            return IM_PackDataToTransmitter(handle, key, tab, 1, temp, sizeof(temp), (uint8_t *)para, len, check_type);
        }
    }
    else
    {
        uint8_t temp[4]; temp[0] = source; temp[1] = target; temp[2] = channel; temp[3] = property;
        return IM_PackDataToTransmitter(handle, key, tab, 2, temp, sizeof(temp), (uint8_t *)para, len, check_type);
    }
}

/*************************************************************************************************
Function    : IM_SpecialGroupPackDataToTransmitter
Description : 按照数据格式x进行封装,并输出到指定的发送器缓存队列
Calls       : IM_PackDataToTransmitter
Called By   : 用户指令
Input       : handle 指定的队列句柄
              key    随机密钥索引
              tab    包标签

              source 来源设备号
              target 目标设备号
              cmd    操作指令

              para   参数起始地址
              len    参数长度
              check_type  数据帧校验方式
Output      : 
Return      : 1-? 数据包实际长度(包含帧头帧尾校验等)
              0   不可能出现
              -1  句柄所对应的收发器不存在
              -2  当前发送器正在被访问
              -3  发送器队列满
              -4  剩余连续缓冲区不足
Others      : 
*************************************************************************************************/
int32_t IM_SpecialGroupPackDataToTransmitter
(
    int8_t handle, uint16_t key, uint32_t tab,
    uint8_t source, uint8_t target, uint8_t cmd,
    void *para, uint8_t len, CHECK_TYPE check_type
)
{
    if(source == 0x00)
    {
        if(target == 0xFF)
        {
            return IM_PackDataToTransmitter(handle, key, tab, 3, &cmd, sizeof(cmd), (uint8_t *)para, len, check_type);
        }
        else
        {
            uint8_t temp[2]; temp[0] = target; temp[1] = cmd;
            return IM_PackDataToTransmitter(handle, key, tab, 4, temp, sizeof(temp), (uint8_t *)para, len, check_type);
        }
    }
    else
    {
        uint8_t temp[3]; temp[0] = source; temp[1] = target; temp[2] = cmd;
        return IM_PackDataToTransmitter(handle, key, tab, 5, temp, sizeof(temp), (uint8_t *)para, len, check_type);
    }
}
