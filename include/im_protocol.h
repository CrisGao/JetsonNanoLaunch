/*************************************************************************************************
Copyright   : 2019  INMOTION Tech. Co., Ltd.
File name   : im_protocol.h
Description : 乐行新版协议库
              本文件功能范围 : 完成数据包封装、解析以及收发缓存等功能(队列形式)，但不对数据段的具体
                               格式做任何限制，即不保证数据段Data的格式符合Format中描述的数据段格式
                               ，收发标准的数据格式建议使用im_std_data_format.h中的数据包收发及处理
                               接口
Author      : 龙乐坪(Lorin)
Version     : V3.4
Date        : 2019.05.23
History     : V1.0.0   2018.08.24   第一个版本
              V1.0.1   2018.08.28   增加 IM_GetDataFromTransmitter，提供更丰富的使用方法
              V1.0.2   2018.09.06   将接收器与发送器合并，接口形式进一步简化
              V1.1.0   2018.09.13   基本帧格式中增加Tab字段，长度0-4字节
                                    对协议Format字段定义进行了适当修改，表示校验方式的位由bit5-7
                                    改为bit4-5, bit7用于表示是否符合标准帧格式，bit6用于表示是否
                                    含包标签(Tab)
                                    对数据帧解析部分进行了部分修改，允许数据长度为0，此时实际有意
                                    义的只有Tab字段
                                    另外修复了CRC32计算时未将Format及Length计入的bug
                                    数据封包接口增加了校验方式的独立选项
              V1.1.1   2018.09.20   将函数IM_DataReceive 中静态变量 pre_sys_time 移至结构体
                                    RECEIVER_INSTANCE 中，解决该函数可重入性问题
              V1.1.2   2018.09.26   修复了内部多个函数句柄使用错误的bug(原本应使用输入形参"handle")
                                    但实际使用了"last_hanle", 该问题会导致在创建了多个收发器时，数
                                    据发生冲突
                                    数据格式化封装接口完善，目前支持格式代号为 1-6的6种数据格式(原
                                    本只支持数据格式3)
                                    更改了部分函数及变量名称，统一将 package 改为 packet
              V1.1.3   2018.09.29   将format中用于表示数据格式的位段由4位改为3位，并更改了极简通讯
                                    的部分格式，使之可适应一主多从及多主多从的通讯方式
              V1.2.0   2018.10.09   增加了数据加密功能，扩展了数据发送部分的函数接口，每种数据格式
                                    根据是否加密以及是否含Tab有四种发送方法，增加了数据接收的规范化
                                    回调接口，数据收发做到每种格式一一对应，并将规范化的接口转移至
                                    单独的源文件 "im_std_data_format.c"
              V1.2.1   2018.10.10   修复了数据包解析时，密钥Key部分未计入校验导致数据包无法正常接收
                                    的BUG
              V1.2.2   2018.10.12   增加了格式0 在格式1的基础上省略设备号
              V1.2.3   2018.10.15   IM_GetReceivedPacket增加 参数 repeatable 用于表示是否允许对同一
                                    数据包重复访问，方便进行数据包筛选时的多次访问操作
              V1.2.4   2018.10.22   修复了收发器队列最大深度定义超过127时IM_CreateTransceiverStatic
                                    内部对链表初始化时内存越界的BUG
                                    更改了 IM_GetPacketFromTransmitter 的形参顺序
              V1.2.5   2018.11.14   增加了密码表重置功能，以支持动态加密，并且支持数据收发可使用不
                                    同的密码表加密
              V1.2.6   2018.12.18   增加了数据接收校验错误，帧格式错误统计功能，可分别通过
                                    IM_GetCheckErrorCount，IM_GetFormatErrorCount 获取校验错误包数
                                    及帧格式错误包数
                                    IM_DataReceive 增加参数 uint8_t *time_out 用于获取上个数据包是
                                    否接收超时
              V1.3.0   2019.04.10   增加了 IM_GetReceivedPacketCount 及 
                                    IM_GetPacketCountOfTransmitter 用于随时获取收发器队列中剩余数
                                    据包的个数
                                    增加了 IM_GetFreeBufferSizeOfReceiver 及 
                                    IM_GetFreeBufferSizeOfTransmitter 用于获取收发器队列剩余可分配
                                    的连续缓冲区大小
                                    去除 IM_GetCheckErrorCount，IM_GetFormatErrorCount 
                                    替代以 IM_GetReceiverStatisticsData 可获取更丰富的接收器接收统
                                    计数据
                                    增加了 IM_ClearReceiverStatisticsData 用于清零接收器统计数据
                                    增加了 IM_GetProtocolConfig 用于获取协议配置
                                    将 IM_PackData 中 FRAME_FORMAT frame_format 以 volatile 修饰
                                    防止优化等级过高时编译器报警告
                                    修复了接收队列缓冲区分配的一处bug，该bug会导致 接收器读（读出）
                                    指针在写（接收）指针之后时，将队列缓冲区剩余长度计算为0从而导
                                    致数据包被丢弃的bug
                                    略微优化了接收化队列缓冲区分配的策略在队列为空时，不管整个缓冲
                                    区大小是否足够接收当前数据帧，都会将缓冲区切换到首部，以获取最
                                    高利用率
              V1.3.1   2019.04.12   更改了 IM_PackDataToTransmitter 的返回值类型（uint16_t 改为
                                    int32_t）及定义，支持对队列满，队列缓存不足等异常的检测
              V1.3.2   2019.04.17   增加了包接收时间记录功能，IM_GetReceivedPacket 可以获取到收到
                                    每个包的时间，该功能主要用于防止队列阻塞，多线程应用中，当某个
                                    线程长时间未收到该有的数据包，并且检查到队首数据包长时间未被处
                                    理时可以丢弃队首数据包
              V3.3     2019.05.06   增加了数据自动覆盖特性，允许该特性后在数据收发过程中，当队列满
                                    时新加入队列的数据会自动覆盖队列中最旧的数据包，用户可以在创建
                                    收发器时配置该属性，也可以在任意时刻使用 IM_EnableDataOverlay 
                                    进行设置，该函数不受互斥锁限制，可在任意时刻调用
                                    完善了 IM_GetPacketFromTransmitter 与 IM_GetDataFromTransmitter
                                    这两种从发送器中提取数据的方法之间的内部同步机制，避免了交叉使
                                    用时 IM_GetDataFromTransmitter 可能会获取到与 
                                    IM_GetPacketFromTransmitter 重复的数据或者有可能获取到已经无效
                                    的被覆盖的数据
                                    IM_GetPacketFromTransmitter 增加参数 uint8_t repeatable， 支持
                                    对发送器队列数据包重复访问
                                    更改了版本号的定义规则 8位主版本号 + 8位次版本号 + 16位编译号
                                    编译号不记录在修订历史记录中
                                    修复了包接收时间记录功能的一处BUG
              V3.4     2019.05.23   修复了内部数据打包发送的一处BUG，该BUG会导致当待发送数据包的标
                                    签号低7位(或多于7位)为0时，实际发出的标签号序列缺失部分字节
*************************************************************************************************/

/*************************************************************************************************
协议帧结构 0xAA 0xAA Format (Key) (Tab) Length Data (Check)
长度(bytes)  1    1    1     0-1   0-4     1   0-255 0-4

各字段含义如下:
Format 描述整个数据帧个格式  包括是否加密，是否含标签(Tab)，数据段(Data)格式，以及校验方式
Key    随机密钥索引
Tab    包标签 高位在前，每个字节的低7位表示具体数值，最高位表示其后是否还有Tab数据，1表示还有，
       0表示本字节已是最后一个字节
Length 数据段(Data)长度
Data   数据段，以下详细描述
Check  校验

数据段(Data)格式共8种，各格式下数据段(Data)具体内容定义如下（除para[]外其他均为一个字节）
格式 0 Data :                   channel + property + para[]
格式 1 Data :          device + channel + property + para[]
格式 2 Data : source + device + channel + property + para[]
格式 3 Data :                   cmd + para[]
格式 4 Data :          target + cmd + para[]
格式 5 Data : source + target + cmd + para[]
格式 6 保留
格式 7 保留

数据帧总长度范围 4-268字节  最简单的数据帧为 0xAA 0xAA 0x00 0x00

协议宗旨: 高效、灵活、简单、通用、安全
*************************************************************************************************/

/*************************************************************************************************
本库特点
优点 ：移植简单   ：接口形式按最简封装，内部集成了所有数据结构和算法，尽可能减小对外部的依赖（如钩
                    子函数，队列处理，内存分配）无论接收还是发送，输出的数据地址是可以直接使用的，
                    不再需要用户开辟缓冲区存储
       使用灵活   ：可以配置为集中发送，集中处理的模式，也可配置为 发送->请求模式
                    内部功能可裁剪，方便针对资源紧张的应用进行定制
       空间效率高 ：所有操作均在用户创建的缓冲区内进行，中间不需要开辟任何临时缓冲区，不会带来额外
                    RAM的消耗
       时间效率高 ：数据在解析时直接输入到缓冲区，这个缓冲区是唯一的数据备份，此后不存在任何拷贝动
                    作，数据打包发送只有一次遍历+拷贝动作
       高速       ：数据包解析耗时均匀，转义字节，校验计算等采用分布式处理方式（在单个字节输入时已
                    经处理）对于高波特率，连续多个数据包串联也能实时处理，不容易丢包
       抗干扰     ：对于诸如数据包混杂（比如一个包被另一个包打断）的情况也能解析出其中完整的数据包
       兼容性     ：库仅包含数据的处理，不涉及任何底层，方便移植到以C，C++为开发语言的任何平台
       安全性     ：以句柄形式封装，用户不可访问内部接收器/发送器/队列/链表等数据结构，保证了安全性
缺点 ：库本身内容较多，目前仍有许多功能（如是否使用队列）暂不支持裁剪，即使裁剪至最简功能编译后依然
       会占用较大ROM，不适合 ROM 低于8K的系统
*************************************************************************************************/

#ifndef _IM_PROTOCOL_H_
#define _IM_PROTOCOL_H_

#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum{
    NO_CHECK = 0, //无校验
    PARITY,       //奇偶校验
    CRC16,        //CRC16校验
    CRC32,        //CRC32校验
}CHECK_TYPE;//校验方式

typedef struct
{
    uint8_t max_transceiver_count;       //支持的最大接收器个数 合法范围 1-128
    uint8_t max_receiver_queue_depth;    //接收器队列最大深度 (即接收器能缓存的数据包个数) 合法范围 1-127
    uint8_t max_transmitter_queue_depth; //发送器队列最大深度 (即发送器能缓存的数据包个数) 合法范围 1-127
    uint8_t support_crc32       : 1;     //支持CRC32
    uint8_t support_encryption  : 1;     //支持密文传输
    uint8_t support_time_record : 1;     //支持记录每个数据包的接收时间
}PROTOCOL_CONFIG;

typedef struct
{
    uint32_t packets_count;           //成功接收的总包数统计
    uint32_t check_error_count;       //校验错误统计
    uint32_t format_error_count;      //帧格式错误统计
    uint32_t queue_full_count;        //因队列满而导致接收失败的数据包计数
    uint32_t no_enough_buf_count;     //因为剩余连续缓存不足而接收失败的数据包计数
}RECEIVER_STATISTICS;

/*************************************************************************************************
Function    : IM_GetProtocolVersion
Description : 获取库版本号
Calls       : 
Called By   : 
Input       : 
Output      : 
Return      : 协议版本 bit24-31 表示主版本号 bit16-23 表示次版本号  bit0-15 表示编译号
Others      : 
*************************************************************************************************/
uint32_t IM_GetProtocolVersion(void);


/*************************************************************************************************
Function    : IM_GetProtocolConfig
Description : 获取协议配置 这些配置依据库的不同编译版本而定
Calls       : 
Called By   : 
Input       : 
Output      : 
Return      : 见 结构体 PROTOCOL_CONFIG 定义
Others      : 
*************************************************************************************************/
PROTOCOL_CONFIG IM_GetProtocolConfig(void);


/*************************************************************************************************
Function    : IM_CreateReceiverDynamic
Description : 以动态分配方式创建一个收发器(包含一个接收器及一个发送器), 内存由本函数内部分配，注意创建
              后无法销毁
Calls       : 
Called By   : 用户定义的初始化程序
Input       : rx_buf_size: 接收器数据缓冲区大小(1-32768),将会影响可以缓存的已接收数据总长度(只包含
                           帧结构中“数据段”内容，不含帧头帧尾、校验、标签等)
              rx_overlay : 允许数据接收覆盖 0 禁止  1 允许(当队列满时 新收到的数据包可以覆盖旧的数据包)
              tx_buf_size: 发送器数据缓冲区大小(1-32768),将会影响可以缓存的待发送数据总长度(指完整
                           的数据帧总长度，除了数据段内容外，还包括帧头帧尾、校验、标签等)
              tx_overlay : 允许数据发送覆盖  0 禁止  1 允许(当队列满时  新加入发送器队列的数据包可以覆
                           盖旧的数据包)
Output      : 
Return      : 0-127创建的接收器句柄  该句柄用于访问已创建的接收器/发送器
              -1 创建的接收器数量已达上限
              -2 输入参数错误
              -3 内存不足
Others      : 注意数据接收时，除了所有已接收数据包的数据段总长度有限制不超过rx_buf_size外，总的数据
              包个数也是有限制的，不会超过内部定义的接收器队列最大深度，数据发送时亦是如此
*************************************************************************************************/
int8_t IM_CreateTransceiverDynamic(uint16_t rx_buf_size, uint8_t rx_overlay, 
                                   uint16_t tx_buf_size, uint8_t tx_overlay);


/*************************************************************************************************
Function    : IM_CreateReceiverStatic
Description : 以静态分配方式创建一个收发器(包含一个接收器及一个发送器), 内存需要用户在外部预先分配，注意
              创建后无法销毁
Calls       : 
Called By   : 用户定义的初始化程序
Input       : rx_buf     : 已分配的接收缓冲区起始地址
              rx_buf_size: 接收器数据缓冲区大小(1-32768),将会影响可以缓存的已接收数据总长度(只包含
                           帧结构中“数据段”内容，不含帧头帧尾、校验、标签等)
              rx_overlay : 允许数据接收覆盖 0 禁止  1 允许(当队列满时 新收到的数据包可以覆盖旧的数据包)
              tx_buf     : 已分配的发送缓冲区起始地址
              tx_buf_size: 发送器数据缓冲区大小(1-32768),将会影响可以缓存的待发送数据总长度(指完整
                           的数据帧总长度，除了数据段内容外，还包括帧头帧尾、校验、标签等)
              tx_overlay : 允许数据发送覆盖  0 禁止  1 允许(当队列满时 新加入发送器队列的数据包可以覆
                           盖旧的数据包)
Output      : 
Return      : 0-127创建的接收器句柄  该句柄用于访问已创建的接收器/发送器
              -1 创建的接收器数量已达上限
              -2 输入参数错误
Others      : 注意数据接收时，除了所有已接收数据包的数据段总长度有限制不超过rx_buf_size外，总的数据
              包个数也是有限制的，不会超过内部定义的接收器队列最大深度，数据发送时亦是如此
*************************************************************************************************/
int8_t IM_CreateTransceiverStatic(uint8_t *rx_buf, uint16_t rx_buf_size, uint8_t rx_overlay,
                                  uint8_t *tx_buf, uint16_t tx_buf_size, uint8_t tx_overlay);


/*************************************************************************************************
Function    : IM_EnableDataOverlay
Description : 允许/禁止 数据覆盖
Calls       : 
Called By   : 
Input       : rx_overlay : 允许数据接收覆盖  0 禁止  1 允许(当队列满时 新收到的数据包可以覆盖旧的数据包)
              tx_overlay : 允许数据发送覆盖  0 禁止  1 允许(当队列满时 加入发送器队列的数据包可以覆盖
                           旧的数据包)
Output      : 
Return      : 0   : 操作成功
              -1  : 句柄所指向的收发器不存在
Others      : 
*************************************************************************************************/
int8_t IM_EnableDataOverlay(int8_t handle, uint8_t rx_overlay, uint8_t tx_overlay);


/*************************************************************************************************
Function    : ResetReceiverCipherTable
Description : 重置收发器中接收器的密码表
Calls       : 
Called By   : 
Input       : handle : 目标收发器句柄
              table  : 新的密码表起始地址 注意 table 为 uint8_t[256]类型，传入的仅为地址，内部不对
                       密码表做备份，所以重置之后table所在内存不能销毁，如果需要再次更改密码表的数
                       值，不再需要调用本函数，直接更改table的内容即可
Output      : 
Return      : 0   : 操作成功
              -1  : 句柄所指向的收发器不存在
              -2  : table 不允许为空
Others      : 
*************************************************************************************************/
int8_t ResetReceiverCipherTable(int8_t handle, uint8_t *table);


/*************************************************************************************************
Function    : ResetTransmitterCipherTable
Description : 重置收发器中发送器的密码表
Calls       : 
Called By   : 
Input       : handle : 目标收发器句柄
              table  : 新的密码表起始地址 注意 table 为 uint8_t[256]类型，传入的仅为地址，内部不对
                       密码表做备份，所以重置之后table所在内存不能销毁，如果需要再次更改密码表的数
                       值，不再需要调用本函数，直接更改table的内容即可
Output      : 
Return      : 0   : 操作成功
              -1  : 句柄所指向的收发器不存在
              -2  : table 不允许为空
Others      : 
*************************************************************************************************/
int8_t ResetTransmitterCipherTable(int8_t handle, uint8_t *table);


/*************************************************************************************************
Function    : IM_ClearReceiverBuf
Description : 清空收发器中接收器的队列及缓冲区，放弃队列中所有已接收指令
Calls       : 
Called By   : 用户定义的异常处理程序
Input       : handle        : 目标收发器句柄
Output      : 
Return      : -1 句柄所指向的收发器不存在
              -2 目标收发器中的接收器正在被其他程序访问
Others      : 本函数一定程度上可替代 IM_FreeReceivedPacket 不同的是 IM_ClearReceiverBuf 会清空所有
              已接收的数据包，而不管其是否已被读取过，而 IM_FreeReceivedPacket 只能清空已使用
              IM_GetReceivedPacket访问过的队首数据包，如队首数据包尚未被读取，它会返回错误 -3
*************************************************************************************************/
int8_t IM_ClearReceiverBuf(int8_t handle);


/*************************************************************************************************
Function    : IM_DataReceive
Description : 向收发器的接收器注入原始数据流，并执行数据包解析
Calls       : 
Called By   : 用户提供的串口接收完成回调(如UART中断)
Input       : handle        : 目标收发器句柄
              rx_data       : 数据起始地址
              bytes_to_read : 数据长度（byte）
              sys_time      : 系统时间（ms）, 如不使用固定为0即可
              max_interval  : 如有分包情况，表示同一数据帧不同包之间最长时间间隔（ms）
                              其数值应根据物理层而定
Output      : *time_out     ：调用函数时，前一帧数据是否超时 （最大分包间隔 max_interval）
                              若有超时则 *time_out被赋值为 1 否则 *time_out 将不被操作 （维持原值）
                              所以用户应将传入的变量应初始为 0
                              该参数可指定为 NULL
Return      : 0-127 本次调用解析完成并成功添加至目标接收器队列的有效数据包数量
                    (理论最大数量等于定义的接收缓冲队列深度)
              -1 句柄所指向的接收器不存在
              -2 接收器正在被其他程序访问
Others      : 返回值为 0 在以下两种情况会出现  1、尚未解出完整的数据帧   2、接收器队列已满
              所以请及时调用 IM_GetReceivedPacket + IM_FreeReceivedPacket 或 IM_ClearReceiverBuf
              清理接收队列的数据，以免影响新的数据解析
*************************************************************************************************/
int8_t  IM_DataReceive(int8_t handle, uint8_t *rx_data, uint16_t bytes_to_read, uint8_t *time_out, 
                                      uint32_t sys_time, uint32_t max_interval);


/*************************************************************************************************
Function    : IM_GetReceivedPacket
Description : 从收发器的接收器缓存队列取出一个数据包，并共享队列内存(用户不必为该数据包分配内存)，
              注意该函数不允许重复访问，必须与 IM_FreeReceivedPacket 成对使用，重复访问将返回-3
Calls       : 
Called By   : 用户数据处理程序
Input       : handle : 目标收发器句柄
              format : 用于接收数据段格式的变量的地址
              data   : 用于接收数据段起始地址的指针变量的地址 (注意不需要为其分配内存)
              length : 用于接收数据段长度的变量地址
              received_time : 用于接收收到数据包的时间的变量的地址
              repeatable : 是否允许重复访问(即对同一数据包访问多次) 0 不允许  1 允许
                           如选择 不允许，则发生重复访问时函数返回 -3 ，用户必须调用
                           IM_FreeReceivedPacket 释放队首数据包或者使用IM_ClearReceiverBuf清空队列
                           才可继续访问
Output      : *tab    : 数据包标签，有效范围 0-0xFFFFFFFE 数值0xFFFFFFFF 表示无标签
              *format : 数据段格式 数值范围 0-7
              **data  : 数据段起始地址
              *length : 数据段长度
              *received_time : 收到本包数据的时间（以包接收完成的时间为准）
Return      : 1-127接收器队列中剩余数据包个数(含本包数据)
              0 接收器队列为空
              -1 句柄所指向的收发器不存在
              -2 收发器中的接收器正在被其他程序访问
              -3 对同一包数据重复访问(该错误只在 repeatable 置0时存在，此时必须使用
                 IM_FreeReceivedPacket 释放前一包数据后才能继续访问)
Others      : 每调用一次将从已接收的数据包队列中取出一包数据，直到队列为空
              注意若累积未处理的数据包数量达到最大限度，IM_DataReceive将暂停数据解析，等待处理完毕
              tab, format, data, length, received_time 均不可指定为 NULL ！
              包接收时间 received_time 主要用于防止队列阻塞，多线程应用中，当某个线程长时间未收到该有
              的数据包，并且检查到队首数据包长时间未被处理时可以丢弃队首数据包
*************************************************************************************************/
int8_t IM_GetReceivedPacket(int8_t handle, uint32_t *tab, uint8_t *format, 
                                           uint8_t **data, uint8_t *length, uint32_t *received_time, 
                                           uint8_t repeatable);


/*************************************************************************************************
Function    : IM_GetReceivedPacketCount
Description : 获取收发器中接收器队列中数据包的个数
Calls       : 
Called By   : 用户数据处理程序
Input       : handle : 目标收发器句柄
Output      : 
Return      : 0-127接收器队列中数据包个数
              -1 句柄所指向的收发器不存在
Others      : 本函数不造成接收器资源锁，也不受接收器资源锁影响，可在任意时刻调用
*************************************************************************************************/
int8_t IM_GetReceivedPacketCount(int8_t handle);


/*************************************************************************************************
Function    : IM_GetFreeBufferSizeOfReceiver
Description : 获取接收器队列中剩余可分配的最大连续缓冲区大小
Calls       : 
Called By   : 用户数据处理程序
Input       : handle : 目标收发器句柄
Output      : 
Return      : 0-？接收器队列中剩余可分配的连续缓冲区大小
              -1 句柄所指向的接收器不存在
              -2 接收器队列正忙
Others      : 
*************************************************************************************************/
int32_t IM_GetFreeBufferSizeOfReceiver(int8_t handle);


/*************************************************************************************************
Function    : IM_FreeReceivedPacket
Description : 释放从接收器队列中取出的数据包，释放从接收器缓存中共享的内存
              本操作将使接收器队首数据包移除，队列前进1位，使用 IM_GetReceivedPacket 后必须使用本函数，
              否则无法访问队列中下一个数据包
Calls       : 
Called By   : 用户数据处理程序
Input       : handle : 目标接收器句柄
Output      : 
Return      : 1-127队列中剩余数据包个数(含本包数据)
              0 接收队列空
              -1 句柄所指向的接收器不存在
              -2 接收器正在被其他程序访问
              -3 当前队首数据包尚未被读取
Others      : 注意本函数只能释放已经读取过的队首数据包，如未使用 IM_GetReceivedPacket 读取队首数据
              包，而在直接调用了本函数将返回 -3，并且不会执行释放队首数据包的操作
              如需不论数据包是否被读取而直接释放，请使用 IM_ClearReceiverBuf
*************************************************************************************************/
int8_t IM_FreeReceivedPacket(int8_t handle);


/*************************************************************************************************
Function    : IM_GetReceiverStatisticsData
Description : 获取收发器中接收器的统计数据
              可使用 IM_ClearReceiverStatisticsData 将统计数据清零
Calls       : 
Called By   : 
Input       : handle : 目标收发器句柄
Output      : 
Return      : 接收器统计数据 详见 RECEIVER_STATISTICS 定义 若句柄所指向的接收器不存在将返回全0
Others      : 
*************************************************************************************************/
RECEIVER_STATISTICS IM_GetReceiverStatisticsData(int8_t handle);


/*************************************************************************************************
Function    : IM_ClearReceiverStatisticsData
Description : 清零收发器中接收器统计数据 这些数据包括  校验错误的数据包，格式错误的数据包，总成功接收的
              数据包
Calls       : 
Called By   : 
Input       : handle : 目标收发器句柄
Output      : 
Return      : 0 句柄所对应的收发器不存在 1 操作成功
Others      : 
*************************************************************************************************/
uint8_t IM_ClearReceiverStatisticsData(int8_t handle);


/*************************************************************************************************
Function    : IM_GetPacketFromTransmitter
Description : 从指定收发器的发送器队列取出待发送的数据包，并共享队列内存(用户不需要为该数据包分配内存)，
              注意必须与 IM_FreePacketFromTransmitter 成对使用
Calls       : 
Called By   : 用户指令
Input       : handle : 指定的队列句柄
              data   : 用于接收待发送数据起始地址的指针变量的地址 (注意不需要为其分配内存)
              length : 用于接收待发送数据长度的变量地址
Output      : **data 数据包起始地址
              *length  数据包总长度
              repeatable : 是否允许重复访问(即对同一数据包访问多次) 0 不允许  1 允许
                           如选择 不允许，则发生重复访问时函数返回 -3 ，用户必须调用
                           IM_FreePacketFromTransmitter 释放队首数据包或者使用 IM_ClearTransmitterBuf
                           清空队列才可继续访问
Return      : 1-127剩余待发送的数据包数量(包含当前获取的数据包)
              0  发送队列为空
              -1 对应句柄的缓冲队列不存在
              -2 队列为空
              -3 队列正忙
Others      : data, length 不可指定为 NULL !
*************************************************************************************************/
int8_t IM_GetPacketFromTransmitter(int8_t handle, uint8_t **data, uint16_t *length, uint8_t repeatable);


/*************************************************************************************************
Function    : IM_GetReceivedPacketCount
Description : 获取发送器队列中剩余数据包的个数
Calls       : 
Called By   : 用户数据处理程序
Input       : handle : 目标收发器句柄
Output      : 
Return      : 0-127队列中数据包个数
              -1 句柄所指向的收发器不存在
Others      : 本函数不造成接收器资源锁，也不受接收器资源锁影响，可在任意时刻调用
*************************************************************************************************/
int8_t IM_GetPacketCountOfTransmitter(int8_t handle);


/*************************************************************************************************
Function    : IM_GetFreeBufferSizeOfTransmitter
Description : 获取收发器队列中剩余可分配的最大连续缓冲区大小
Calls       : 
Called By   : 用户数据处理程序
Input       : handle : 目标收发器句柄
Output      : 
Return      : 0-？发送器队列中剩余可分配的连续缓冲区大小
              -1 句柄所指向的收发器不存在
              -2 发送器队列正忙
Others      : 
*************************************************************************************************/
int32_t IM_GetFreeBufferSizeOfTransmitter(int8_t handle);


/*************************************************************************************************
Function    : IM_FreePacketFromTransmitter
Description : 释放从发送器队列获取的数据包，释放从发送器缓存中共享的内存 
              与 IM_GetPacketFromTransmitter 配套使用，(如底层已发送完成或者已将指令Copy了副本，需
              要使用此接口释放 IM_GetPacketFromTransmitter 所占用的队列资源)
Calls       : 
Called By   : 用户指令
Input       : handle 指定的队列句柄
Output      : 
Return      : 1-127剩余待发送的数据包数量(包含刚被释放的数据包，即如果队列中剩余1个数据包那么调用后将
              返回1，但调用后实际的数据包个数已经为0)
              0  发送队列已经为空
              -1 对应句柄的缓冲队列不存在
              -2 队列为空
              -3 队列正忙
Others      : 
*************************************************************************************************/
int8_t IM_FreePacketFromTransmitter(int8_t handle);


/*************************************************************************************************
Function    : IM_GetDataFromTransmitter
Description : 从指定收发器的发送缓冲队列取出指定长度的数据(如果队列中所有数据总长度小于指定长度，
              将只返回实际长度的数据)，可以取出的最大数据长度即为创建的收发器时tx_buf_size的大小
              如果介意共享内存需要free操作，可以用此种方式，输出的数据不再以指令为单位，一次获取的
              数据可能包含多条指令，也可能只是一条指令的一部分，这取决于 buf_size 大小
              
              注意：对于接收器不提供类似的读取方式，因为只有完整的数据包才能被分析处理
Calls       : 
Called By   : 用户指令
Input       : handle 指定的队列句柄
              buf_size  输出缓冲区大小
Output      : *data  输出缓冲区起始地址
Return      : 实际的数据长度
              -1 对应句柄的缓冲队列不存在
              -2 队列为空
Others      : 如果队列中数据长度小于缓冲区长度则只输出队列实际数据长度
              data 不可指定为 NULL !
*************************************************************************************************/
int32_t IM_GetDataFromTransmitter(int8_t handle, uint8_t *data, uint16_t buf_size);


/*************************************************************************************************
Function    : IM_ClearTransmitterBuf
Description : 清空数据发送队列
Calls       : 
Called By   : 用户指令
Input       : handle 指定的收发器句柄
Output      : 
Return      : 0 操作成功
              -1 对应句柄的收发器不存在
              -2 发送器正忙
Others      : 
*************************************************************************************************/
int8_t IM_ClearTransmitterBuf(int32_t handle);


/*************************************************************************************************
Function    : IM_AddPacketToTransmitter
Description : 将数据打包并添加至指定收发器的发送器队列
Calls       : 
Called By   : 用户指令
Input       : 公共参数
              handle 指定的队列句柄
              key    随机密钥索引  有效范围 0-0xFF 超过此范围表示不对数据加密
              tab    包标签        有效范围 0-0x0FFFFFFF  超过此范围表示不使用标签
              format 数据段格式    低3位(bit[0-2])有效，高5位(bit[3-7]) 将被忽略
                                   如果严格遵循规范，即将设备号，通道或功能号等传入data1,将其携带
                                   的参数传入data2, 那么format即表示data1的具体格式，每种格式对应
                                   的data1长度是固定的，但这里为了灵活起见(保留了用户直接将整条指
                                   令包括设备号，通道等以及参数传入data1(data2)而data2传入NULL的方
                                   式)，所以仍然给出了data1长度自定义接口 length1
                                   各格式定义见 im_packet_send_receive_method.h 或协议文档
              data1   第一段数据  一般用于表示 设备号，通道，属性，指令号等
              length1 第一段数据长度
              data2   第二段数据  一般用于表示指令的参数，传输的数据等  如无请指定为 NULL
              length2 第二段数据长度  如无请指定为 0
Output      : 
Return      : 1-? 数据包实际长度(包含帧头帧尾校验等)
              0   不可能出现
              -1  句柄所对应的收发器不存在
              -2  当前发送器正在被访问
              -3  发送器队列满
              -4  剩余连续缓冲区不足
Others      : 发送器会将data1与data2合并发送, 总数据长度为 len1+len2
              即接收方使用IM_GetReceivedPacket 获得的数据为data = data1, data2; length = len1+len2
*************************************************************************************************/
int32_t IM_PackDataToTransmitter(int8_t handle, uint16_t key, uint32_t tab, uint8_t format, 
                                 uint8_t *data1, uint8_t length1, uint8_t *data2, uint8_t length2,
                                 CHECK_TYPE check_type);

#ifdef __cplusplus
}
#endif

#endif //_IM_PROTOCOL_H_

/*************************************************************************************************
                                        六、使用示例
                              以物理层为uart为例,典型的使用方式如下
*************************************************************************************************/

/*************************************************************************************************
1 指令发送
__________________________________________________________________________________________________

//收发器句柄
int8_t transceiver1 = -1;

//创建一个收发器
transceiver1 = IM_CreateTransceiverDynamic(50, 0, 50, 0);

//用户处理程序
uint8_t cmd1, cmd2, cmd3;
uint32_t para1, para2, para3; 
//注 ：以下RAND() 应生成一个范围在 0-255之间的随机数
IM_PackDataToTransmitter(transceiver1, RAND(), 0xFFFFFFFF, 3, &cmd1, 1, &para1, sizoef(para1), PARITY);
IM_PackDataToTransmitter(transceiver1, RAND(), 0xFFFFFFFF, 3, &cmd2, 1, &para2, sizoef(para2), PARITY);
IM_PackDataToTransmitter(transceiver1, RAND(), 0xFFFFFFFF, 3, &cmd3, 1, &para3, sizoef(para3), PARITY);

//底层发送处理 有两种方式 

1.使用共享内存 (共享队列缓存)
uint8_t *p_data;
uint16_t length;
if(IM_GetPacketFromTransmitter(transceiver1, &length, &p_data) > 0)
{
    UART_SEND_DATA(p_data, length);
    
    //已发送完毕或者已经拷贝了副本后必须释放共享的内存
    IM_FreePacketFromTransmitter(transceiver1);
}

2.不使用共享内存 需要用户分配内存
uint8_t data[50];
uint16_t length;
length = IM_GetDataFromTransmitter(transceiver1, data, sizeof(data));
if(length > 0)
{
    UART_SEND_DATA(data, length);//数据输出到串口硬件 用户自行实现
}

*************************************************************************************************/


/*************************************************************************************************
2 数据接收及处理的相关配置 (以下将按有无操作系统分别进行介绍)
__________________________________________________________________________________________________

2.1 带操作系统
含操作系统的环境下建议创建独立线程进行处理

//接收器句柄
int8_t transceiver1 = -1;

//通讯接收处理线程
uint8_t MessageProcess()
{
    //初始化串口硬件
    UART_INIT();//用户自行实现
    
    //创建接收器
    transceiver1 = IM_CreateTransceiverDynamic(50, 0, 50, 0);//建议不小于50字节
    
    for(;;)
    {
        //等待通知
        if(OS_PendMsg())//用户自行实现
        {
            uint32_t tab;
            uint8_t format, length, *p_data;
            uint32_t received_time;
            
            //注意必须未防止“死包”堆积，必须用while
            while(IM_GetReceivedPacket(transceiver1, &tab, &format, &p_data, &length, &received_time, 0) > 0)
            {
                //用户处理函数
                UserProcess(format, p_data, length);//用户自行实现
                IM_FreeReceivedPacket(transceiver1);
            }
        }
    }
}

//串口接收数据回调
//回调函数由用户构造，必须提供的有  length : 接收到的数据长度  data : 接收到的数据的起始地址
void UartDataReceivedCallback(uint8_t *data, uint16_t length)
{
    //获取系统时间(ms)
    uint32_t sys_time = GET_SYS_TIME();//用户自行实现 如不使用 给定为0即可
    
    //数据输入接收器
    if(IM_DataReceive(transceiver1, data, length, NULL, sys_time, 20) > 0)
    {
        //收到有效数据包，发送信号量通知相应线程处理
        OS_PostMsg();//用户自行实现
    }
}
__________________________________________________________________________________________________

2.2 无操作系统
无操作系统的环境下按照数据包处理的位置不同,有三种配置方式 (这三种方式在带操作系统的环境下同样适用)

2.2.1 主循环直接处理

//接收器句柄
int8_t transceiver1 = -1;

//用户主程序
void main()
{
    //初始化串口硬件
    UART_INIT();//用户自行实现
    
    //创建接收器
    transceiver1 = IM_CreateTransceiverDynamic(50, 0, 50, 0);
    
    for(;;)
    {
        uint32_t tab;
        uint8_t format, length, *p_data;
        uint32_t received_time;
        
        if(IM_GetReceivedPacket(transceiver1, &tab, &format, &p_data, &length, &received_time, 0) > 0)
        {
            //用户处理函数
            UserProcess(format, p_data, length);//用户自行实现
            IM_FreeReceivedPacket(transceiver1);
        }
    }
}
//串口接收数据回调
void UartDataReceivedCallback(void)
{
    //获取系统时间(ms)
    uint32_t sys_time = GET_SYS_TIME();//用户自行实现
    //获取接收的数据长度
    uint16_t length = GET_UART_RECEIVED_DATA_LENGTH();//用户自行实现
    //数据输入接收器
    IM_DataReceive(transceiver1, UART_RX_BUF, length, NULL, sys_time, 20);
}


2.2.2 周期性定时处理

//接收器句柄
int8_t transceiver1 = -1;

//用户主程序
void main()
{
    //初始化串口硬件
    UART_INIT();//用户自行实现
    
    //创建接收器
    transceiver1 = IM_CreateTransceiverDynamic(50, 0, 50, 0);
}
//周期处理(建议周期不长于50ms)
uint8_t PeriodProcess()
{
    uint32_t tab;
    uint8_t format, length, *p_data;
    uint32_t received_time;
    
    if(IM_GetReceivedPacket(transceiver1, &tab, &format, &p_data, &length, &received_time, 0) > 0)
    {
        //用户处理函数
        UserProcess(format, p_data, length);//用户自行实现
        IM_FreeReceivedPacket(transceiver1);
    }
}
//串口接收数据回调
void UartDataReceivedCallback(void)
{
    //获取系统时间(ms)
    uint32_t sys_time = GET_SYS_TIME();//用户自行实现
    //获取接收的数据长度
    uint16_t length = GET_UART_RECEIVED_DATA_LENGTH();//用户自行实现
    //数据输入接收器
    IM_DataReceive(transceiver1, UART_RX_BUF, length, NULL, sys_time, 20);
}


2.2.3 接收的同时处理

//接收器句柄
int8_t transceiver1 = -1;

//用户主程序
void main()
{
    //初始化串口硬件
    UART_INIT();//用户自行实现
    
    //创建收发器
    transceiver1 = IM_CreateTransceiverDynamic(50, 0, 50, 0);
}
//串口接收数据回调
void UartDataReceivedCallback(void)
{
    if(IM_DataReceive(handle1, UART_RX_BUF, length, NULL, sys_time, 20) > 0)
    {
        uint32_t tab;
        uint8_t format, length, *p_data;
        uint32_t received_time;
        
        //注意必须未防止“死包”堆积，必须用while
        while(IM_GetReceivedPacket(transceiver1, &tab, &format, &p_data, &length, &received_time, 0) > 0)
        {
            //用户处理函数
            UserProcess(format, p_data, length);//用户自行实现
            IM_FreeReceivedPacket(transceiver1);
        }
    }
}

*************************************************************************************************/

/*************************************************************************************************
3 扩展应用方式

1、在一些数据传输应用中为了使程序具备良好的结构，发送和接收需要位于同一线程，建议用如下方式
uint8_t retry;
uint32_t time_out;
uint32_t tx_tab, rx_tab;
uint8_t format, length, *p_data;
uint32_t received_time;
uint8_t buf[50];
uint16_t length;

transceiver1 = IM_CreateTransceiverDynamic(50, 0, 50, 0);//创建收发器

IM_ClearReceiverBuf(transceiver1);//清空所有已接收的数据包

retry = 10;
time_out = 10;
do{
    uint8_t cmd = 1;
    uint32_t para = 0;
    tx_tab = RAND();
    IM_PackDataToTransmitter(transceiver1, RAND(), tx_tab, 3, &cmd, 1, &para, sizoef(para), PARITY);
    
    //将数据通过串口发送 (可用以下方式直接发送，也可在中断或专用数据发送线程处理)
    length = IM_GetDataFromTransmitter(transceiver1, buf, sizeof(buf));
    UART_SEND_DATA(buf, length);//数据输出到串口硬件 用户自行实现
    
    if(OS_PendMsg(time_out))//等待数据
    {
        int8_t left_package = IM_GetReceivedPacket(transceiver1, &rx_tab, &format, &p_data, &length, &received_time, 0);
        if(left_package == 1 && rx_tab == tx_tab)//只有一个数据包并且包标签一致
        {
            //处理
        }
        IM_ClearReceiverBuf(transceiver1);//清空所有已接收的数据包
    }
    tx_tab++;
}while(retry--);

2、 可防止队列中数据包未及时处理而导致队列阻塞的安全接收方法
（若使用多线程，并且每个线程只处理部分符合特定条件的数据帧，每个处理线程对于不符合条件的数据帧既不处理也不丢弃，这时可能需要用到）

方法一

#define MAX_HEARTBEAT_PACKET_LENGTH    50   //心跳包的最大长度
static PROTOCOL_CONFIG config = IM_GetProtocolConfig();//配置对于每个已编译库是固定的，调用一次即可

//串口接收数据回调
void UartDataReceivedCallback(void)
{
    static uint32_t receive_fail_count = 0;
    uint8_t time_out = 0;
    int8_t res;
    
    res = IM_DataReceive(handle1, UART_RX_BUF, length, &time_out, sys_time, 20);
    
    if(res > 0)
    {
        receive_fail_count = 0;
    }
    else if(res == 0)//若串口使用中断接收方式，为了避免每个字节都进行判断，降低效率，推荐使用 time_out 来对帧计数， 即条件改为 res == 0 && time_out == 1
    {
        receive_fail_count++;
        if(receive_fail_count >= 3)
        {
            if( IM_GetReceivedPacketCount(transceiver1) == config.max_receiver_queue_depth
             || IM_GetFreeBufferSizeOfReceiver(transceiver1) < MAX_HEARTBEAT_PACKET_LENGTH )//接收队列满或者队列剩余缓冲区可能不足
            {
                IM_ClearReceiverBuf(transceiver1);//清空接收器队列
            }
        }
    }
}

方法二

//串口接收数据回调
void UartDataReceivedCallback(void)
{
    static uint32_t receive_fail_count = 0;
    static RECEIVER_STATISTICS old_statistics = {0};
    RECEIVER_STATISTICS statistics;
    uint8_t time_out = 0;
    int8_t res;
    
    res = IM_DataReceive(handle1, UART_RX_BUF, length, &time_out, sys_time, 20);
    
    statistics = IM_GetReceiverStatisticsData(transceiver1);//获取接收器统计数据
    
    if(res > 0)
    {
        receive_fail_count = 0;
    }
    else if(res == 0)//若串口使用中断接收方式，为了避免每个字节都进行判断，降低效率，推荐使用 time_out 来区对帧计数， 即条件改为 res == 0 && time_out == 1
    {
        receive_fail_count++;
        if(receive_fail_count >= 3)
        {
            if(statistics.queue_full_count > old_statistics.queue_full_count 
            || statistics.no_enough_buf_count > old_statistics.no_enough_buf_count) //有新的队列满或者队列剩余内存不足事件发生
            {
                IM_ClearReceiverBuf(transceiver1);//清空接收器队列
            }
        }
    }
    
    old_statistics = statistics;//保存旧的统计数据
}

*************************************************************************************************/
