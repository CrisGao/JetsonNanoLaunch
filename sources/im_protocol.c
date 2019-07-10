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

#include <string.h>
#include <stdlib.h>
#include "im_protocol.h"

//本库版本号
#define PROTOCOL_VERSION       0x03040001  //8位主版本号 + 8位次版本号 + 16位编译号  V3.3.1

//以下宏为用户配置项  可根据系统资源及需求在合法范围内自由更改

//支持的最大收发器个数 合法范围 1-128
#define MAX_INSTANCE_NUM       (2)

//接收器队列最大深度 (即接收器能缓存的数据包个数) 合法范围 1-127
#define MAX_RX_QUEUE_LENGTH    (5)

//发送器队列最大深度 (即发送器能缓存的数据包个数) 合法范围 1-127
#define MAX_TX_QUEUE_LENGTH    (5)

//支持CRC32   0 不支持  1 支持
#define SUPPORT_CRC32          (0)

//支持加密    0 不支持  1 支持
#define SUPPORT_ENCRYPTION     (1)

//支持记录每个数据包接收时间    0 不支持  1 支持
#define SUPPORT_TIME_RECORD    (0)

//以下宏请勿更改
//用于协议解析的内部状态机各状态定义
//必须满足 PROTOCOL_IDLE/FIND_HEAD/READ_FORMAT/READ_KEY 数值最小，
//且 PROTOCOL_IDLE < FIND_HEAD < READ_FORMAT < READ_KEY
#define PROTOCOL_IDLE     0
#define FIND_HEAD         1
#define READ_FORMAT       2
#define READ_KEY          3
#define READ_TAB          4
#define READ_LENGTH       5

#define READ_DATA         6
#define READ_DATA_SUM     7
#define READ_DATA_CRC16   8
#define READ_DATA_CRC32   9

#define READ_SUM          10
#define READ_CRC16        11
#define READ_CRC32        12

#define FRAME_HEADER      0xAA  //帧头
#define ESCAPE_BYTE       0xA5  //转义字符

#ifdef __GNUC__
#define GNUC_PACKED __attribute__ ((packed))
#else
#define GNUC_PACKED
#endif
#ifdef __arm
#define ARM_PACKED __packed
#else
#define ARM_PACKED
#endif
#ifdef WIN32
#pragma pack(1)
#endif

typedef ARM_PACKED struct
{
    uint8_t data_format : 3; //数据格式
    uint8_t encryption  : 1; //数据是否加密
    uint8_t check_type  : 2; //校验方式
    uint8_t tab_exist   : 1; //标签Tab是否存在
    uint8_t extend      : 1; //扩展格式
}GNUC_PACKED FRAME_FORMAT; //帧格式（format）位定义

#ifdef WIN32
#pragma pack()
#endif

//链表节点定义
typedef struct QUEUE_BUF
{
    uint8_t read_busy;      //当期队首数据正在被应用程序访问
    FRAME_FORMAT format;    //帧格式
    uint32_t tab;           //标签
    uint8_t length;         //数据长度
    uint16_t data_offset;   //数据在缓冲区的偏移地址
#if (SUPPORT_TIME_RECORD)
    uint32_t received_time;  //收到本包数据的时间 ms 
#endif
    struct QUEUE_BUF *next; //下一个节点
    struct QUEUE_BUF *last; //上一个节点
}RX_LINK_LIST;

//链表节点定义
//发送数据不关注格式
typedef struct QUEUE_BUF2
{
    uint8_t read_busy;       //缓冲区正在被底层或应用程序访问 主要用于防止数据包重复读取，以及未读取就被销毁
    uint16_t data_offset;    //数据在缓冲区的偏移地址
    uint16_t data_length;
    struct QUEUE_BUF2 *next; //下一个节点
    struct QUEUE_BUF2 *last; //下一个节点
}TX_LINK_LIST;

//接收器定义
typedef struct
{
    //数据接收相关
    uint8_t *rx_buf;             //数据接收缓冲区
    uint16_t rx_buf_size;        //接收缓冲区大小
    
    //数据接收环形队列(数据包缓存区)相关成员
    RX_LINK_LIST rx_buf_array[MAX_RX_QUEUE_LENGTH]; //所有队列结点 (队列以环形链表形式存在，链表的每个结点对应一个接收缓冲区)
    RX_LINK_LIST *rx_buf_list;   //当前输入结点(队尾)
    RX_LINK_LIST *read_buf_list; //当前输出结点(队首)
    uint8_t packets_to_read;     //队列长度 (剩余待读取的数据包个数)
    
    uint8_t rx_lock;             //互斥锁
    
    //数据帧解析相关变量
    uint8_t step;                //数据帧解析步骤
    uint8_t overlay;             //允许数据覆盖（当队列满或者缓冲区不足时，新的数据包将自动覆盖队列中最旧的数据包）
    uint32_t sys_time;           //收到前一个字节的系统时间 ms
    
    FRAME_FORMAT format;         //正在解析的数据包格式
    uint8_t rx_key_index;        //密钥索引 对应 ENCRYPTION_TABLE 中的成员 协议中以明文传送
    uint8_t tab_cnt;             //标签计数
    uint32_t tab;                //标签
    uint8_t length;              //正在解析的数据包包长
    uint8_t offset;              //当前接收数据位于缓冲区的地址偏移
    uint8_t ctl_flag;            //控制字符(ESCAPE_BYTE)标志
    
    uint8_t check_cnt;           //接收的校验字节计数
    uint8_t check_byte[4];       //用于接收校验字段
    
    uint8_t check_sum;           //实际计算的奇偶校验值
    uint16_t crc16;              //实际计算的CRC16值
    uint32_t crc32;              //实际计算的CRC32值
    
    uint8_t *cipher_table;       //密码表
    
    uint32_t packets_cnt;        //成功接收的总包数统计
    uint32_t check_error_cnt;    //校验错误统计
    uint32_t format_error_cnt;   //帧格式错误统计
    uint32_t queue_full_cnt;     //因队列满而导致接收失败的数据包计数
    uint32_t no_enough_buf_cnt;  //因为剩余连续缓存不足而接收失败的数据包计数
}RECEIVER_INSTANCE;

//发送器定义
typedef struct
{
    uint8_t *tx_buf;              //数据发送缓冲区
    uint16_t tx_buf_size;         //发送缓冲区大小
    
    //数据接收环形队列(数据包缓存区)相关成员
    TX_LINK_LIST tx_buf_array[MAX_TX_QUEUE_LENGTH]; //所有队列结点 (队列以环形链表形式存在，链表的每个结点对应一个接收缓冲区)
    TX_LINK_LIST *in_buf_list;    //当前输入结点(队尾)
    TX_LINK_LIST *out_buf_list;   //当前输出结点(队首)
    uint8_t packets_to_send;      //队列长度 (剩余待读取的数据包个数)
    
    uint8_t overlay;              //允许数据覆盖（当队列满或者缓冲区不足时，新的数据包将自动覆盖队列中最旧的数据包）
    
    uint8_t *p_data;              //数据指针
    uint16_t length;              //当前数据长度
    
    uint8_t tx_lock;              //互斥锁
    
    uint8_t *cipher_table;        //密码表
}TRANSMITTER_INSTANCE;

//收发器
typedef struct
{
    RECEIVER_INSTANCE  receivers;
    TRANSMITTER_INSTANCE  transmitters;
}IM_INSTANCE;

//最后一个收发器句柄
static int8_t   last_handle = -1;

//收发器实例
static IM_INSTANCE instance[MAX_INSTANCE_NUM] = {0};

//MODBUS CRC-16表 逆序
static const uint16_t CRC16_MODBUS_TABLE[256] = 
{
    0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
    0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
    0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
    0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
    0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
    0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
    0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
    0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
    0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
    0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
    0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
    0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
    0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
    0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
    0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
    0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
    0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
    0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
    0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
    0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
    0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
    0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
    0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
    0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
    0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
    0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
    0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
    0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
    0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
    0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
    0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
    0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040
};

#if (SUPPORT_CRC32)
static const uint32_t CRC32_TABLE[256] = 
{
   0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA,
   0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
   0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988,
   0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
   0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE,
   0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
   0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC,
   0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
   0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,
   0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
   0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940,
   0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
   0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116,
   0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
   0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,
   0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
   0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A,
   0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
   0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818,
   0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
   0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,
   0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
   0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C,
   0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
   0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2,
   0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
   0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,
   0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
   0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086,
   0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
   0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4,
   0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
   0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A,
   0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
   0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8,
   0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
   0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE,
   0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
   0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC,
   0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
   0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252,
   0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
   0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60,
   0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
   0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
   0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
   0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04,
   0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
   0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A,
   0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
   0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38,
   0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
   0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E,
   0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
   0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C,
   0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
   0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2,
   0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
   0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0,
   0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
   0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6,
   0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
   0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,
   0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
};
#endif

//加密表
#if (SUPPORT_ENCRYPTION)
static const uint8_t DEFAULT_CIPHER_TABLE[256] = 
{
    0xED,0xB9,0xF2,0x58,0x8B,0xE3,0xC5,0xEC,0x14,0xC0,0x9D,0x42,0x60,0x17,0x0F,0x9E,
    0x0B,0x98,0xDC,0x16,0x9D,0xDF,0x66,0xD1,0xC3,0x9C,0xC7,0xBF,0x4D,0xFD,0xBD,0xCE,
    0xEB,0x95,0xE6,0x96,0x25,0x4F,0xA8,0x8D,0x52,0x49,0x96,0xD0,0xC2,0x80,0xBB,0xCC,
    0x90,0x02,0x09,0x3E,0x1E,0x7B,0x39,0xF3,0x47,0x4A,0x20,0x00,0x6F,0x9B,0x2F,0xA9,
    0x1A,0x50,0xA2,0x0E,0x06,0x21,0x9C,0xFB,0x1E,0x59,0x76,0x4A,0x83,0xD3,0xCA,0x4D,
    0xDC,0xFB,0x31,0xC2,0xA6,0xDE,0x14,0x11,0x60,0x99,0x3E,0xC6,0x7A,0x41,0xC5,0x5B,
    0xF6,0xCB,0xD9,0x4D,0x4F,0x62,0x2B,0x15,0x3D,0xAF,0x4D,0xCB,0x7A,0x6D,0x32,0x40,
    0xAD,0x38,0xE7,0x7A,0x3B,0x1D,0x25,0x19,0x3D,0x3E,0xF4,0xE1,0xD5,0x32,0x8E,0xF2,
    0xF1,0x40,0xA4,0x4E,0xD9,0x83,0xDC,0x90,0x44,0x64,0x6C,0xEB,0x02,0xCC,0xA6,0x6E,
    0x6E,0x2B,0x6F,0x8B,0x67,0x09,0x3B,0xD2,0x4E,0x8C,0x3F,0x95,0x99,0x9C,0xC5,0x66,
    0x67,0x7B,0xA5,0xD7,0xBA,0x89,0xE9,0x72,0x4D,0xA1,0xD6,0x57,0xF6,0x35,0x54,0x9D,
    0x2D,0xE0,0x5B,0x6A,0x68,0x66,0x05,0xA8,0x0F,0x07,0xEB,0xEE,0x56,0x31,0x77,0x94,
    0x05,0xF8,0x07,0xED,0xE7,0x87,0x98,0x3F,0x3E,0xB2,0xB1,0x31,0xFA,0x2D,0x94,0x7D,
    0x99,0xC6,0x1F,0x83,0x66,0xD9,0x11,0x32,0x15,0x8B,0xF8,0x04,0x61,0x8E,0x04,0x81,
    0xF0,0x92,0x80,0xD9,0x7E,0xB7,0xD4,0x24,0xD1,0xA4,0xAD,0x21,0x89,0x81,0x55,0x8D,
    0x50,0x56,0x6A,0x28,0xD8,0xD5,0x82,0x89,0x5A,0x1C,0xEA,0x42,0x2A,0x08,0x6A,0xDF,
};
#endif


//数据加密及转义
static uint16_t encrypt_escape_data(void *src, uint8_t src_size, void *dest, uint16_t dest_bufsize, uint8_t *key_index, const uint8_t *encryption_table)
{
    uint8_t *p_dest,*p_src;
    uint8_t i,real_size = 0;
    uint8_t temp, temp_key;
    
    p_src = (uint8_t *)src;
    p_dest = (uint8_t *)dest;
    
    if(key_index != NULL && encryption_table != NULL)
    {
        temp_key = *key_index;
        for(i = 0; i < src_size; i++)
        {
            if(real_size >= dest_bufsize)//
                return 0xFFFF;
            temp = *p_src ^ encryption_table[temp_key];
            if(temp == FRAME_HEADER || temp == ESCAPE_BYTE)
            {
                *p_dest++ = ESCAPE_BYTE;
                real_size++;
            }
            *p_dest++ = temp;
            real_size++;
            p_src++;
            temp_key++;
        }
        *key_index = temp_key;
    }
    else
    {
        for(i = 0; i < src_size; i++)
        {
            if(real_size >= dest_bufsize)//
                return 0xFFFF;
            temp = *p_src;
            if(temp == FRAME_HEADER || temp == ESCAPE_BYTE)
            {
                *p_dest++ = ESCAPE_BYTE;
                real_size++;
            }
            *p_dest++ = temp;
            real_size++;
            p_src++;
        }
    }
    return real_size;
}

//奇偶校验 计算
static uint8_t calc_check_sum(uint8_t * data, int32_t num, uint8_t init)
{
    uint8_t sum = init;
    while(num-- > 0)
    {
        sum ^= *data++;
    }
    return sum;
}

//CRC16 计算
static uint16_t calc_crc_16(uint8_t * data, uint32_t num, uint16_t init, const uint16_t *table)
{
    uint16_t crc16 = init;
    while(num--)
    {
        crc16 = (crc16 >> 8) ^ table[(crc16 ^ *data++) & 0xFF];
    }
    return crc16;
}

#if (SUPPORT_CRC32)
//CRC32 计算
static uint32_t calc_crc_32(uint8_t *data, uint32_t num, uint32_t init, const uint32_t *table)
{
    uint32_t crc32 = init;
    while (num--)
    {
        crc32 = (crc32 >> 8) ^ table[(crc32 ^ *data++) & 0xFF];
    }
    return crc32;
}
#endif

//获取协议版本
uint32_t IM_GetProtocolVersion(void)
{
    return (uint32_t)PROTOCOL_VERSION;
}

//获取协议配置
PROTOCOL_CONFIG IM_GetProtocolConfig(void)
{
    PROTOCOL_CONFIG config = {MAX_INSTANCE_NUM, MAX_RX_QUEUE_LENGTH, MAX_TX_QUEUE_LENGTH, SUPPORT_CRC32, SUPPORT_ENCRYPTION, SUPPORT_TIME_RECORD};
    return config;
}

//重置接收器密码表
int8_t ResetReceiverCipherTable(int8_t handle, uint8_t *table)
{
    if(handle < 0 || handle > last_handle)
    {
        return -1;
    }
    if(table == NULL)
    {
        return -2;
    }
    instance[handle].receivers.cipher_table = table;
    return 0;
}

//重置发送器密码表
int8_t ResetTransmitterCipherTable(int8_t handle, uint8_t *table)
{
    if(handle < 0 || handle > last_handle)
    {
        return -1;
    }
    if(table == NULL)
    {
        return -2;
    }
    instance[handle].transmitters.cipher_table = table;
    return 0;
}

//使用已分配的静态内存创建收发器
//注意接收缓冲区仅被用于存放收到的数据的数据段(Data)部分，而发送缓冲区将被用于存放包括帧头、校验在内的完整帧数据（即发送至底层的原始数据）
//所以数据接收时，能够解析的最大包长即等于 rx_buf_size，但数据发送时，能够发送的最大数据包长要 <= tx_buf_size
//实际有效数据长度为  实际长度 - 数据中FRAME_HEADER以及ESCAPE_BYTE的个数 - 帧头,format,length以及校验所占字节数
//对于长度为n的数据，实际占用缓冲区长度为 [n+4, 2*n+8],若希望能够发送长度为n的任意数据，建议缓冲区设置为 2*n+8(考虑校验最长4字节)
int8_t IM_CreateTransceiverStatic(uint8_t *rx_buf, uint16_t rx_buf_size, uint8_t rx_overlay,
                                  uint8_t *tx_buf, uint16_t tx_buf_size, uint8_t tx_overlay)
{
    uint8_t i;
    RECEIVER_INSTANCE *p_receiver;
    TRANSMITTER_INSTANCE *p_transmitter;
    
    if(last_handle+1 >= MAX_INSTANCE_NUM)
    {
        return -1;
    }
    if(rx_buf_size == 0 || rx_buf_size > 32768
    || tx_buf_size == 0 || tx_buf_size > 32768)
    {
        return -2;
    }
    
    last_handle++;
    
    //接收器配置
    p_receiver = &instance[last_handle].receivers;
    
    //密码表配置
    #if (SUPPORT_ENCRYPTION)
    p_receiver->cipher_table = (uint8_t *)DEFAULT_CIPHER_TABLE;
    #endif
    
    //缓冲区内存分配
    p_receiver->rx_buf = rx_buf;
    p_receiver->rx_buf_size = rx_buf_size;
    
    //构建MAX_RX_QUEUE_LENGTH个结点的环形链表
    for(i=0; i<MAX_RX_QUEUE_LENGTH; i++)
    {
        p_receiver->rx_buf_array[i].read_busy = 0;
        p_receiver->rx_buf_array[i].next = &(p_receiver->rx_buf_array[(i+1)%MAX_RX_QUEUE_LENGTH]);
        p_receiver->rx_buf_array[i].last = &(p_receiver->rx_buf_array[(i+MAX_RX_QUEUE_LENGTH-1)%MAX_RX_QUEUE_LENGTH]);
    }
    
    //输入结点(rx_buf_list),输出结点(read_buf_list)以及队列长度(packets_to_read)初始化
    p_receiver->rx_buf_list = &(p_receiver->rx_buf_array[0]);
    p_receiver->rx_buf_list->data_offset = 0;//初始化第一个结点的偏移
    
    p_receiver->read_buf_list = p_receiver->rx_buf_list;
    p_receiver->packets_to_read = 0;
    
    p_receiver->step = PROTOCOL_IDLE;
    
    p_receiver->packets_cnt = 0;
    p_receiver->check_error_cnt = 0;
    p_receiver->format_error_cnt = 0;
    p_receiver->queue_full_cnt = 0;
    p_receiver->no_enough_buf_cnt = 0;
    
    p_receiver->overlay = (rx_overlay == 0 ? 0 : 1);
    
    p_receiver->rx_lock = 0;
    
    
    //发送器配置
    p_transmitter = &instance[last_handle].transmitters;
    
    //密码表配置
    #if (SUPPORT_ENCRYPTION)
    p_transmitter->cipher_table = (uint8_t *)DEFAULT_CIPHER_TABLE;
    #endif
    
    //缓冲区内存分配
    p_transmitter->tx_buf = tx_buf;
    p_transmitter->tx_buf_size = tx_buf_size;
    
    //构建MAX_RX_QUEUE_LENGTH个结点的环形链表
    for(i=0; i<MAX_TX_QUEUE_LENGTH; i++)
    {
        p_transmitter->tx_buf_array[i].read_busy = 0;
        p_transmitter->tx_buf_array[i].next = &(p_transmitter->tx_buf_array[(i+1)%MAX_TX_QUEUE_LENGTH]);
        p_transmitter->tx_buf_array[i].last = &(p_transmitter->tx_buf_array[(i+MAX_TX_QUEUE_LENGTH -1)%MAX_TX_QUEUE_LENGTH]);
    }
    
    //输入结点(rx_buf_list),输出结点(read_buf_list)以及队列长度(packets_to_read)初始化
    p_transmitter->in_buf_list = &(p_transmitter->tx_buf_array[0]);
    p_transmitter->in_buf_list->data_offset = 0;//初始化第一个结点的偏移
    
    p_transmitter->out_buf_list = p_transmitter->in_buf_list;
    p_transmitter->packets_to_send = 0;
    
    p_transmitter->length = 0;
    
    p_transmitter->overlay = (tx_overlay == 0 ? 0 : 1);
    
    p_transmitter->tx_lock = 0;
    
    return last_handle;
}

//使用动态分配内存方式创建收发器
int8_t IM_CreateTransceiverDynamic(uint16_t rx_buf_size, uint8_t rx_overlay, uint16_t tx_buf_size, uint8_t tx_overlay)
{
    int8_t result;
    uint8_t *temp_ptr;
    
    temp_ptr = (uint8_t *)malloc(rx_buf_size + tx_buf_size);
    if(temp_ptr == NULL)
    {
        return -3;
    }
    
    result = IM_CreateTransceiverStatic(temp_ptr, rx_buf_size, rx_overlay, temp_ptr + rx_buf_size, tx_buf_size, tx_overlay);
    if(result < 0)
    {
        free(temp_ptr);
    }
    return result;
}

//允许/禁止数据覆盖选项
int8_t IM_EnableDataOverlay(int8_t handle, uint8_t rx_overlay, uint8_t tx_overlay)
{
    if(handle < 0 || handle > last_handle)
    {
        return -1;
    }
    instance[handle].receivers.overlay = (rx_overlay == 0 ? 0 : 1);
    instance[handle].transmitters.overlay = (tx_overlay == 0 ? 0 : 1);
    return 0;
}


//清空接收器
int8_t IM_ClearReceiverBuf(int8_t handle)
{
    int8_t i;
    
    RECEIVER_INSTANCE *p_receiver;
    if(handle < 0 || handle > last_handle)
    {
        return -1;
    }
    p_receiver = &instance[handle].receivers;
    
    if(p_receiver->rx_lock)
    {
        return -2;
    }
    
    p_receiver->rx_lock = 1;
    
    //将输入结点移动至输出结点(队首移动至队尾)，队列长度清零
    p_receiver->rx_buf_list = &(p_receiver->rx_buf_array[0]);
    p_receiver->rx_buf_list->data_offset = 0;//初始化第一个结点的偏移
    
    p_receiver->read_buf_list = p_receiver->rx_buf_list;
    p_receiver->packets_to_read = 0;
    
    for(i=0; i<MAX_RX_QUEUE_LENGTH; i++)
    {
        p_receiver->rx_buf_array[i].read_busy = 0;
    }
    
    //重置协议解析步骤
    p_receiver->step = PROTOCOL_IDLE;
    
    p_receiver->rx_lock = 0;
    return 0;
}

//数据帧解析器(从数据流中剥除帧头帧尾校验转义，并将有效数据存入接收器队列)
//本函数空间复杂度较高，这里主要考虑对时间复杂度的优化
//返回值
//未解析出有效数据包              0
//收到有效数据包返回              1
//校验错误返回                    2
//帧格式错误(TAB不合要求)         3
//因队列满而无法接收数据包         4
//因没有足够缓冲区而无法接收数据包  5
//不支持的数据帧(扩展帧)           6
static uint8_t protocol_analysis(RECEIVER_INSTANCE *p_receiver, uint8_t r_byte)
{
    if(p_receiver->step > FIND_HEAD)
    {
        if(p_receiver->ctl_flag)
        {
            p_receiver->ctl_flag = 0;
            
            if(r_byte != FRAME_HEADER && r_byte != ESCAPE_BYTE)//说明数据出错
            {
                p_receiver->step = PROTOCOL_IDLE;
                return 0;
            }
        }
        else
        {
            if(r_byte == ESCAPE_BYTE)
            {
                p_receiver->ctl_flag = 1;
                return 0;
            }
            else if(r_byte == FRAME_HEADER)
            {
                if(p_receiver->step == READ_FORMAT)
                {
                    return 0;//前面无转义字符，只可能是重复的包头 例如 AA AA AA， 以最后两个AA 作为包头
                }
                else//前面无转义字符，只可能是包头
                {
                    p_receiver->step = FIND_HEAD;//说明帧同步失败，跳转到帧头识别
                    return 0;
                }
            }
        }
        
        //数据解密
        #if (SUPPORT_ENCRYPTION)
        if(p_receiver->step > READ_KEY && p_receiver->format.encryption)
        {
            r_byte ^= p_receiver->cipher_table[p_receiver->rx_key_index++];
        }
        #endif
    }
    
    switch(p_receiver->step)
    {
        case PROTOCOL_IDLE:
            if(r_byte == FRAME_HEADER)
            {
                p_receiver->step = FIND_HEAD;
            }
            break;
        case FIND_HEAD:
            if(r_byte == FRAME_HEADER)
            {
                p_receiver->ctl_flag = 0;
                p_receiver->step = READ_FORMAT;
            }
            else
            {
                p_receiver->step = PROTOCOL_IDLE;
            }
            break;
        case READ_FORMAT:
            p_receiver->format = *(FRAME_FORMAT*)&r_byte;
            if(p_receiver->format.extend)
            {
                p_receiver->step = PROTOCOL_IDLE;//非标准格式暂不支持
                return 6;//不支持的数据帧
            }
            else
            {
                switch(p_receiver->format.check_type)//计数校验
                {
                    case 1:
                        p_receiver->check_sum = r_byte;
                        break;
                    case 2:
                        p_receiver->crc16 = 0xFFFF;
                        p_receiver->crc16 = (p_receiver->crc16 >> 8) ^ CRC16_MODBUS_TABLE[(p_receiver->crc16 ^ r_byte) & 0xFF];
                        break;
                    #if (SUPPORT_CRC32)
                    case 3:
                        p_receiver->crc32 = 0xFFFFFFFF;
                        p_receiver->crc32 = (p_receiver->crc32 >> 8) ^ CRC32_TABLE[(p_receiver->crc32 ^ r_byte) & 0xFF];
                        break;
                    #endif
                    default://无校验
                        break;
                }
                
                if(p_receiver->format.encryption)
                {
                    p_receiver->step = READ_KEY;
                }
                else
                {
                    if(p_receiver->format.tab_exist)
                    {
                        p_receiver->tab_cnt = 0;
                        p_receiver->tab = 0;
                        p_receiver->step = READ_TAB;
                    }
                    else
                    {
                        p_receiver->tab = 0xFFFFFFFF;
                        p_receiver->step = READ_LENGTH;
                    }
                }
            }
            break;
        case READ_KEY:
            switch(p_receiver->format.check_type)//计算校验
            {
                case 1:
                    p_receiver->check_sum ^= r_byte;
                    break;
                case 2:
                    p_receiver->crc16 = (p_receiver->crc16 >> 8) ^ CRC16_MODBUS_TABLE[(p_receiver->crc16 ^ r_byte) & 0xFF];
                    break;
                #if (SUPPORT_CRC32)
                case 3:
                    p_receiver->crc32 = (p_receiver->crc32 >> 8) ^ CRC32_TABLE[(p_receiver->crc32 ^ r_byte) & 0xFF];
                    break;
                #endif
                default:
                    break;
            }
            p_receiver->rx_key_index = r_byte;//密钥索引
            if(p_receiver->format.tab_exist)
            {
                p_receiver->tab_cnt = 0;
                p_receiver->tab = 0;
                p_receiver->step = READ_TAB;
            }
            else
            {
                p_receiver->tab = 0xFFFFFFFF;
                p_receiver->step = READ_LENGTH;
            }
            break;
        case READ_TAB:
            switch(p_receiver->format.check_type)//计算校验
            {
                case 1:
                    p_receiver->check_sum ^= r_byte;
                    break;
                case 2:
                    p_receiver->crc16 = (p_receiver->crc16 >> 8) ^ CRC16_MODBUS_TABLE[(p_receiver->crc16 ^ r_byte) & 0xFF];
                    break;
                #if (SUPPORT_CRC32)
                case 3:
                    p_receiver->crc32 = (p_receiver->crc32 >> 8) ^ CRC32_TABLE[(p_receiver->crc32 ^ r_byte) & 0xFF];
                    break;
                #endif
                default:
                    break;
            }
            p_receiver->tab |= (r_byte&0x7F);//tab按照高位在前排序
            p_receiver->tab_cnt++;
            if(r_byte & 0x80)//还有tab字节
            {
                if(p_receiver->tab_cnt >= 4)
                {
                    p_receiver->step = PROTOCOL_IDLE;//Tab不允许超过4字节
                    return 3;//格式错误
                }
                else
                {
                    p_receiver->tab <<= 7;
                }
            }
            else
            {
                p_receiver->step = READ_LENGTH;
            }
            break;
        case READ_LENGTH:
            if(p_receiver->packets_to_read == MAX_RX_QUEUE_LENGTH)//队列已满
            {
                if(p_receiver->overlay)//允许数据覆盖
                {
                    //丢弃队首数据包
                    p_receiver->read_buf_list = p_receiver->read_buf_list->next;//切换到下一个缓冲区
                    p_receiver->packets_to_read--;
                }
                else
                {
                    p_receiver->step = PROTOCOL_IDLE;
                    return 4;
                }
            }
            if(r_byte == 0)
            {
                p_receiver->length = r_byte;
                switch(p_receiver->format.check_type)
                {
                    case 0:
                        goto PACKAGE_RECEIVED;
                    case 1:
                        p_receiver->check_sum ^= r_byte;
                        p_receiver->step = READ_SUM;//空包且无校验，解析完成
                        break;
                    case 2:
                        p_receiver->crc16 = (p_receiver->crc16 >> 8) ^ CRC16_MODBUS_TABLE[(p_receiver->crc16 ^ r_byte) & 0xFF];
                        p_receiver->check_cnt = 0;
                        p_receiver->step = READ_CRC16;
                        break;
                    #if (SUPPORT_CRC32)
                    case 3:
                        p_receiver->crc32 = (p_receiver->crc32 >> 8) ^ CRC32_TABLE[(p_receiver->crc32 ^ r_byte) & 0xFF];
                        p_receiver->crc32 ^= 0xFFFFFFFF;
                        p_receiver->check_cnt = 0;
                        p_receiver->step = READ_CRC32;
                        break;
                    #endif
                    default:
                        p_receiver->step = PROTOCOL_IDLE;//暂不支持
                        break;
                }
            }
            else
            {
                //判断剩余连续缓冲区是否足够接收本帧数据，并分配以最佳内存块
                
                MALLOC:
                
                if(p_receiver->read_buf_list->data_offset <= p_receiver->rx_buf_list->data_offset)//读（读出）指针在写（接收）指针之前
                {
                    //有两片连续的缓冲区可选（分别在缓冲区首尾）
                    //分配策略 ： 最大缓冲区利用率优先  即：能不切换就不切换，即 “够用就行” 除非检测到接收器队列全空，则切换至首部缓冲区  以获得最大利用率
                    if(p_receiver->rx_buf_size - p_receiver->rx_buf_list->data_offset < r_byte)//尾部缓冲区不足
                    {
                        if(p_receiver->rx_buf_list == p_receiver->read_buf_list)//特殊情况
                        {
                            //首尾重合，缓冲区可能全空或全满
                            if(p_receiver->packets_to_read == 0)
                            {
                                //接收器队列空 则切换至首部缓冲区后可将首尾两片缓冲区可连成一片，可用的连续缓冲区大小为整个缓冲区大小，这样可得到最高利用率
                                p_receiver->rx_buf_list->data_offset = 0;//切换至首部缓冲区 由于指针重合 p_receiver->read_buf_list->data_offset = 0 可略
                                if(p_receiver->rx_buf_size < r_byte)
                                {
                                    goto BUF_LACK_PROCESS;//即使整个缓冲区大小仍不足以接收该数据帧
                                }
//                                else
//                                {
//                                    p_receiver->rx_buf_list->data_offset = 0;//切换至首部缓冲区 由于指针重合 p_receiver->read_buf_list->data_offset = 0 可略
//                                }
                            }
                            else//应不可能存在 否则 p_receiver->packets_to_read == MAX_RX_QUEUE_LENGTH 在前面会 break
                            {
                                goto BUF_LACK_PROCESS;//接收器队列满
                            }
                        }
                        else
                        {
                            if(p_receiver->read_buf_list->data_offset < r_byte)//首部缓冲区也不足
                            {
                                goto BUF_LACK_PROCESS;//没有连续的足够缓冲区
                            }
                            else
                            {
                                p_receiver->rx_buf_list->data_offset = 0;//切换至首部缓冲区
                            }
                        }
                    }
                }
                else//读（读出）指针在写（接收）指针之后
                {
                    //缓冲区位于中部，且只有一个
                    if(p_receiver->read_buf_list->data_offset - p_receiver->rx_buf_list->data_offset < r_byte)
                    {
                        goto BUF_LACK_PROCESS;//没有连续的足够缓冲区
                    }
                }
                
                p_receiver->offset = 0;
                p_receiver->length = r_byte;
                switch(p_receiver->format.check_type)
                {
                    case 0:
                        p_receiver->step = READ_DATA;//空包且无校验，解析完成
                        break;
                    case 1:
                        p_receiver->check_sum ^= r_byte;
                        p_receiver->step = READ_DATA_SUM;//空包且无校验，解析完成
                        break;
                    case 2:
                        //先计算format+length的校验
                        p_receiver->crc16 = (p_receiver->crc16 >> 8) ^ CRC16_MODBUS_TABLE[(p_receiver->crc16 ^ r_byte) & 0xFF];
                        p_receiver->step = READ_DATA_CRC16;//空包且无校验，解析完成
                        break;
                    #if (SUPPORT_CRC32)
                    case 3:
                        p_receiver->crc32 = (p_receiver->crc32 >> 8) ^ CRC32_TABLE[(p_receiver->crc32 ^ r_byte) & 0xFF];
                        p_receiver->step = READ_DATA_CRC32;//空包且无校验，解析完成
                        break;
                    #endif
                    default:
                        p_receiver->step = PROTOCOL_IDLE;//暂不支持
                        break;
                }
                
                break;
                
                BUF_LACK_PROCESS: //缓冲区不足的处理
                
                if(p_receiver->overlay && p_receiver->packets_to_read != 0)
                {
                    //丢弃队首数据包
                    p_receiver->read_buf_list = p_receiver->read_buf_list->next;
                    p_receiver->packets_to_read--;
                    goto MALLOC;//重新分配接收缓存
                }
                else
                {
                    p_receiver->step = PROTOCOL_IDLE;//没有连续的足够缓冲区
                    return 5;
                }
            }
            break;
        case READ_DATA:
            p_receiver->rx_buf[p_receiver->rx_buf_list->data_offset + p_receiver->offset++] = r_byte;
            if(p_receiver->offset == p_receiver->length)
            {
                goto PACKAGE_RECEIVED;
            }
            break;
        case READ_DATA_SUM:
            p_receiver->rx_buf[p_receiver->rx_buf_list->data_offset + p_receiver->offset++] = r_byte;
            p_receiver->check_sum ^= r_byte;
            if(p_receiver->offset == p_receiver->length)
            {
                p_receiver->step = READ_SUM;
            }
            break;
        case READ_DATA_CRC16:
            p_receiver->rx_buf[p_receiver->rx_buf_list->data_offset + p_receiver->offset++] = r_byte;
            p_receiver->crc16 = (p_receiver->crc16 >> 8) ^ CRC16_MODBUS_TABLE[(p_receiver->crc16 ^ r_byte) & 0xFF];
            if(p_receiver->offset == p_receiver->length)
            {
                p_receiver->check_cnt = 0;
                p_receiver->step = READ_CRC16;
            }
            break;
        #if (SUPPORT_CRC32)
        case READ_DATA_CRC32:
            p_receiver->rx_buf[p_receiver->rx_buf_list->data_offset + p_receiver->offset++] = r_byte;
            p_receiver->crc32 = (p_receiver->crc32 >> 8) ^ CRC32_TABLE[(p_receiver->crc32 ^ r_byte) & 0xFF];
            if(p_receiver->offset == p_receiver->length)
            {
                p_receiver->crc32 ^= 0xFFFFFFFF;
                p_receiver->check_cnt = 0;
                p_receiver->step = READ_CRC32;
            }
            break;
        #endif
        case READ_SUM:
            if(p_receiver->check_sum == r_byte)
            {
                goto PACKAGE_RECEIVED;
            }
            else
            {
                p_receiver->step = PROTOCOL_IDLE;
                return 2;
            }
            //break;
        case READ_CRC16:
            p_receiver->check_byte[p_receiver->check_cnt] = r_byte;
            if(p_receiver->check_cnt == 1)
            {
                if(*(uint16_t *)(&p_receiver->check_byte[0]) == p_receiver->crc16)
                {
                    goto PACKAGE_RECEIVED;
                }
                else
                {
                    p_receiver->step = PROTOCOL_IDLE;
                    return 2;
                }
            }
            else
            {
                p_receiver->check_cnt++;
            }
            break;
        #if (SUPPORT_CRC32)
        case READ_CRC32:
            p_receiver->check_byte[p_receiver->check_cnt] = r_byte;
            if(p_receiver->check_cnt == 3)
            {
                if(*(uint32_t *)(&p_receiver->check_byte[0]) == p_receiver->crc32)
                {
                    goto PACKAGE_RECEIVED;
                }
                else
                {
                    p_receiver->step = PROTOCOL_IDLE;
                    return 2;
                }
            }
            else
            {
                p_receiver->check_cnt++;
            }
            break;
        #endif
        default:
            p_receiver->step = PROTOCOL_IDLE;
            break;
    }
    
    return 0;
    
    PACKAGE_RECEIVED:
    
    p_receiver->rx_buf_list->format = p_receiver->format;
    p_receiver->rx_buf_list->tab = p_receiver->tab;
    p_receiver->rx_buf_list->length = p_receiver->length;
    p_receiver->rx_buf_list->read_busy = 0;
#if (SUPPORT_TIME_RECORD)
    p_receiver->rx_buf_list->received_time = p_receiver->sys_time;//收到本包数据的时间 ms 
#endif
    
    //设置下个缓冲区地址
    //注意 p_receiver->rx_buf_list->data_offset + p_receiver->length 可能等于 p_receiver->rx_buf_size
    p_receiver->rx_buf_list->next->data_offset = (p_receiver->rx_buf_list->data_offset + p_receiver->length) % p_receiver->rx_buf_size;
    //以下应确保为原子操作
    p_receiver->packets_to_read++;
    p_receiver->rx_buf_list = p_receiver->rx_buf_list->next;//切换缓冲区
    
    p_receiver->step = PROTOCOL_IDLE;
    
    return 1;
}

//返回收到的数据包数量
int8_t IM_DataReceive(int8_t handle, uint8_t *rx_data, uint16_t bytes_to_read, uint8_t *time_out, uint32_t sys_time, uint32_t max_interval)
{
    uint8_t result;
    uint8_t package_num = 0;
    RECEIVER_INSTANCE *p_receiver;
    
    if(handle < 0 || handle > last_handle)
    {
        return -1;
    }
    
    p_receiver = &instance[handle].receivers;
    
    //数据包间隔时间限制
    if(sys_time - p_receiver->sys_time > max_interval)
    {
        //重置协议解析步骤
        if(p_receiver->step > READ_FORMAT)//收到帧头及格式即认为数据帧解析开始，
        {
            if(time_out != NULL) *time_out = 1;//数据帧接收超时
        }
        p_receiver->step = PROTOCOL_IDLE;
    }
    p_receiver->sys_time = sys_time;
    
    if(p_receiver->rx_lock)
    {
        return -2;
    }
    p_receiver->rx_lock = 1;
    
    //逐字节解析
    while(bytes_to_read--)
    {
        result = protocol_analysis(p_receiver, *rx_data++);
        if(result == 0)//最高频结果 帧未解析完毕
        {
            continue;
        }
        else if(result == 1)//次高频结果 解出有效数据帧
        {
            package_num++;
            p_receiver->packets_cnt++;
        }
        else if(result == 2)//以下为其他异常结果 低频出现
        {
            p_receiver->check_error_cnt++;
        }
        else if(result == 3)
        {
            p_receiver->format_error_cnt++;
        }
        else if(result == 4)
        {
            p_receiver->queue_full_cnt++;
        }
        else if(result == 5)
        {
            p_receiver->no_enough_buf_cnt++;
        }
        else
        {
            //不支持的数据帧
        }
    }
    
    p_receiver->rx_lock = 0;
    
    return package_num;
}

//从队列中获取一个数据包，并占用其内存 
//注意必须与 IM_FreeReceivedPackage 配套使用，否则内存持续被占用，并且读指针不会移动
//返回当期队列中数据包个数(含本包数据)
int8_t IM_GetReceivedPacket(int8_t handle, uint32_t *tab, uint8_t *format, uint8_t **data, uint8_t *length, uint32_t *received_time, uint8_t repeatable)
{
    RECEIVER_INSTANCE *p_receiver;
    
    if(handle < 0 || handle > last_handle)
    {
        return -1;
    }
    p_receiver = &instance[handle].receivers;
    
    if(p_receiver->rx_lock)
    {
        return -2;
    }
    p_receiver->rx_lock = 1;
    
    if(!repeatable && p_receiver->read_buf_list->read_busy)//当前队首正在被访问 防止重复访问
    {
        p_receiver->rx_lock = 0;
        return -3;
    }
    
    if(p_receiver->packets_to_read == 0)//接收器队列为空
    {
        p_receiver->rx_lock = 0;
        return 0;
    }
    
    p_receiver->read_buf_list->read_busy = 1;
    
    //取出队列当前输出结点数据
    if(p_receiver->read_buf_list->format.tab_exist)
    {
        *tab = p_receiver->read_buf_list->tab;
    }
    else
    {
        *tab = 0xFFFFFFFF;
    }
    *format = p_receiver->read_buf_list->format.data_format;
    *data   = p_receiver->rx_buf + p_receiver->read_buf_list->data_offset;
    *length = p_receiver->read_buf_list->length;
#if (SUPPORT_TIME_RECORD)
    *received_time = p_receiver->read_buf_list->received_time;
#endif
    
    p_receiver->rx_lock = 0;
    
    return p_receiver->packets_to_read;
}

//获取当前接收器队列中数据包的个数
//返回 -1 表示句柄所指向的接收器不存在
int8_t IM_GetReceivedPacketCount(int8_t handle)
{
    if(handle < 0 || handle > last_handle)
    {
        return -1;
    }
    return instance[handle].receivers.packets_to_read;
}

//获取接收器队列中剩余可分配的最大连续缓冲区大小
int32_t IM_GetFreeBufferSizeOfReceiver(int8_t handle)
{
    int32_t max_free_buf_size;
    RECEIVER_INSTANCE *p_receiver;
    
    if(handle < 0 || handle > last_handle)
    {
        return -1;
    }
    p_receiver = &instance[handle].receivers;
    
    if(p_receiver->rx_lock)
    {
        return -2;
    }
    p_receiver->rx_lock = 1;
    
    //判断剩余连续缓冲区是否足够接收本帧数据
    if(p_receiver->read_buf_list->data_offset <= p_receiver->rx_buf_list->data_offset)//读（读出）指针在写（接收）指针之前
    {
        //有两片连续的缓冲区可选（分别在缓冲区首尾）
        if(p_receiver->rx_buf_list == p_receiver->read_buf_list && p_receiver->packets_to_read == 0)//特殊情况
        {
            //接收队列全空，意味着这两片缓冲区连成一片，即整个缓冲区大小
            //注意若 p_receiver->packets_to_read != 0 意味着接收队列全满
            max_free_buf_size = p_receiver->rx_buf_size;
        }
        else
        {
            //尾部缓冲区大小 p_receiver->rx_buf_size - p_receiver->rx_buf_list->data_offset
            max_free_buf_size = p_receiver->rx_buf_size - p_receiver->rx_buf_list->data_offset;
            
            //首部缓冲区大小 p_receiver->read_buf_list->data_offset
            if(p_receiver->read_buf_list->data_offset > max_free_buf_size)
            {
                max_free_buf_size = p_receiver->read_buf_list->data_offset;
            }
        }
    }
    else//读（读出）指针在写（接收）指针之后
    {
        max_free_buf_size = p_receiver->read_buf_list->data_offset - p_receiver->rx_buf_list->data_offset;
    }
    
    p_receiver->rx_lock = 0;
    
    return max_free_buf_size;
}

//释放正在读取的数据包所占内存
//返回当期队列中数据包个数(含本包数据)
int8_t IM_FreeReceivedPacket(int8_t handle)
{
    RECEIVER_INSTANCE *p_receiver;
    
    if(handle < 0 || handle > last_handle)
    {
        return -1;
    }
    p_receiver = &instance[handle].receivers;
    
    if(p_receiver->rx_lock)
    {
        return -2;
    }
    p_receiver->rx_lock = 1;
    
    if(p_receiver->read_buf_list->read_busy == 0)//当前队首数据尚未被读取
    {
        p_receiver->rx_lock = 0;
        return -3;
    }
    
    if(p_receiver->packets_to_read == 0)
    {
        p_receiver->rx_lock = 0;
        return 0;
    }
    
    p_receiver->read_buf_list->read_busy = 0;
    
    //移动当前输出结点，并更新队列长度
    //以下应确保为原子操作
    p_receiver->read_buf_list = p_receiver->read_buf_list->next;//切换到下一个缓冲区
    p_receiver->packets_to_read--;
    
    p_receiver->rx_lock = 0;
    
    return p_receiver->packets_to_read + 1;
}

//若句柄所对应的接收器不存在将返回 全0
//获取接收器统计数据
RECEIVER_STATISTICS IM_GetReceiverStatisticsData(int8_t handle)
{
    RECEIVER_STATISTICS statistics = {0};
    RECEIVER_INSTANCE *p_receiver;
    if(handle < 0 || handle > last_handle)
    {
        return statistics;
    }
    p_receiver = &instance[handle].receivers;
    
    statistics.packets_count = p_receiver->packets_cnt;
    statistics.check_error_count = p_receiver->check_error_cnt;
    statistics.format_error_count = p_receiver->format_error_cnt;
    statistics.queue_full_count = p_receiver->queue_full_cnt;
    statistics.no_enough_buf_count = p_receiver->no_enough_buf_cnt;
    
    return statistics;
}

//清零接收器统计数据
uint8_t IM_ClearReceiverStatisticsData(int8_t handle)
{
    RECEIVER_INSTANCE *p_receiver;
    if(handle < 0 || handle > last_handle)
    {
        return 0;
    }
    p_receiver = &instance[handle].receivers;
    
    p_receiver->packets_cnt = 0;
    p_receiver->check_error_cnt = 0;
    p_receiver->format_error_cnt = 0;
    p_receiver->queue_full_cnt = 0;
    p_receiver->no_enough_buf_cnt = 0;
    return 1;
}

//按指定格式及密码表打包数据，并输出至 缓冲区 out_buf
//输入
//cipher_table 密码表首地址
//key          随机密钥索引  超过255 无效  可用0xFFFF 表示不对数据加密
//tab          包标签  超过28位无效 可用0xFFFFFFFF 表示不使用标签
//data_format  数据格式 低7位有效，即有效范围 0-7
//data1        第一个数据段首地址 一般为 标准组中的 设备 通道 属性 或者专用组中的 功能号
//length1      第一个数据段长度 (byte)
//data2        第二个数据段首地址 一般为 指令参数
//length2      第二个数据段长度 (byte)
//check_type   校验类型
//输出
//out_buf      输出缓冲区起始地址  打包后的数据将被输出至此
//out_buf_size 输出缓冲区长度 (byte)
//返回
//打包后的数据总长度
static uint16_t IM_PackData(uint8_t *cipher_table, uint16_t key, uint32_t tab, uint8_t data_format, 
                            uint8_t *data1, uint8_t length1, uint8_t *data2, uint8_t length2, CHECK_TYPE check_type,
                            uint8_t *out_buf, uint16_t out_buf_size)
{
    uint16_t data_size;
    uint16_t offset;
    volatile FRAME_FORMAT frame_format;
    #if (SUPPORT_ENCRYPTION)
    uint8_t temp_key;
    #endif
    uint8_t *p_key;
    uint8_t header1_len;//Format + (Key) 长度 1-2 该部分固定为明文
    uint8_t header_len;//Format + (Key) + (Tab) + Length  长度 1-5
    uint8_t header[7];//Format + (Key) + (Tab) + Length
    
    uint8_t check_len;
    uint8_t sum;
    uint16_t crc16;
    #if (SUPPORT_CRC32)
    uint32_t crc32;
    #endif
    uint8_t check_byte[4];
    
    data_size = length1 + length2;
    if(out_buf_size < 4 + data_size || data_size > 255)//最短长度
    {
        return 0;
    }
    
    //Format整合
    frame_format.extend = 0;
    if(tab <= 0x0FFFFFFF)
    {
        frame_format.tab_exist = 1;
    }
    else
    {
        frame_format.tab_exist = 0;
    }
    frame_format.check_type = check_type&0x03;
    #if (SUPPORT_ENCRYPTION)
    if(key <= 0xFF)
    {
        frame_format.encryption = 1;
        temp_key = (uint8_t)key;
        p_key = &temp_key;
    }
    else
    {
        frame_format.encryption = 0;
        p_key = NULL;
    }
    #else
    frame_format.encryption = 0;
    p_key = NULL;
    #endif
    
    frame_format.data_format = data_format&0x07;
    
    header1_len = 0;
    header[header1_len++] = *(uint8_t *)&frame_format;
    if(frame_format.encryption)
    {
        header[header1_len++] = (uint8_t)key;
    }
    
    header_len = header1_len;
    if(frame_format.tab_exist)
    {
        uint32_t mask = 0x0FE0000;
        uint8_t tab_len = 4;
        
        while((tab & mask) == 0 && tab_len > 1)
        {
            mask >>= 7;
            tab_len--;
        }
        header_len += tab_len;
        
        header[header_len - 1] = (uint8_t)(tab&0x7F);//最后一个字节 最高位固定为 0
        
        if(tab_len > 1)//tab 占 2 3 4 字节
        {
            uint8_t *p_tab = &header[header_len - 2];//指向倒数第二个字节
            while(--tab_len)
            {
                tab >>= 7;
                *p_tab-- = (uint8_t)(tab|0x80);//其他字节最高位置 1
            }
        }
    }
    header[header_len++] = length1 + length2;
    
    //添加帧头
    out_buf[0] = FRAME_HEADER;
    out_buf[1] = FRAME_HEADER;
    offset = 2;
    
    //封装帧头(format + key)
    data_size = encrypt_escape_data(header, header1_len, &(out_buf[offset]), out_buf_size - offset, NULL, NULL);//固定明文
    if(data_size == 0xFFFF)
    {
        return 0;
    }
    offset += data_size;
    
    //封装帧头(tab + length)
    data_size = encrypt_escape_data(&header[header1_len], header_len - header1_len, &(out_buf[offset]), out_buf_size - offset, p_key, cipher_table);
    if(data_size == 0xFFFF)
    {
        return 0;
    }
    offset += data_size;
    
    //封装数据段1
    if(data1 != NULL && length1 != 0)
    {
        data_size = encrypt_escape_data(data1, length1, &(out_buf[offset]), out_buf_size - offset, p_key, cipher_table);
        if(data_size == 0xFFFF)
        {
            return 0;
        }
        offset += data_size;
    }
    
    //封装数据段2
    if(data2 != NULL && length2 != 0)
    {
        data_size = encrypt_escape_data(data2, length2, &(out_buf[offset]), out_buf_size - offset, p_key, cipher_table);
        if(data_size == 0xFFFF)
        {
            return 0;
        }
        offset += data_size;
    }
    
    //计算校验
    switch(check_type)
    {
        case NO_CHECK:
            check_len = 0;
            break;
        case PARITY:
            sum = calc_check_sum(header, header_len, 0);
            sum = calc_check_sum(data1, length1, sum);
            sum = calc_check_sum(data2, length2, sum);
            check_byte[0] = sum;
            check_len = 1;
            break;
        case CRC16:
            crc16 = calc_crc_16(header, header_len, 0xFFFF, CRC16_MODBUS_TABLE);
            crc16 = calc_crc_16(data1, length1, crc16, CRC16_MODBUS_TABLE);
            crc16 = calc_crc_16(data2, length2, crc16, CRC16_MODBUS_TABLE);
            check_byte[0] = (uint8_t)crc16;
            check_byte[1] = (uint8_t)(crc16>>8);
            check_len = 2;
            break;
        #if (SUPPORT_CRC32)
        case CRC32:
            crc32 = calc_crc_32(header, header_len, 0xFFFFFFFF, CRC32_TABLE);
            crc32 = calc_crc_32(data1, length1, crc32, CRC32_TABLE);
            crc32 = calc_crc_32(data2, length2, crc32, CRC32_TABLE);
            crc32 ^= 0xFFFFFFFF;
            check_byte[0] = (uint8_t)crc32;
            check_byte[1] = (uint8_t)(crc32>>8);
            check_byte[2] = (uint8_t)(crc32>>16);
            check_byte[3] = (uint8_t)(crc32>>24);
            check_len = 4;
            break;
        #endif
        default:
            return 0;
    }
    
    //封装校验字段
    data_size = encrypt_escape_data(check_byte, check_len, &(out_buf[offset]), out_buf_size - offset, p_key, cipher_table);
    if(data_size == 0xFFFF)
    {
        return 0;
    }
    offset += data_size;
    
    return offset;
}

//按指定格式将数据打包并输出至指定的发送器队列
//返回
//1-?数据包实际长度(包含帧头帧尾校验等)
//0 不可能出现
//-1 句柄所对应的收发器不存在
//-2 当前发送器正在被访问
//-3 发送器队列满
//-4 剩余连续缓冲区不足
int32_t IM_PackDataToTransmitter(int8_t handle, uint16_t key, uint32_t tab, uint8_t format, 
                                  uint8_t *data1, uint8_t length1, uint8_t *data2, uint8_t length2, 
                                  CHECK_TYPE check_type)
{
    static const uint8_t check_length[4] = {0, 1, 2, 4};
    uint16_t min_need_buf, max_need_buf, free_buf_size, real_data_length;
    TRANSMITTER_INSTANCE *p_transmitter;
    
    if(handle < 0 || handle > last_handle)
    {
        return -1;
    }
    p_transmitter = &instance[handle].transmitters;
    
    min_need_buf = 4 + length1 + length2 + check_length[(format>>4)&0x03];//0xAA 0xAA Format Length Data(length1 + length2) Check
    max_need_buf = min_need_buf*2 + 8;// = 2 + (min_need_buf - 2 + 5)*2;//按照Key、Tab、Data全为 FRAME_HEADER 或 ESCAPE_BYTE 算
    
    if(p_transmitter->tx_lock)
    {
        return -2;
    }
    
    p_transmitter->tx_lock = 1;
    
    if(p_transmitter->packets_to_send == MAX_TX_QUEUE_LENGTH)
    {
        if(p_transmitter->overlay)
        {
            goto BUF_LACK_PROCESS;
        }
        else
        {
            p_transmitter->tx_lock = 0;
            return -3;
        }
    }
    
    //从队列寻找一片足够空间的连续内存块
    MALLOC:
    
    if(p_transmitter->out_buf_list->data_offset <= p_transmitter->in_buf_list->data_offset)//读（发送）指针在写（输入）指针之前
    {
        //有两片连续的缓冲区可选（分别在缓冲区首尾）
        //分配策略 ： 最大成功率优先  即 尾部缓冲区 确定完全足够 则直接以尾部缓冲区分配， 若可能不足 则比较首部和尾部缓冲区大小，分配其中较大者，当然若队列全空，则分配整个缓冲区
        //这里于接收缓冲区的分配策略有所不同 主要由于数据发送时由于转义字节的存在会使实际发送数据有所增加，所以在不确定能满足需求的情况下，优先选取较大者 以确保最高成功率
        //当然也可以在分配前先通过遍历计算出实际需要的缓冲区大小，从而可以类似接收缓冲区的分配策略按照缓冲区利用率优先的原则分配，但这样做多了一次遍历操作大大增加了计算开销
        free_buf_size = p_transmitter->tx_buf_size - p_transmitter->in_buf_list->data_offset;
        if(free_buf_size < max_need_buf)//在尾部缓冲区可能不足时如果首部缓冲区更大则切换至首部缓冲区
        {
            if(p_transmitter->out_buf_list == p_transmitter->in_buf_list)//缓冲区全空或全满可以特殊处理
            {
                if(p_transmitter->packets_to_send == 0)
                {
                    //队列为空，将读写偏移移动至缓冲区首部 得到最大
                    p_transmitter->in_buf_list->data_offset = 0;//切换至首部缓冲区  p_transmitter->out_buf_list->data_offset = 0 可省略
                    free_buf_size = p_transmitter->tx_buf_size;
                }
                else//应不可能运行至该处 否则 p_transmitter->packets_to_send == MAX_TX_QUEUE_LENGTH 在之前已处理
                {
                    //缓冲区已满
                    p_transmitter->tx_lock = 0;
                    return -3;
                }
            }
            else
            {
                if(p_transmitter->out_buf_list->data_offset > free_buf_size)//首部缓冲区更大
                {
                    free_buf_size = p_transmitter->out_buf_list->data_offset;
                    p_transmitter->in_buf_list->data_offset = 0;//切换至首部缓冲区
                }
            }
        }
    }
    else
    {
        //缓冲区位于中部，且只有一个
        free_buf_size = p_transmitter->out_buf_list->data_offset - p_transmitter->in_buf_list->data_offset;
    }
    
    if(free_buf_size < min_need_buf)//剩余缓冲区不足
    {
        goto BUF_LACK_PROCESS;
    }
    
    real_data_length = IM_PackData(p_transmitter->cipher_table, key, tab, format, data1, length1, data2, length2, check_type, 
                                   p_transmitter->tx_buf + p_transmitter->in_buf_list->data_offset, free_buf_size);
    
    if(real_data_length == 0)//剩余缓冲区不足
    {
        goto BUF_LACK_PROCESS;
    }
    
    p_transmitter->in_buf_list->data_length = real_data_length;
    p_transmitter->in_buf_list->read_busy = 0;
    //设置下个缓冲区地址
    p_transmitter->in_buf_list->next->data_offset = (p_transmitter->in_buf_list->data_offset + real_data_length)%p_transmitter->tx_buf_size;
    //以下应确保为原子操作
    p_transmitter->packets_to_send++;
    p_transmitter->in_buf_list = p_transmitter->in_buf_list->next;//切换缓冲区
    
    p_transmitter->tx_lock = 0;
    return (int32_t)real_data_length;
    
    BUF_LACK_PROCESS:
    
    if(p_transmitter->overlay && p_transmitter->packets_to_send != 0)
    {
        p_transmitter->packets_to_send--;
        p_transmitter->out_buf_list = p_transmitter->out_buf_list->next;
        
        //以下步骤是为 同步 IM_GetDataFromTransmitter 方法，避免 用户使用 IM_GetDataFromTransmitter 读到与 IM_GetPacketFromTransmitter 重复的数据，也避免读到无效的数据
        if(p_transmitter->packets_to_send == 0)
        {
            p_transmitter->length = 0;
        }
        else
        {
            p_transmitter->length = p_transmitter->out_buf_list->data_length;
            p_transmitter->p_data = p_transmitter->tx_buf + p_transmitter->out_buf_list->data_offset;
        }
        goto MALLOC;//重新分配
    }
    else
    {
        p_transmitter->tx_lock = 0;
        return -4;
    }
}

//从输出队列读出一条指令  返回起始地址及数据长度，函数会将指令缓存队列内存共享给用户
//用户不需要再开辟内存存储数据, 但使用完毕必须使用IM_FreePackageFromTransmitter释放资源
//否则若队列写满，会影响新的指令入队
//返回剩余指令条数(含当期数据包)
int8_t IM_GetPacketFromTransmitter(int8_t handle, uint8_t **data, uint16_t *length, uint8_t repeatable)
{
    TRANSMITTER_INSTANCE *p_transmitter;
    
    if(handle < 0 || handle > last_handle)
    {
        return -1;
    }
    p_transmitter = &instance[handle].transmitters;
    
    if(p_transmitter->tx_lock)
    {
        return -2;
    }
    p_transmitter->tx_lock = 1;
    
    if(!repeatable && p_transmitter->out_buf_list->read_busy)//当前数据包正在被读取 防止重复读取
    {
        p_transmitter->tx_lock = 0;
        return -3;
    }
    
    if(p_transmitter->packets_to_send == 0)//发送器队列为空
    {
        p_transmitter->tx_lock = 0;
        return 0;
    }
    
    p_transmitter->out_buf_list->read_busy = 1;
    
    *data = p_transmitter->tx_buf + p_transmitter->out_buf_list->data_offset;
    *length = p_transmitter->out_buf_list->data_length;
    
    p_transmitter->tx_lock = 0;
    
    return p_transmitter->packets_to_send;
}

//获取发送器队列中数据包的数量
//返回 -1 表示句柄所指向的发送器不存在
int8_t IM_GetPacketCountOfTransmitter(int8_t handle)
{
    if(handle < 0 || handle > last_handle)
    {
        return -1;
    }
    return instance[handle].transmitters.packets_to_send;
}

//获取发送器队列中剩余可分配的最大连续缓冲区大小
int32_t IM_GetFreeBufferSizeOfTransmitter(int8_t handle)
{
    int32_t max_free_buf_size;
    TRANSMITTER_INSTANCE *p_transmitter;
    
    if(handle < 0 || handle > last_handle)
    {
        return -1;
    }
    p_transmitter = &instance[handle].transmitters;
    
    if(p_transmitter->tx_lock)
    {
        return -2;
    }
    p_transmitter->tx_lock = 1;
    
    //判断剩余连续缓冲区是否足够接收本帧数据
    if(p_transmitter->out_buf_list->data_offset <= p_transmitter->in_buf_list->data_offset)//读（发送）指针在写（输入）指针之前
    {
        //有两片连续的缓冲区可选（分别在缓冲区首尾）
        if(p_transmitter->out_buf_list == p_transmitter->in_buf_list && p_transmitter->packets_to_send == 0)//特殊情况
        {
            //发送队列全空，意味着这两片缓冲区连成一片，即整个缓冲区大小
            //注意若 p_transmitter->packets_to_send != 0 意味着发送队列全满
            max_free_buf_size = p_transmitter->tx_buf_size;
        }
        else
        {
            //尾部缓冲区大小 p_transmitter->tx_buf_size - p_transmitter->in_buf_list->data_offset
            max_free_buf_size = p_transmitter->tx_buf_size - p_transmitter->in_buf_list->data_offset;
            
            //首部缓冲区大小 p_transmitter->out_buf_list->data_offset
            if(p_transmitter->out_buf_list->data_offset > max_free_buf_size)
            {
                max_free_buf_size = p_transmitter->out_buf_list->data_offset;
            }
        }
    }
    else//读（发送）指针在写（输入）指针之后
    {
        max_free_buf_size = p_transmitter->out_buf_list->data_offset - p_transmitter->in_buf_list->data_offset;
    }
    
    p_transmitter->tx_lock = 0;
    
    return max_free_buf_size;
}

//释放从缓冲队列获取的数据包，释放从发送器缓存队列共享的内存
//返回 队列剩余数据包个数(含刚被释放的那个数据包)
int8_t IM_FreePacketFromTransmitter(int8_t handle)
{
    TRANSMITTER_INSTANCE *p_transmitter;
    
    if(handle < 0 || handle > last_handle)
    {
        return -1;
    }
    p_transmitter = &instance[handle].transmitters;
    
    if(p_transmitter->tx_lock)
    {
        return -2;
    }
    p_transmitter->tx_lock = 1;
    
    if(p_transmitter->out_buf_list->read_busy == 0)//当期数据包未被读取
    {
        p_transmitter->tx_lock = 0;
        return -3;
    }
    
    if(p_transmitter->packets_to_send == 0)//队列已经为空
    {
        p_transmitter->tx_lock = 0;
        return 0;
    }
    
    p_transmitter->out_buf_list->read_busy = 0;
    
    p_transmitter->packets_to_send--;
    p_transmitter->out_buf_list = p_transmitter->out_buf_list->next;
    
    //以下步骤是为 同步 IM_GetDataFromTransmitter 方法，避免 用户使用 IM_GetDataFromTransmitter 读到与 IM_GetPacketFromTransmitter 重复的数据，也避免读到无效的数据
    if(p_transmitter->packets_to_send == 0)
    {
        p_transmitter->length = 0;
    }
    else
    {
        p_transmitter->length = p_transmitter->out_buf_list->data_length;
        p_transmitter->p_data = p_transmitter->tx_buf + p_transmitter->out_buf_list->data_offset;
    }
    
    p_transmitter->tx_lock = 0;
    
    return p_transmitter->packets_to_send + 1;
}


//将队列数据输出到指定缓冲区，如果队列数据总体大小大于缓冲区大小，则将填满缓冲区，否则只输出实际的数据长度
//此方法不占用队列空间
int32_t IM_GetDataFromTransmitter(int8_t handle, uint8_t *data, uint16_t buf_size)
{
    uint16_t data_sum;
    uint16_t real_len;
    TRANSMITTER_INSTANCE *p_transmitter;
    
    if(handle < 0 || handle > last_handle)
    {
        return -1;
    }
    p_transmitter = &instance[handle].transmitters;
    
    if(p_transmitter->tx_lock)
    {
        return -2;
    }
    p_transmitter->tx_lock = 1;
    
    data_sum = 0;
    for(;;)
    {
        if(p_transmitter->length == 0)//前一包数据已经输出完毕
        {
            if(p_transmitter->packets_to_send == 0)
            {
                break;//队列空
            }
            p_transmitter->length = p_transmitter->out_buf_list->data_length;
            p_transmitter->p_data = p_transmitter->tx_buf + p_transmitter->out_buf_list->data_offset;
            
            p_transmitter->packets_to_send--;
            p_transmitter->out_buf_list = p_transmitter->out_buf_list->next;
        }
        
        if(buf_size == 0)//输出缓冲区已满
        {
            break;
        }
        
        real_len = p_transmitter->length > buf_size ? buf_size : p_transmitter->length;//取数据实际长度与输出缓冲区剩余大小中较小者
        
        memcpy(data, p_transmitter->p_data, real_len);
        
        data += real_len;
        buf_size -= real_len;
        p_transmitter->p_data += real_len;
        p_transmitter->length -= real_len;
        
        data_sum += real_len;
    }
    
    p_transmitter->tx_lock = 0;
    
    return data_sum;
}

//清空数据发送队列
int8_t IM_ClearTransmitterBuf(int32_t handle)
{
    int8_t i;
    TRANSMITTER_INSTANCE *p_transmitter;
    
    if(handle < 0 || handle > last_handle)
    {
        return -1;
    }
    p_transmitter = &instance[handle].transmitters;
    
    if(p_transmitter->tx_lock)
    {
        return -2;
    }
    
    p_transmitter->tx_lock = 1;
    
    p_transmitter->in_buf_list = &(p_transmitter->tx_buf_array[0]);
    p_transmitter->in_buf_list->data_offset = 0;//初始化第一个结点的偏移
    
    p_transmitter->out_buf_list = p_transmitter->in_buf_list;
    p_transmitter->packets_to_send = 0;
    
    p_transmitter->length = 0;
    
    for(i=0; i<MAX_TX_QUEUE_LENGTH; i++)
    {
        p_transmitter->tx_buf_array[i].read_busy = 0;
    }
    
    p_transmitter->tx_lock = 0;
    
    return 0;
}
