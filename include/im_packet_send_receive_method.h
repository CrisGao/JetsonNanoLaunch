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
#ifndef _IM_PACKET_SEND_RECEIVE_METHOD_H_
#define _IM_PACKET_SEND_RECEIVE_METHOD_H_

#include "stdint.h"
#include "im_protocol.h"

#ifdef __cplusplus
extern "C" {
#endif

//以下请勿更改
#define NO_ENCRYPT 0xFFFF     //作为数据发送接口的参数之一，表示不对数据加密
#define NO_TAB     0xFFFFFFFF //作为数据发送接口的参数之一，表示不使用标签

/*************************************************************************************************
数据段(Data)格式共8种，各格式下数据段(Data)具体内容定义如下（除para[]外其他均为一个字节）
标准组：
格式 0 Data :                   channel + property + para[]
格式 1 Data :          target + channel + property + para[]
格式 2 Data : source + target + channel + property + para[]
专用组：
格式 3 Data :                   cmd + para[]
格式 4 Data :          target + cmd + para[]
格式 5 Data : source + target + cmd + para[]
其他
格式 6 保留
格式 7 保留
*************************************************************************************************/

/*************************************************************************************************
特别注意 : 以下函数为数据包接收处理的不同方法，一般只选用其中一种方法即可

方法 1 最原始的接收处理方式 IM_GetReceivedPacket + IM_FreeReceivedPacket(或IM_ClearReceiverBuf)

方法 2 统一处理，按组别回调 IM_DataPacketProcess 或 IM_DataPacketProcessWithBuf + 
                            IM_StandardGroupPacketReceivedCallback +
                            IM_SpecialGroupPacketReceivedCallback
                            此种方式特别注意 IM_DataPacketProcess 或 IM_DataPacketProcessWithBuf
                            需要针对不同收发器分别调用
                            但其回调函数 IM_StandardGroupPacketReceivedCallback
                            及 IM_SpecialGroupPacketReceivedCallback 是唯一赋值，唯一调用的
                            即所有接收器共用这两个回调函数，用户通过传入的参数 handle 判别数据包
                            来源接收器

方法 3 按组别分别处理       IM_StandardGroupGetReceivedPacket + IM_SpecialGroupGetReceivedPacket +
                            IM_FreeReceivedPacket(或IM_ClearReceiverBuf)

警告！
为确保程序结构合理，防止因为访问冲突而导致应用层包丢失，以上接口应只在唯一的一个线程中调用，并且只
需要使用其中一组即可，如果实在需要多处并行使用或者需要在一个应用程序中使用两种及两种以上接收处理方
法，请确保同时期只使用其中之一
*************************************************************************************************/


/*************************************************************************************************
                     标准组及专用组数据包接收处理方法1 ：统一处理，分组回调
*************************************************************************************************/

/*************************************************************************************************
Function    : IM_DataPacketProcess, IM_DataPacketProcessWithBuf
Description : 标准组及专用组数据包处理，处理完成将清空接收器队列中的所有数据包
              注意！只能唯一赋值，如有多个接收器对象应该，在本回调函数内部根据 handle 分别处理
Calls       : IM_GetReceivedPacket (使用其不可重复访问的方法)
              IM_FreeReceivedPacket 所以外部无需再调用此函数
              IM_StandardGroupPacketReceivedCallback and IM_SpecialGroupPacketReceivedCallback
Called By   : 用户接收处理函数
Input       : handle 指定的接收器句柄
              received_packet_num  : 用于接收已收到的数据包总数的变量地址
              processed_packet_num : 用于接收已处理的数据包数量的变量地址
Output      : *received_packet_num : 收到的数据包数量
              *processed_packet_num: 已处理的数据包数量
Return      : 0 无任何异常
              -1 句柄所指向的接收器不存在
              -2 接收器忙
              -3 当前队列数据正在被其他应用程序访问
Others      : IM_DataPacketProcessWithBuf 与 IM_DataPacketProcess 区别在于 
              IM_DataPacketProcessWithBuf 不长期占用队列内存，但多256 bytes RAM 开销
*************************************************************************************************/
int8_t IM_DataPacketProcess(int8_t handle, uint8_t *received_packet_num, uint8_t *processed_packet_num);
int8_t IM_DataPacketProcessWithBuf(int8_t handle, uint8_t *received_packet_num, uint8_t *processed_packet_num);


/*************************************************************************************************
Function    : IM_StandardGroupPacketReceivedCallback
Description : 标准数据接收回调  需与 IM_DataPacketProcess 搭配使用
Calls       : 
Called By   : IM_DataPacketProcess
Input       : handle  : 包来源接收器
              tab     : 包标签
              source  : 来源设备  0x00表示未知来源设备
              target  : 目标设备  0xFF表示广播目标设备(即针对所有从设备)
              channel : 目标通道
              property: 目标属性
              para    : 参数起始地址
              length  : 参数para的长度
              received_time : 收到本包数据的时间（以包接收完成的时间为准）
Output      : 
Return      : 1-127处理完成的数据包数量
              0 没有有效的数据包
              -1 
              -2
Others      : 
*************************************************************************************************/
extern uint8_t (*IM_StandardGroupPacketReceivedCallback)
(
    int8_t handle, uint32_t tab, 
    uint8_t source, uint8_t target, uint8_t channel, uint8_t property,
    void *para, uint8_t length, uint32_t received_time
);


/*************************************************************************************************
Function    : IM_SpecialGroupPacketReceivedCallback
Description : 通用组数据接收回调  需与 IM_DataPacketProcess 搭配使用
Calls       : 
Called By   : IM_DataPacketProcess
Input       : handle : 包来源接收器
              tab    : 包标签
              target : 目标设备号  0x00表示未知来源设备
              source : 来源设备号  0xFF表示广播目标设备(即针对所有从设备)
              cmd    : 功能号
              para   : 参数起始地址
              length : 参数para的长度
              received_time : 收到本包数据的时间（以包接收完成的时间为准）
Output      : 
Return      : 1-127处理完成的数据包数量
              0 没有有效的数据包
              -1 
              -2
Others      : 
*************************************************************************************************/
extern uint8_t (*IM_SpecialGroupPacketReceivedCallback)
(
    int8_t handle, uint32_t tab, 
    uint8_t source, uint8_t target, uint8_t cmd, 
    void *para, uint8_t length, uint32_t received_time
);


/*************************************************************************************************
                        标准组与专用组数据包接收处理方法2 ：分组处理，各自独立
*************************************************************************************************/

/*************************************************************************************************
Function    : IM_StandardGroupGetReceivedPacket
Description : 从接收器缓存队列取出一个标准组数据包，并共享队列内存(用户不必为该数据包分配内存)，
              注意本函数应与 IM_SpecialGroupGetReceivedPacket 位于同一线程，先后调用，互补处理，不
              能只单方面处理一组数据包
              本函数及 IM_SpecialGroupGetReceivedPacket 都允许对同一数据包重复访问，两者先后处理完
              毕后必须使用 IM_FreeReceivedPacket 移除队首数据，这样才能移动队列，访问新的数据包。
Calls       : IM_GetReceivedPacket (使用其可重复访问的方法)
Called By   : 用户数据包接收处理线程
Input       : handle 指定的收发器句柄
              tab      : 用于接收包标签的变量地址
              source   : 用于接收来源设备号的变量地址  0xFF表示广播目标设备(即针对所有从设备)
              target   : 用于接收目标设备号的变量地址  0x00表示未知来源设备
              channel  : 用于接收通道号的变量地址
              property : 用于接收属性的变量地址
              para     : 用于接收数据段起始地址的指针变量的地址 (注意不需要为其分配内存)
              length   : 用于接收数据段长度的变量地址
              received_time : 用于接收收到数据包的时间的变量的地址
Output      : *tab     : 包标签
              *source  : 来源设备  0x00表示未知来源设备
              *target  : 目标设备  0xFF表示广播目标设备(即针对所有从设备)
              *channel : 目标通道
              *property: 目标属性
              **para   : 参数起始地址
              *length  : 参数para的长度
              *received_time : 收到本包数据的时间（以包接收完成的时间为准）
Return      : 0  接收器队列为空
              1 从队首成功获取一个标准组数据包  使用完毕必须 调用 IM_FreeReceivedPacket 释放资源
              2 当前队列非空，但队首数据包非标准组  请使用IM_SpecialGroupGetReceivedPacket读取，
                读取完毕请使用 IM_FreeReceivedPacket释放队首数据包
              3 数据格式为标准组但数据段不符合规范，请使用 IM_FreeReceivedPacket释放队首数据包
              -1 句柄所指向的接收器不存在
              -2 接收器忙
Others      : 只有当返回值 <=0 时可不必调用 IM_FreeReceivedPacket 释放资源
              tab, source, target, cmd, para, length 均可指定为 NULL，表示不需读出对应项
*************************************************************************************************/
int8_t IM_StandardGroupGetReceivedPacket
(
    int8_t handle, uint32_t *tab, 
    uint8_t *source, uint8_t *target, uint8_t *channel, uint8_t *property, 
    void **para, uint8_t *length, uint32_t *received_time
);


/*************************************************************************************************
Function    : IM_SpecialGroupGetReceivedPacket
Description : 从接收器缓存队列取出一个标准组数据包，并共享队列内存(用户不必为该数据包分配内存)，
              注意本函数应与 IM_StandardGroupGetReceivedPacket 位于同一线程，先后调用，互补处理，不
              能只单方面处理一组数据包
              本函数及 IM_StandardGroupGetReceivedPacket 都允许对同一数据包重复访问，两者先后处理完
              毕后必须使用 IM_FreeReceivedPacket 移除队首数据，这样才能移动队列，访问新的数据包。
Calls       : IM_GetReceivedPacket (使用其可重复访问的方法)
Called By   : 用户数据包接收处理线程
Input       : handle 指定的收发器句柄
              tab      : 用于接收包标签的变量地址
              source   : 用于接收来源设备号的变量地址  0xFF表示广播目标设备(即针对所有从设备)
              target   : 用于接收目标设备号的变量地址  0x00表示未知来源设备
              cmd      : 用于接收功能号的变量地址
              para     : 用于接收数据段起始地址的指针变量的地址 (注意不需要为其分配内存)
              length   : 用于接收数据段长度的变量地址
              received_time : 用于接收收到数据包的时间的变量的地址
Output      : *tab     : 包标签
              *source  : 来源设备  0x00表示未知来源设备
              *target  : 目标设备  0xFF表示广播目标设备(即针对所有从设备)
              *channel : 目标通道
              *property: 目标属性
              **para   : 参数起始地址
              *length  : 参数para的长度
              *received_time : 收到本包数据的时间（以包接收完成的时间为准）
Return      : 0  接收器队列为空
              1 从队首成功获取一个标准组数据包  使用完毕必须 调用 IM_FreeReceivedPacket 释放资源
              2 当前队列非空，但队首数据包非标准组  请使用IM_SpecialGroupGetReceivedPacket读取，
                读取完毕请使用 IM_FreeReceivedPacket释放队首数据包
              3 数据格式为专用组但数据段不符合规范，请使用 IM_FreeReceivedPacket释放队首数据包
              -1 句柄所指向的接收器不存在
              -2 接收器忙
Others      : 只有当返回值 <=0 时可不必调用 IM_FreeReceivedPacket 释放资源
              tab, source, target, cmd, para, length 均可指定为 NULL，表示不需读出对应项
*************************************************************************************************/
int8_t IM_SpecialGroupGetReceivedPacket
(
    int8_t handle, uint32_t *tab, 
    uint8_t *source, uint8_t *target, uint8_t *cmd, 
    void **para, uint8_t *length, uint32_t *received_time
);


/*************************************************************************************************
                                标准组与专用组数据发送方法
*************************************************************************************************/

/*************************************************************************************************
Function    : IM_StandardGroupPackDataToTransmitter
Description : 按照标准组格式进行封装,并输出到指定的发送器缓存队列
Calls       : IM_PackDataToTransmitter
Called By   : 用户应用程序
Input       : handle    : 指定的队列句柄
              key       : 随机密钥索引 有效范围0-0xFF 超过0xFF表示无密钥(可用 NO_ENCRYPT 表示)
              tab       : 包标签       有效范围0-0x0FFFFFFF 超过0x0FFFFFFF表示无标签(可用NO_TAB表示)
              source    : 来源设备 source为0表示未知来源，将按格式0(target==0xFF)或1(target!=0xFF)
                          发送，source非0将按数据格式2发送
              target    : 目标设备 target为0xFF表示广播设备，在source等于0的情况下，target设置为0xFF
                          将按数据格式0发送，否则按数据格式1发送
              channel   : 目标通道
              property  : 目标属性
              para      : 参数起始地址  指定为 NULL表示无参数
              len       : 参数para的长度  指定为 0 表示无参数
              check_type:数据帧校验方式
Output      : 
Return      : 1-? 数据包实际长度(包含帧头帧尾校验等)
              0   不可能出现
              -1  句柄所对应的收发器不存在
              -2  当前发送器正在被访问
              -3  发送器队列满
              -4  剩余连续缓冲区不足
Others      : para 指定为 NULL 或者 len指定为0 可表示无参数
*************************************************************************************************/
int32_t IM_StandardGroupPackDataToTransmitter
(
    int8_t handle, uint16_t key, uint32_t tab,
    uint8_t source, uint8_t target, uint8_t channel, uint8_t property,  
    void *para, uint8_t len, CHECK_TYPE check_type
);


/*************************************************************************************************
Function    : IM_SpecialGroupPackDataToTransmitter
Description : 按专用组格式进行封装,并输出到指定的发送器缓存队列
Calls       : IM_PackDataToTransmitter
Called By   : 用户应用程序
Input       : 公共参数
              handle    : 指定的队列句柄
              key       : 随机密钥索引 有效范围0-0xFF 超过0xFF表示无密钥(可用 NO_ENCRYPT 表示)
              tab       : 包标签       有效范围0-0x0FFFFFFF 超过0x0FFFFFFF表示无标签(可用NO_TAB表示)
              cmd       : 功能号
              source    : 来源设备 source为0表示未知来源，将按格式3(target==0xFF)或4(target!=0xFF)
                          发送，source非0将按数据格式5发送
              target    : 目标设备 target为0xFF表示广播设备，在source等于0的情况下，target设置为0xFF
                          将按数据格式3发送，否则按数据格式4发送
              para      : 参数起始地址  指定为 NULL表示无参数
              len       : 参数para的长度  指定为 0 表示无参数
              check_type: 数据帧校验方式
Output      : 
Return      : 1-? 数据包实际长度(包含帧头帧尾校验等)
              0   不可能出现
              -1  句柄所对应的收发器不存在
              -2  当前发送器正在被访问
              -3  发送器队列满
              -4  剩余连续缓冲区不足
Others      : para 指定为 NULL 或者 len指定为0 可表示无参数
*************************************************************************************************/
int32_t IM_SpecialGroupPackDataToTransmitter
(
    int8_t handle, uint16_t key, uint32_t tab,
    uint8_t source, uint8_t target, uint8_t cmd,
    void *para, uint8_t len, CHECK_TYPE check_type
);

#ifdef __cplusplus
}
#endif

#endif //_IM_PACKET_SEND_RECEIVE_METHOD_H_
