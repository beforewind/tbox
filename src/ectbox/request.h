#ifndef REQUEST_H
#define REQUEST_H

//#include <QObject>
//#include "ethercatbus.h"
//#include <mutex>

//#include <QtCore>
//#include <QtCore/QDatastream.h>

#include <stdint.h>

#define TPU_SYNC_MODE

//#define REQ_PDO_MODE
//#define RESP_PDO_MODE


#define IO_DELAY_TICK_PARA  5  // ms

// SyncPDO State
#define TPU_SYNC_PDO_IDLE       (0x00)  // req & resp
#define TPU_SYNC_PDO_SET        (0x01)  // req
#define TPU_SYNC_PDO_GET        (0x02)  // req
#define TPU_SYNC_PDO_SETREADY   (0x03)  // resp
#define TPU_SYNC_PDO_GETREADY   (0x04)  // resp

// ErrorCode
#define TPU_SUCCESS      (0x00)
#define TPU_SUCCESS1     (0x00)
#define TPU_SUCCESS2     (0x00)
#define TPU_SETIO_INDEX_ERROR    (0x80)
#define TPU_SETIO_BANK_ERROR     (0x81)
#define TPU_SETCOUT_ERROR        (0x82)


// Cmd
#define REQ_CMD_RESET      (0x01)
#define REQ_CMD_SELFTEST   (0x02)
#define REQ_CMD_SETCONFIG  (0x03)
#define REQ_CMD_GETCONFIG  (0x04)
#define REQ_CMD_SETPARA    (0x05)
#define REQ_CMD_GETPARA    (0x06)
#define REQ_CMD_SETIO      (0x07)
#define REQ_CMD_GETIO      (0x08)
#define REQ_CMD_SETCOUT    (0x09)
#define REQ_CMD_GETMEM     (0x0A)
#define REQ_CMD_SETMEM     (0x0B)



// 定义命令，用16bit表示对一个IO的操作？
// 3bitOp_5bitBank_8bitIndex
// 用16bit表示一个IO的操作，包括：IO的操作op，IO所在Bank，IO所在Bank中的位置index
#define INDEX_MASK     (0x00FF)
#define INDEX_SHIFT    (0)
#define BANK_MASK      (0x1F00)
#define BANK_SHIFT     (8)
#define OP_MASK        (0xE000)
#define OP_SHIFT       (13)
#define OP_OFF         (0x00)  // OP码3个bit，用于设置HF/LF/?
#define OP_HF          (0x01)  // OP_HF  100, OP_LF  010，可以“或”操作
#define OP_LF          (0x02)  // 支持高、低电平、悬空、双接四种状态
#define OP_HF_LF       (0x03)
#define OP_HS          (0x03)
#define OP_LS          (0x04)

// 由配置的SDO大小决定
#define MAX_SDO_REQ_SIZE  (16*4)  // 64bytes
#define MAX_SDO_SEG_SIZE  (MAX_SDO_REQ_SIZE-4)
#define MAX_SDO_RES_SIZE  (16*4)  // 64bytes
#define MAX_CFG_DAT_SIZE  (16*MAX_SDO_SEG_SIZE)  // max 4096byte
// TPU的存储规划
#define TPU_MAGIC_NUMBER   (0x12345678)
#define INDEX_HEADER       (0)
#define INDEX_MODULE       (1)


// 定义TPU的配置数据结构<SDO size-4
// default size<60bytes
// 出厂数据？
typedef struct
{
    uint32_t magicNumber;  // PDID?
//    uint32_t pdid;  // pdid一方面可以判断是否
    uint32_t createDate;  //???
    uint32_t updateDate;  // 0x20211026
    uint32_t updateTime;  // 00HHMMSS
    uint32_t tpuType;  // string?有Type就可以了，有关TPU的硬件资源情况在cfg()中
    uint8_t moduleDataIndex;  // Config数据存储的起始index
    uint8_t moduleDataABC;  // rfu
    uint16_t moduleDataSize;  // Config数据的大小/可能要多次读取后拼接
    uint32_t rfu[8];
} TPU_CONFIG_HEADER_T;

typedef struct
{
    uint32_t magicNumber;
    uint8_t dataType;  // 指明该struct包含的数据类型
    uint8_t tpuID;
    uint8_t tpuGroup;
    uint8_t tpuBank;
    uint8_t tpuOffset;
    uint8_t adapterTypeLength;
    uint8_t moduleNameLength;
    uint8_t adapterType[16];  //
    uint8_t moduleName[16];  //
} TPU_CONFIG_MODULE_T;

//#define REQ_MAX_DATA_FRAGMENT_SIZE	(512)
//#define REQ_DUMMY_BYTES					    (4)
//#define REQ_MAX_XFER_BUF_SIZE			  (512)
//#define REQ_XFER_TIMEOUT					  (10)  //ms

// tboxserver根据syncPDO和resp(或16bit的PDO)进行同步
// response是包的有效载荷，可以用PDO/MAIL方式传输，缺省用PDO

typedef struct
{
    uint8_t  reqSyncPDO;
    uint8_t  sequence;  // add+1
} REQUEST_SYNC_T;


//#ifdef REQ_PDO_MODE
typedef struct
{
    uint8_t  cmd;
    uint8_t  bank;
    uint8_t  index;
    uint8_t  reserved;
    uint8_t  data[8];
} REQUEST_T;

typedef struct
{
    uint8_t  cmd;
    uint8_t  index;
    uint16_t length;  // split into offset+length?? suport coarse/fine grain memory access
} REQUEST_HEADER_T;

typedef struct
{
    REQUEST_HEADER_T header;
    uint16_t data[4];
} REQUEST_IOPARA_T;

// continuity para
typedef struct
{
    uint8_t  cmd;
    uint8_t  mode;
    uint16_t timeFactor;
    uint16_t voltage;
    uint16_t current;
    uint16_t lowerThreshold;
    uint16_t upperThreshold;
} REQUEST_CONPARA_T;

// short para
typedef struct
{
    uint8_t  cmd;
    uint8_t  mode;
    uint16_t timeFactor;
    uint16_t voltage;
    uint16_t current;
    uint16_t lowerThreshold;
    uint16_t upperThreshold;
} REQUEST_SHTPARA_T;

typedef struct
{
    uint8_t  cmd;
    uint8_t  index;
    uint16_t value;
    uint16_t reserved0;
    uint16_t reserved1;
    uint16_t reserved2;
    uint16_t reserved3;
} REQUEST_SETCOUT_T;

typedef struct
{
    uint8_t  cmd;
    uint8_t  index;
    uint16_t value;
    uint8_t tpuType;
    uint8_t bankCount;  // ->bankCount
    uint16_t ioCount;  // ->ioCount
    uint32_t PDID;  // no use
} REQUEST_SETCONFIG_T;

//#else
//#endif



typedef struct
{
    uint8_t  respSyncPDO;
    //uint8_t  status;
    uint8_t  sequence;
} RESPONSE_SYNC_T;


//#ifdef RESP_PDO_MODE
typedef struct
{
    uint16_t status;
    uint16_t result;
    uint8_t  data[8];
} RESPONSE_T;

typedef struct
{
    uint16_t status;
    uint16_t result;
    uint16_t HS;
    uint16_t HF;
    uint16_t LF;
    uint16_t LS;
} RESPONSE_RESULT_T;

//#if 0
//// NOT implemented: QDataStream &operator >>(QDataStream &,RESPONSE_RESULT_T &)
//inline QDataStream &operator >>(QDataStream &stream, RESPONSE_RESULT_T &response)
//{
//    return (stream >> response.status >> response.result >> response.HS >> response.HF >> response.LF >> response.LS);
//}
//#endif

typedef struct
{
    uint16_t status;
    uint16_t result;
    uint8_t tpuType;
    uint8_t bankCount;  // ->bankCount
    uint16_t ioCount;  // ->ioCount
    uint32_t PDID;
} RESPONSE_CONFIG_T;
//#else
//#endif


// 测试结果指示
#define GOOD                         0  //	Good (no error)
#define OPEN_CIRCUIT                 1	//	Open Circuit
#define OPEN_CIRCUIT_HIGH_OHMIC      2	//	Open Circuit High Ohmic
#define CROSSED_WIRE                 8	//	Crossed Wires
#define MISWIRE                      9	//	Miswire
#define SHORT_CIRCUIT                10	//	Short Circuit
#define SHORT_CIRCUIT_HIGH_OHMIC     11	//	Short Circuit High Ohmic
#define INSULATION_FAULT             12	//	Insulation Fault
#define BREAKDOWN                    13	//	Breakdown
#define UNDER_VOLTAGE                14	//	Undervoltage
//#define 15	//	Sense Error
//#define 16	//	Current Low
//#define 17	//	HV: Unidentifiable Breakdown
//#define 18	//	HV: Unidentifiable Insulation Error
//#define 20	//	Resistor Missing
//#define 21	//	Resistor Short Circuit
//#define 22	//	Resistor Value Too High
//#define 23	//	Resistor Value Too Low


// 测试函数运行返回状态指示



#endif // REQUEST_H
