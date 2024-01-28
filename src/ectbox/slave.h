#ifndef SLAVE_H
#define SLAVE_H

#include <iostream>
#include <vector>
#include <map>

#include <soem/ethercat.h>
#include "osal.h"
//#include "parameter.h"
#include "request.h"

#define FLASH_MAGIC_NUMBER (0x12345678)
#define ADDR_FIRMWARE_BASE (0x20000)
#define ADDR_FLASH_BASE (0x40000) // pageIndex //
#define ADDR_OBJ_BASE (0x41000)   // object存储的page基地址，object的pageIndex必须从1开始 //
#define SIZE_FLASH_PAGE (0x1000)
#define COUNT_FLASH_PAGE (62)
#define ADDR_FLASH_META (0x40000)
#define INDX_FLASH_META (0)
#define ADDR_FLASH_ITEM (ADDR_FLASH_META + sizeof(FoeFlashHeaderMeta))
#define ADDR_FLASH_DATA (ADDR_FLASH_META + 0x1000)

// 每个page 4KB，最多64个page //
// 用flash的page0作为活跃表，用page1作为备份表？【备份表应该要放到较远的地址】//
// 存储在FLASH基地址的flash表头表项，每项对应一个bin文件或数据，可以用foe函数访问 //

#define MAX_NAME_SIZE (16)

struct FoeFlashItem
{
    // uint32_t magicNumber;
    uint32_t valid;    // if the item is valid
    uint32_t dataType; // device/adapter...
    // uint32_t baseAddr;    // data location in flash
    uint32_t pageIndex;       // there are multiple pages for one name
    int32_t dataSize;         // byte size of data
    uint32_t dataOffset;      // byte offset of data, 表示当前page的数据在obj.objData中的偏移地址，这样可以支持乱序排列item //
    char name[MAX_NAME_SIZE]; // data name(file name or structure name)
};

struct FoeFlashMeta
{
    uint32_t magicNumber; // PDID
    uint32_t createDate;  //
    uint32_t updateDate;  // 0x20211026
    uint32_t updateTime;  // 00HHMMSS
    uint32_t itemCount;   // max 64-1
    uint32_t rfu[3];      //
    // char     tpuName[16]; //
    FoeFlashItem itemList[COUNT_FLASH_PAGE];
    uint32_t magicNumber2;
};

enum EntryType
{
    Input,
    Output
};

struct PDOSubEntry
{
    std::string name;
    uint16_t subIndex;
    ec_datatype datatype;
    uint16_t bitLength;
    EntryType direction;
    uint16_t totalOffsetInBits;

    /*
    struct PDOSubEntryHash
    {
        std::size_t operator()(uint16_t slaveId, EntryType inputOutput, uint16_t pdoIndex, uint16_t pdoSubIndex) const noexcept
        {
            std::size_t h=0;
            helper::hash_combine(h, slaveId, inputOutput, pdoIndex, pdoSubIndex);
            return h;
        }
    };
    std::size_t hash;
    */
};

struct PDOEntry
{
    std::string name;
    uint16_t index;
    std::vector<PDOSubEntry> entries;
};

// 需要用一些SDO，存储device 配置信息，如板卡数量，电压电流阈值等作为read only参数 //
struct PDODescription  // 改成 std::map<std::string, PDOEntry> ? //
{
    //std::vector<PDOEntry> slaveOutputs;
    //std::vector<PDOEntry> slaveInputs;

    std::map<std::string, PDOEntry> sendPDO;
    std::map<std::string, PDOEntry> recvPDO;

    std::map<std::string, PDOEntry> sendSDO;
    std::map<std::string, PDOEntry> recvSDO;
};

class Slave
{
public:
    //Slave(RESPONSE_SYNC_T *readmap_, REQUEST_SYNC_T *writemap_, int slavenum_);
    // new functions
    Slave(ec_slavet *slaveT_, uint16_t slaveId_, uint8_t *readmap_, uint8_t *writemap_);

    // for FoE FLASH
    std::vector<uint8_t> &meta();
    FoeFlashMeta *pMeta();
    bool checkMeta();
    bool createMeta();
    FoeFlashItem *pItem(int i);
    std::vector<FoeFlashItem *> getFlashItemList(const std::string &objName);
    int32_t invalidFlashItemList(std::vector<FoeFlashItem *> &itemList);
    int32_t invalidFlashObject(const std::string &objName);
    std::vector<FoeFlashItem *> addFlashObjectToItemList(const std::string &objName, const uint32_t objType, const int32_t objSize);
    int32_t getFlashObjectSize(const std::string &objName);
    //uint32_t getFlashObjectType(const std::string &objType);
    std::map<std::string, std::pair<uint32_t, int32_t>> getFlashObjectMap();

    // for CoE SDO
    int32_t setDesc(PDODescription& desc);
    //PDOSubEntry& getSdoProject(const std::string& objName, const uint32_t objType);

    bool setSyncPDO();
    bool resetSyncPDO();
    bool canAcceptPDO();
    //bool isBusy();
    bool isRspReady();
    bool setPDO(uint16_t start_index, uint16_t pdo);
    bool setPDO(uint16_t start_index, std::vector<uint8_t> &pdoList);
    std::vector<uint8_t> getPDO(uint16_t start_index, uint16_t byteSize);

    uint16_t getReqPdoSize();
    uint16_t getRspPdoSize();

    // slave(RESPONSE_T* readmap_,REQUEST_T* writemap_,int slavenum_);
    //void sendPdo(uint8 Pdonum, uint32 Pdo);
    //void sendDataPdo(uint8_t *ba, int32_t size); // yinwenbo
    //void getDataPdo(uint8_t *ba, int32_t &size); // yinwenbo
    //uint16 getPdo(uint8 Pdonum);
    uint16_t getId();

private:
    //   mutable std::recursive_mutex mutex_;//我知道了，这个mutex不应该放在这里，应该放在一个既能读写又能通信的地方
    //int slavenum;
    //RESPONSE_SYNC_T *readmap;
    //REQUEST_SYNC_T *writemap;

    //RESPONSE_T *respDataPdo; // yinwenbo
    //REQUEST_T *reqDataPdo;

    // 有关该slave的设备信息: bankCount //
    //    uint16_t bankCount;

    // 指向ec_slavet
    ec_slavet *slaveT;
    uint16_t slaveId;
    //uint8_t *readmap;
    //uint8_t *writemap;

    // PDO固定部分
    std::map<uint16_t, uint16_t> rxPDO;
    uint16_t *reqSyncPDO;
    uint16_t *rspSyncPDO;
    uint16_t *reqStatPDO;
    uint16_t *rspStatPDO;

    // PDO可变部分
    uint16_t *reqDataPDO;
    uint16_t reqDataSize;
    uint16_t *rspDataPDO;
    uint16_t rspDataSize;

    // for CoE SDO
    // 从desc可以建立映射表，上层程序对index是不可见的 //
    // 因此使用name索引index //
    // PDO&SDO
    PDODescription pdoDesc;
//    std::map<std::string, PDOEntry> sendPDO;
//    std::map<std::string, PDOEntry> recvPDO;

//    std::map<std::string, PDOEntry> sendSDO;
//    std::map<std::string, PDOEntry> recvSDO;


    // for FoE FLASH
    std::vector<uint8_t> metaPage; // read from FoE page 0
    std::vector<FoeFlashItem *> itemList;
};

#endif // SLAVE_H
